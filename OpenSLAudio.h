//////////////////////////////////////////////////////////////////////////////////////////
//		Android OpenSLAudio��Ƶ������ͷ�ļ� OpenSLAudio.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_OPENSL_AUDIO_MANAGER_HEAD_FILE__
#define __BOWEN_HU_OPENSL_AUDIO_MANAGER_HEAD_FILE__

#include "opensl_audio.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "SLES/OpenSLES_AndroidConfiguration.h"
#include "AacEncoder.h"
#include "AacDecoder.h"
#include "localdef.h"

///////////////////////////////////////////////////////////////////////////////////////////
class COpenSLAudio: public CServiceThread
{
private:
	bool					m_bInited;						//��ʼ����־
	bool					m_bRecInited;					//��̨��¼����־
	bool					m_bRecording;					//¼����־
	int						m_RecWarning;					//¼������
	int						m_RecError;						//������

private:	
	IMideaDataSink			*m_pIMideaDataSink;				//�ص��ӿ�

private:
	SLObjectItf				engineObject;
	SLEngineItf				engineEngine;
	SLObjectItf				recorderObject;
	SLRecordItf				recorderRecord;
	SLAndroidSimpleBufferQueueItf recorderBufferQueue;

private:
	uint16_t				m_nRecordingDelay;
	SLDataFormat_PCM		record_pcm_;

private:
	CAudioQueue				m_RecQueue;						//¼������
	CAudioQueue				m_EncodeQueue;					//���뻺��
	CAudioQueue				m_ReadyQueue;					//�ձն���
	CThreadLock				m_AudioLock;					//��Ƶ������

private:
	int8_t					m_rec_buf[N_REC_QUEUE_BUFFERS][REC_BUF_SIZE];
	int8_t					m_rec_enc_buf[N_REC_QUEUE_BUFFERS][REC_BUF_SIZE];

private:
	AACHANDLE				m_AacHandle;					//Aac���
	BYTE					*m_pcbPcmBuffer;				//PCM���ݻ���
	DWORD					m_dwPcmDataSize;				//PCM���ݴ�С

private:
	bool					m_bInitPlay;					//���ų�ʼ��
	bool					m_bIsPlaying;					//�����б�־
	bool					m_bPlayoutSpecified;			//�����־
	bool					m_bIsSpeadkerInited;			//Speak��ʼ����־

private:
	SLDataFormat_PCM		player_pcm_;					//����PCM����
	SLObjectItf				playerObject;					//���Ŷ���
	


public:
	COpenSLAudio();
	~COpenSLAudio();

public:
	//��ʼ��
	bool InitOpenSLAudio(IMideaDataSink *pIMideaDataSink);
	//��ʼ��¼��
	bool InitRecording();
	//��ʼ¼��
	bool StartRecording();
	//ֹͣ¼��
	bool StopRecording();

public:
	//��ʼ������
	bool InitPlayout();

private:
	//����¼���ӳ�
	void UpdateRecordingDelay();

protected:
     //��ʼ֪ͨ����
     virtual bool OnThreadStartEvent();
     //�߳�ֹ֪ͣͨ����
     virtual bool OnThreadStopEvent();
     
     //Inside Function
protected:
     //�߳��庯��
     virtual bool RepetitionRun();


protected:
	//¼�����庯��
	void RecorderSimpleBufferQueueCallbackHandler(SLAndroidSimpleBufferQueueItf queueItf);
	//¼������ص�����
	static void RecorderSimpleBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf,void *pContext);

private:
	//����AAC�����
	bool LoadAacLibrary();
	//������Ƶ����
	int EncodeAacData(int8_t *pAudioData,DWORD dwDataSize);

};

#endif //__BOWEN_HU_OPENSL_AUDIO_MANAGER_HEAD_FILE__
