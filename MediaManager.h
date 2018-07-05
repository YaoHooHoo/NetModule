//////////////////////////////////////////////////////////////////////////////////////////
//		ý�������ͷ�ļ� MediaManager.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
#define __BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
#define HAVE_AUDIO				1				//����Ƶ
#define HAVE_CAPSCREEN			1				//�н����¼�

#include "../AudioModule/voice_engine/AudioManager.h"
#include "VideoEncodeThread.h"
#include "NetDataSyncSink.h"
#include "ScreenCapThread.h"
#include "TimerEngine.h"

///////////////////////////////////////////////////////////////////////////////////////////
class CMediaManager: public INetNotifySink,public ITimerEngineSink
{
private:
	bool						m_bRuning;					//���б�־
	CVideoEncodeThread			m_VideoThread;				//��Ƶ�̶߳���
#if HAVE_AUDIO
	CAudioManager				m_AudioManager;					//��Ƶ����
#endif
	CNetDataSyncSink			m_NetDataSyncSink;			//����ص�����
	CScreenCapThread			m_ScreenCapThread;			//��Ļ��ͼ�߳�
	CTimerEngine				m_TimerEngine;				//��ʱ���߳�

public:
	CMediaManager();
	~CMediaManager();

public:
	//��ʼ��������
	bool InitManager(const char * pLbsIpList,const char *lpMediaPath);
	//��ʼ����
	bool StartService(int codecx,int codecy,int screencx,int screency);
	//ֹͣ����
	bool StopService();

public:
	//�����Ļ��ͼ
	bool AddScreenImage(BYTE *pImgData,int cx,int cy);
	//��Ӳɼ��ı�
	bool AddCaptureText(const char * pTxtContent);

public:
	//��ʼ����Ƶ�ɼ�
	virtual bool StartAVCapThread();
	//��ʼ������Ƶ���
	virtual bool StartAudioService();
	//ֹͣ��Ƶ���
	virtual bool StopAudioService(bool bStopRec);

private:
	//��������Ƶ�ɼ�
	bool StarAVCapute();

public:
	//��ʱ���¼�
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam,void * pTimerSink);
};

#endif //__BOWEN_HU_MEDIA_MANAGER_HEAD_FILE__
