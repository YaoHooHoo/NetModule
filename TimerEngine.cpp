/////////////////////////////////////////////////////////////////////////
//		定时器实现文件
/////////////////////////////////////////////////////////////////////////
#include "TimerEngine.h"

//宏定义
#define NO_TIME_LEFT						DWORD(-1)				//不响应时间
//////////////////////////////////////////////////////////////////////////

//构造函数
CTimerThread::CTimerThread(void)
{
	m_dwTimerSpace=0L;
	m_pTimerEngine=NULL;
}

//析构函数
CTimerThread::~CTimerThread(void)
{
}

//配置函数
bool CTimerThread::InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace)
{
	//效益参数
	ASSERT(dwTimerSpace>=10L);
	ASSERT(pTimerEngine!=NULL);
	if (pTimerEngine==NULL) return false;

	//设置变量
	m_dwTimerSpace=dwTimerSpace;
	m_pTimerEngine=pTimerEngine;

	return true;
}

//运行函数
bool CTimerThread::RepetitionRun()
{
	ASSERT(m_pTimerEngine!=NULL);
	MyThreadWait(m_dwTimerSpace);
	m_pTimerEngine->OnTimerThreadSink();
	return true;
}

//////////////////////////////////////////////////////////////////////////
ITimerEngine *g_pITimerEngine=NULL;			//定时器接口

//构造函数
CTimerEngine::CTimerEngine(void)
{
	m_bService=false;
	m_dwTimePass=0L;
	m_dwTimerSpace=25L;
	m_dwTimeLeave=NO_TIME_LEFT;

	g_pITimerEngine=(ITimerEngine *)this;
}

//析构函数
CTimerEngine::~CTimerEngine(void)
{
	//停止服务
	StopService();
	//清理内存
	tagTimerItem * pTimerItem=NULL;
	INT_PTR nCount=m_TimerItemFree.size();
	for (INT_PTR i=0;i<nCount;i++)
	{
		pTimerItem=m_TimerItemFree[i];
		ASSERT(pTimerItem!=NULL);
		delete pTimerItem;
	}
	for (CTimerItemMapIt it=m_TimerItemActive.begin();it!=m_TimerItemActive.end();it++)
	{
		pTimerItem=(*it).second;
		ASSERT(pTimerItem!=NULL);
		delete pTimerItem;
	}
	m_TimerItemFree.clear();
	m_TimerItemActive.clear();

	return;
}

//设置定时器
bool CTimerEngine::SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam)
{
	return SetTimer(wTimerID,dwElapse,dwRepeat,wParam,NULL);
}

//设置定时器
bool CTimerEngine::SetTimer(WORD wTimerID,DWORD dwElapse,DWORD dwRepeat,WPARAM wParam,void * pTimerSink)
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//效验参数
	ASSERT(dwRepeat>0L);
	if (dwRepeat==0) return false;
	dwElapse=(dwElapse+m_dwTimerSpace-1)/m_dwTimerSpace*m_dwTimerSpace;

	//查找定时器
	bool bTimerExist=false;
	tagTimerItem * pTimerItem=NULL;

	CTimerItemMapIt it=m_TimerItemActive.find(wTimerID);
	if(it!=m_TimerItemActive.end())
	{
		bTimerExist=true;
		pTimerItem=(*it).second;
	}

	//创建定时器
	if (bTimerExist==false)
	{
		INT_PTR nFreeCount=(INT_PTR)m_TimerItemFree.size();
		if (nFreeCount>0)
		{
			pTimerItem=m_TimerItemFree.back();
			ASSERT(pTimerItem!=NULL);
			m_TimerItemFree.pop_back();
		}
		else
		{
			try
			{
				pTimerItem=new tagTimerItem;
				ASSERT(pTimerItem!=NULL);
				if (pTimerItem==NULL) return false;
			}
			catch (...) { return false; }
		}
	}

	//设置参数
	ASSERT(pTimerItem!=NULL);
	pTimerItem->wTimerID=wTimerID;
	pTimerItem->wBindParam=wParam;
	pTimerItem->dwElapse=dwElapse;
	pTimerItem->dwRepeatTimes=dwRepeat;
	pTimerItem->dwTimeLeave=dwElapse+m_dwTimePass;
	pTimerItem->pTimerSink=pTimerSink;

	//激活定时器
	m_dwTimeLeave=__min(m_dwTimeLeave,dwElapse);
	if (bTimerExist==false) m_TimerItemActive[wTimerID]=pTimerItem;

	return true;
}

//删除定时器
bool CTimerEngine::KillTimer(WORD wTimerID)
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//查找定时器
	tagTimerItem * pTimerItem=NULL;
	CTimerItemMapIt it=m_TimerItemActive.find(wTimerID);
	if(it==m_TimerItemActive.end()) return false;

	pTimerItem=(*it).second;
	m_TimerItemActive.erase(it);
	m_TimerItemFree.push_back(pTimerItem);
	INT nCount=(INT)m_TimerItemActive.size();
	if(nCount==0)
	{
		m_dwTimePass=0L;
		m_dwTimeLeave=NO_TIME_LEFT;
	}

	return true;
}

