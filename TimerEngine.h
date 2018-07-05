//////////////////////////////////////////////////////////////////////////
//		定时器引擎类头文件 TimerEngine.h
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <map>
#include "localdef.h"

/////////////////////////////////////////////////////////////////////////////
#define TIMER_REPEAT_TIMER			DWORD(-1)			//重复次数
#define SETTIMER(id,elapse,repeat,param,sink) {if(g_pITimerEngine!=NULL){g_pITimerEngine->SetTimer(id,elapse,repeat,param,sink);}}
#define KILLTIMER(id){if(g_pITimerEngine!=NULL){g_pITimerEngine->KillTimer(id);}}

//类说明
class CTimerEngine;

///////////////////////////////////////////////////////////////////////////
struct ITimerSink
{
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam)=0;
};

//定时器回调接口
struct ITimerEngineSink
{
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam,void * pTimerSink)=0;
};

/////////////////////////////////////////////////////////////////////////
struct ITimerEngine
{
	//设置定时器
	virtual bool SetTimer(WORD wTimerID,DWORD dwElapse,DWORD dwRepeat,WPARAM wParam)=0;
	//设置定时器
	virtual bool SetTimer(WORD wTimerID,DWORD dwElapse,DWORD dwRepeat,WPARAM wParam,void * pTimerSink)=0;
	//删除定时器
	virtual bool KillTimer(WORD wTimerID)=0;
	//删除所有定时器
	virtual bool KillAllTimer()=0;
};

struct ITimerEngineManager
{
	//开始服务
	virtual bool StartService()=0;
	//停止服务
	virtual bool StopService()=0;
	//设置接口
	virtual bool SetTimerEngineSink(ITimerEngineSink * pITimerEngineSink)=0;
};

//////////////////////////////////////////////////////////////////////////

//定时器线程
class CTimerThread : public CServiceThread
{
	//变量定义
protected:
	DWORD								m_dwTimerSpace;					//时间间隔
	CTimerEngine						* m_pTimerEngine;				//定时器引擎

	//函数定义
public:
	//构造函数
	CTimerThread(void);
	//析构函数
	virtual ~CTimerThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//定时器子项
struct tagTimerItem
{
	WORD								wTimerID;						//定时器 ID
	DWORD								dwElapse;						//定时时间
	DWORD								dwTimeLeave;					//倒计时间
	DWORD								dwRepeatTimes;					//重复次数
	WPARAM								wBindParam;						//绑定参数
	void								*pTimerSink;					//回调接口
};

//定时器子项数组定义
typedef std::map<WORD,tagTimerItem *>					CTimerItemMap;
typedef std::map<WORD,tagTimerItem *>::iterator			CTimerItemMapIt;
typedef std::vector<tagTimerItem *>						CTimerItemVector;
typedef std::vector<tagTimerItem *>::iterator			CTimerItemVectorIt;
typedef std::map<ITimerEngineSink *,DWORD>				CTimerSinkMap;
typedef std::map<ITimerEngineSink *,DWORD>::iterator	CTimerSinkMapIt;
//////////////////////////////////////////////////////////////////////////

//定时器引擎
class CTimerEngine :public ITimerEngine, public ITimerEngineManager
{
	friend class CTimerThread;

	//配置定义
protected:
	DWORD								m_dwTimerSpace;					//时间间隔

	//状态变量
protected:
	bool								m_bService;						//运行标志
	DWORD								m_dwTimePass;					//经过时间
	DWORD								m_dwTimeLeave;					//倒计时间
	CTimerItemVector					m_TimerItemFree;				//空闲数组
	CTimerItemMap						m_TimerItemActive;				//活动数组

	//组件变量
protected:
	CThreadLock							m_ThreadLock;					//线程锁
	CTimerThread						m_TimerThread;					//定时器线程
	CTimerSinkMap						m_TimerSinkMap;					//回调接口map对象

	//函数定义
public:
	//构造函数
	CTimerEngine(void);
	//析构函数
	virtual ~CTimerEngine(void);

	//接口函数
public:
	//设置定时器
	virtual bool SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam);
	//设置定时器
	virtual bool SetTimer(WORD wTimerID,DWORD dwElapse,DWORD dwRepeat,WPARAM wParam,void * pTimerSink);
	//删除定时器
	virtual bool KillTimer(WORD wTimerID);
	//删除定时器
	virtual bool KillAllTimer();

	//管理接口
public:
	//开始服务
	virtual bool StartService();
	//停止服务
	virtual bool StopService();
	//设置接口
	virtual bool SetTimerEngineSink(ITimerEngineSink * pITimerEngineSink);

	//内部函数
private:
	//定时器通知
	void OnTimerThreadSink();
	//定时器回调
	void OnTimerEventSink(WORD wTimerID,WPARAM wParam,void * pTimerSink);

};

////////////////////////////////////////////////////////////////////////////////
extern ITimerEngine *g_pITimerEngine;			//定时器接口
