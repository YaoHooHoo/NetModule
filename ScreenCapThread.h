//////////////////////////////////////////////////////////////////////////////////////////
//		��Ļ�����߳���ͷ�ļ� ScreenCapThread.h
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
	bool				m_bCapVideo;					//�ɼ���־

public:
	CScreenCapThread();
	~CScreenCapThread();

public:
	//��ʼ�ɼ��߳�
	bool StartCapThread();
	//ֹͣ�ɼ��߳�
	bool StopCapThread();
	//�豸�ɼ���־
	void SetCapVideo(bool bCap){m_bCapVideo=bCap;}
     
     //Inside Function
protected:
     //�߳��庯��
     virtual bool RepetitionRun();
};

#endif //__BOWEN_HU_SCREEN_CAP_THREAD_HEAD_FILE__