//删除定时器
bool CTimerEngine::KillAllTimer()
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//设置变量
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;

	//删除定时器
	for(CTimerItemMapIt it=m_TimerItemActive.begin();it!=m_TimerItemActive.end();it++)
	{
		m_TimerItemFree.push_back((*it).second);
	}
	m_TimerItemActive.clear();

	return true;
}

//开始服务
bool CTimerEngine::StartService()
{
	//效验状态
	if (m_bService==true) 
	{
		return true;
	}

	//没有回调
	if (m_TimerSinkMap.size()==0)
	{
		return false;
	}

	//设置变量
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;
	if (m_TimerThread.InitThread(this,m_dwTimerSpace)==false)
	{
		return false;
	}

	//启动服务
	if (m_TimerThread.StartThread()==false)
	{
		return false;
	}

	//设置变量
	m_bService=true;

	return true;
}

//停止服务
bool CTimerEngine::StopService()
{
	//设置变量
	m_bService=false;

	//停止线程
	m_TimerThread.StopThread();

	KillAllTimer();

	return true;
}

//设置接口
bool CTimerEngine::SetTimerEngineSink(ITimerEngineSink * pITimerEngineSink)
{
	//效验参数
	ASSERT(pITimerEngineSink!=NULL);
	ASSERT(m_bService==false);
	if (m_bService==true) return false;
	if (pITimerEngineSink==NULL) return false;

	CThreadLockHandle LockHandle(&m_ThreadLock);
	CTimerSinkMapIt it=m_TimerSinkMap.find(pITimerEngineSink);
	if(it!=m_TimerSinkMap.end())
	{
		(*it).second++;
		return true;
	}
	m_TimerSinkMap[pITimerEngineSink]=1;
	
	return true;
}

//定时器通知
void CTimerEngine::OnTimerThreadSink()
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//倒计时间
	if (m_dwTimeLeave==NO_TIME_LEFT) 
	{
		ASSERT(m_TimerItemActive.size()==0);
		return;
	}

	//减少时间
	ASSERT(m_dwTimeLeave>=m_dwTimerSpace);
	m_dwTimeLeave-=m_dwTimerSpace;
	m_dwTimePass+=m_dwTimerSpace;

	//查询定时器
	if (m_dwTimeLeave==0)
	{
		CTimerItemMap killItemMap;
		bool bKillTimer=false;
		tagTimerItem * pTimerItem=NULL;
		DWORD dwTimeLeave=NO_TIME_LEFT;
		for(CTimerItemMapIt it=m_TimerItemActive.begin();it!=m_TimerItemActive.end();it++)
		{
			//效验参数
			pTimerItem=(*it).second;
			ASSERT(pTimerItem!=NULL);
			ASSERT(pTimerItem->dwTimeLeave>=m_dwTimePass);

			//定时器处理
			bKillTimer=false;
			pTimerItem->dwTimeLeave-=m_dwTimePass;
			if (pTimerItem->dwTimeLeave==0L)
			{
				//发送通知
				OnTimerEventSink(pTimerItem->wTimerID,pTimerItem->wBindParam,pTimerItem->pTimerSink);
				
				//设置次数
				if (pTimerItem->dwRepeatTimes!=TIMER_REPEAT_TIMER)
				{
					ASSERT(pTimerItem->dwRepeatTimes>0);
					if (pTimerItem->dwRepeatTimes==1L)
					{
						bKillTimer=true;
						killItemMap[pTimerItem->wTimerID]=pTimerItem;
					}
					else pTimerItem->dwRepeatTimes--;
				}

				//设置时间
				if (bKillTimer==false) pTimerItem->dwTimeLeave=pTimerItem->dwElapse;
			}

			//增加索引
			if (bKillTimer==false) 
			{
				dwTimeLeave=__min(dwTimeLeave,pTimerItem->dwTimeLeave);
				ASSERT(dwTimeLeave%m_dwTimerSpace==0);
			}
		}

		//设置响应
		m_dwTimePass=0L;
		m_dwTimeLeave=dwTimeLeave;

		for(CTimerItemMapIt kIt=killItemMap.begin();kIt!=killItemMap.end();kIt++)
		{
			pTimerItem=(*kIt).second;
			m_TimerItemActive.erase((*kIt).first);
			m_TimerItemFree.push_back(pTimerItem);
		}
	}

	return;
}

//定时器回调
void CTimerEngine::OnTimerEventSink(WORD wTimerID,WPARAM wParam,void * pTimerSink)
{
	ITimerEngineSink *pITimerEngineSink=NULL;
	for(CTimerSinkMapIt it=m_TimerSinkMap.begin();it!=m_TimerSinkMap.end();it++)
	{
		pITimerEngineSink=(*it).first;
		ASSERT(pITimerEngineSink!=NULL);
		if(pITimerEngineSink!=NULL)
			pITimerEngineSink->OnEventTimer(wTimerID,wParam,pTimerSink);
	}
}


//////////////////////////////////////////////////////////////////////////
