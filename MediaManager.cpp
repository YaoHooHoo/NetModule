//////////////////////////////////////////////////////////////////////////////////////////
//		媒体管理类实现文件 MediaManager.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/MediaManager"
#include "log.h"
#include "MediaManager.h"

////////////////////////////////////////////////////////////////////
GLOBALDATA g_GlobalData={0};

//解析服务器IP地址
void ParseLbsIpList(LPCTSTR lszTmpIP);
///////////////////////////////////////////////////////////////////////////////////////////
CMediaManager::CMediaManager()
{
	m_bRuning=false;
}

CMediaManager::~CMediaManager()
{
	
}

//初始化管理类
bool CMediaManager::InitManager(const char * pLbsIpList,const char *lpMediaPath)
{
	//解析IP列表
	ParseLbsIpList(pLbsIpList);

	ITimerEngineSink *pITimerEngineSink=(ITimerEngineSink *)this;
	m_TimerEngine.SetTimerEngineSink(pITimerEngineSink);

	IMideaDataSink *pIMideaDataSink=(IMideaDataSink*)&m_NetDataSyncSink;
	IAudioDataSink *pIAudioDataSink=(IAudioDataSink *)&m_NetDataSyncSink;
	if(!m_VideoThread.InitEncodeThread(pIMideaDataSink))
	{
		return false;
	}
#if HAVE_AUDIO
	if(!m_AudioManager.InitAudioManage())
	{
		return false;
	}
	if(!m_AudioManager.SetAudioDataSink(pIAudioDataSink))
	{
		return false;
	}
	IAudioManager *pIAudioManager=(IAudioManager *)&m_AudioManager;
	m_NetDataSyncSink.SetAudioManager(pIAudioManager);
#endif
	INetNotifySink *pINetNotifySink=(INetNotifySink *)this;
	m_NetDataSyncSink.InitDataSyncThread(pINetNotifySink,lpMediaPath);

	return true;
}

//启动音视频采集
bool CMediaManager::StarAVCapute()
{
	LOGI("---1--start video enocder thread...");
	if(!m_VideoThread.StartVideoThread())
	{
		OnNotifyScreenCap(JNI_SC_1006);
		return false;
	}
	LOGI("---2--start cap screen thread...");
#if HAVE_CAPSCREEN
	if(!m_ScreenCapThread.StartCapThread())
	{
		OnNotifyScreenCap(JNI_SC_1004);
	}
#endif

	LOGI("---4--StarAVCapute success");
	m_bRuning=true;
	return true;
}

//开始服务
bool CMediaManager::StartService(int codecx,int codecy,int screencx,int screency)
{
	m_VideoThread.SetVideoSize(codecx,codecy,screencx,screency);

	if(!m_TimerEngine.StartService())
	{
		LOGE("---start timer engine faild");
		OnNotifyScreenCap(JNI_SC_1020);
	}

#if DT_NET_MODE
	if(!m_NetDataSyncSink.StartNetThread())
	{
		OnNotifyScreenCap(JNI_SC_1007);
	}
#endif
	if(!m_NetDataSyncSink.ConnectToServer(CST_LBSVR))
	{
		OnNotifyScreenCap(JNI_SC_1008);
	}

#if !DT_NET_MODE
	StartAudioService();
	return StarAVCapute();
#endif

	return true;
}

//停止服务
bool CMediaManager::StopService()
{
	m_bRuning=false;

	LOGI("---1--stop timer thread...");
	m_TimerEngine.StopService();
	LOGI("---2--stop screen cat thread...");
#if HAVE_CAPSCREEN
	//m_ScreenCapThread.SetCapVideo(false);
	m_ScreenCapThread.StopCapThread();
#endif
	LOGI("---3--stop video encode thread...");
	m_VideoThread.StopVideoThread();
	LOGI("---4--StopAudioService...");
	StopAudioService(false);
	LOGI("---5--stop net thread...");
#if DT_NET_MODE
	m_NetDataSyncSink.CloseConnect();
#endif
	LOGI("-----service stop completed");
	return true;
}

//添加屏幕截图
bool CMediaManager::AddScreenImage(BYTE *pImgData,int cx,int cy)
{
	if(!m_bRuning) return true;
	m_VideoThread.AddFrameImage(pImgData,cx,cy);
	return true;
}

//添加采集文本
bool CMediaManager::AddCaptureText(const char * pTxtContent)
{
	if(!m_bRuning) return true;
	return m_NetDataSyncSink.SendTextData(pTxtContent);
}

//开始音视频采集
bool CMediaManager::StartAVCapThread()
{
	return StarAVCapute();
}

//开始启动音频组件
bool CMediaManager::StartAudioService()
{
#if HAVE_AUDIO
	if(g_GlobalData.bHaveVoice)
	{
		LOGI("---3--start recording audio...");
		if(!m_AudioManager.StartPlaying())		//启动播放
		{
			OnNotifyScreenCap(JNI_SC_1006);
			LOGI("---3.1--start audio player fail ...");
			return false;
		}
		if(!m_AudioManager.StartRecording())
		{
			OnNotifyScreenCap(JNI_SC_1006);
			LOGI("---3.1--start audio recorder fail ...");
			return false;
		}
	}
#endif
	return true;
}

//停止音频组件
bool CMediaManager::StopAudioService(bool bStopRec)
{
#if HAVE_AUDIO
	if(g_GlobalData.bHaveVoice)
	{
		m_AudioManager.StopPlaying();
		m_AudioManager.StopRecording(bStopRec);
	}
#endif

	return true;
}

//定时器事件
bool CMediaManager::OnEventTimer(WORD wTimerID,WPARAM wBindParam,void * pTimerSink)
{
	if(pTimerSink!=NULL)
	{
		((ITimerSink *)pTimerSink)->OnEventTimer(wTimerID,wBindParam);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//获取以指定字符分隔的字符串
int ParseSubString(LPCTSTR lpszList,const TCHAR *split,CStringVec & slist)
{
	if(lpszList==NULL) return 0;

	int nCount=0;
	TCHAR *pSplit = strtok((TCHAR *)lpszList,split); 	
	while(pSplit!=NULL)
	{
		string strSub=pSplit;
		slist.push_back(strSub);
		pSplit= strtok(NULL,split); 
		nCount++;
	}

	return nCount;
}

//解析服务器IP地址
void ParseLbsIpList(LPCTSTR lszTmpIP)
{
	CStringVec slist;
	int nCount=ParseSubString(lszTmpIP,",",slist);
	if(nCount>0)
	{
		TCHAR szSubStr[MAX_PATH];
		for(int i=0;i<nCount && g_GlobalData.wLbsCount<LBSMAXNUM;i++)
		{
			CStringVec itemVec;
			string strItem=slist[i];
			lstrcpy(szSubStr,strItem.c_str());
			int nSubCount=ParseSubString(szSubStr,":",itemVec);
			if(nSubCount==2)
			{
				string strSvrIp=itemVec[0];
				string strPort=itemVec[1];
				WORD wPort=(WORD)atoi(strPort.c_str());
				strcpy(g_GlobalData.mLbsList[g_GlobalData.wLbsCount].szLbsIp,strSvrIp.c_str());
				g_GlobalData.mLbsList[g_GlobalData.wLbsCount++].wLbsPort=wPort;
			}
		}
	}
	if(g_GlobalData.wLbsCount>0)
	{	
		strcpy(g_GlobalData.mLbsInfo.szLbsIp,g_GlobalData.mLbsList[0].szLbsIp);
		g_GlobalData.mLbsInfo.wLbsPort=g_GlobalData.mLbsList[0].wLbsPort;
	}
}

