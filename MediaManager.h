//////////////////////////////////////////////////////////////////////////////////////////
//		媒体管理类头文件 MediaManager.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
#define __BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
#define HAVE_AUDIO				1				//有音频
#define HAVE_CAPSCREEN			1				//有截屏事件

#include "../AudioModule/voice_engine/AudioManager.h"
#include "VideoEncodeThread.h"
#include "NetDataSyncSink.h"
#include "ScreenCapThread.h"
#include "TimerEngine.h"

///////////////////////////////////////////////////////////////////////////////////////////
class CMediaManager: public INetNotifySink,public ITimerEngineSink
{
private:
	bool						m_bRuning;					//运行标志
	CVideoEncodeThread			m_VideoThread;				//视频线程对象
#if HAVE_AUDIO
	CAudioManager				m_AudioManager;					//音频管理
#endif
	CNetDataSyncSink			m_NetDataSyncSink;			//网络回调对象
	CScreenCapThread			m_ScreenCapThread;			//屏幕截图线程
	CTimerEngine				m_TimerEngine;				//定时器线程

public:
	CMediaManager();
	~CMediaManager();

public:
	//初始化管理类
	bool InitManager(const char * pLbsIpList,const char *lpMediaPath);
	//开始服务
	bool StartService(int codecx,int codecy,int screencx,int screency);
	//停止服务
	bool StopService();

public:
	//添加屏幕截图
	bool AddScreenImage(BYTE *pImgData,int cx,int cy);
	//添加采集文本
	bool AddCaptureText(const char * pTxtContent);

public:
	//开始音视频采集
	virtual bool StartAVCapThread();
	//开始启动音频组件
	virtual bool StartAudioService();
	//停止音频组件
	virtual bool StopAudioService(bool bStopRec);

private:
	//启动音视频采集
	bool StarAVCapute();

public:
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam,void * pTimerSink);
};

#endif //__BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
