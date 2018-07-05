//////////////////////////////////////////////////////////////////////////////////////////
//		屏幕截屏线程类头文件 ScreenCapThread.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_SCREEN_CAP_THREAD_HEAD_FILE__
#define __BOWEN_HU_SCREEN_CAP_THREAD_HEAD_FILE__

#include "localdef.h"

///////////////////////////////////////////////////////////////////////////////////////////
class CScreenCapThread:public CServiceThread
{
private:
	bool				m_bCapVideo;					//采集标志

public:
	CScreenCapThread();
	~CScreenCapThread();

public:
	//开始采集线程
	bool StartCapThread();
	//停止采集线程
	bool StopCapThread();
	//设备采集标志
	void SetCapVideo(bool bCap){m_bCapVideo=bCap;}
     
     //Inside Function
protected:
     //线程体函数
     virtual bool RepetitionRun();
};

#endif //__BOWEN_HU_SCREEN_CAP_THREAD_HEAD_FILE__
