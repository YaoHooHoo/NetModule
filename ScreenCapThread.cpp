//////////////////////////////////////////////////////////////////////////////////////////
//		屏幕截屏线程类头文件 ScreenCapThread.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/ScreenThread"
#include "log.h"
#include "ScreenCapThread.h"

bool OnNotifyScreenCap(int nEventID);
///////////////////////////////////////////////////////////////////////////////////////////
CScreenCapThread::CScreenCapThread()
{
	m_bCapVideo=false;
}

CScreenCapThread::~CScreenCapThread()
{
}

//开始采集线程
bool CScreenCapThread::StartCapThread()
{
	m_bCapVideo=StartThread();
	return m_bCapVideo;
}

//停止采集线程
bool CScreenCapThread::StopCapThread()
{
	m_bRun=false;

	return StopThread(1);
}

//线程体函数
bool CScreenCapThread::RepetitionRun()
{
	MyThreadWait(200);

	if(m_bRun && m_bCapVideo)
	{
		OnNotifyScreenCap(0);		//通知Java层截屏
	}

	return m_bRun;
}
