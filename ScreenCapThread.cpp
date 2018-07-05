//////////////////////////////////////////////////////////////////////////////////////////
//		��Ļ�����߳���ͷ�ļ� ScreenCapThread.cpp
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

//��ʼ�ɼ��߳�
bool CScreenCapThread::StartCapThread()
{
	m_bCapVideo=StartThread();
	return m_bCapVideo;
}

//ֹͣ�ɼ��߳�
bool CScreenCapThread::StopCapThread()
{
	m_bRun=false;

	return StopThread(1);
}

//�߳��庯��
bool CScreenCapThread::RepetitionRun()
{
	MyThreadWait(200);

	if(m_bRun && m_bCapVideo)
	{
		OnNotifyScreenCap(0);		//֪ͨJava�����
	}

	return m_bRun;
}
