//////////////////////////////////////////////////////////////////////////////////////////
//		Android OpenSLAudio音频管理类头文件 OpenSLAudio.h
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
	bool					m_bInited;						//初始化标志
	bool					m_bRecInited;					//初台化录音标志
	bool					m_bRecording;					//录音标志
	int						m_RecWarning;					//录音警告
	int						m_RecError;						//错误码

private:	
	IMideaDataSink			*m_pIMideaDataSink;				//回调接口

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
	CAudioQueue				m_RecQueue;						//录音缓冲
	CAudioQueue				m_EncodeQueue;					//编码缓冲
	CAudioQueue				m_ReadyQueue;					//空闭队列
	CThreadLock				m_AudioLock;					//音频锁对象

private:
	int8_t					m_rec_buf[N_REC_QUEUE_BUFFERS][REC_BUF_SIZE];
	int8_t					m_rec_enc_buf[N_REC_QUEUE_BUFFERS][REC_BUF_SIZE];

private:
	AACHANDLE				m_AacHandle;					//Aac句柄
	BYTE					*m_pcbPcmBuffer;				//PCM数据缓冲
	DWORD					m_dwPcmDataSize;				//PCM数据大小

private:
	bool					m_bInitPlay;					//播放初始化
	bool					m_bIsPlaying;					//播放中标志
	bool					m_bPlayoutSpecified;			//输入标志
	bool					m_bIsSpeadkerInited;			//Speak初始化标志

private:
	SLDataFormat_PCM		player_pcm_;					//播放PCM对象
	SLObjectItf				playerObject;					//播放对象
	


public:
	COpenSLAudio();
	~COpenSLAudio();

public:
	//初始化
	bool InitOpenSLAudio(IMideaDataSink *pIMideaDataSink);
	//初始化录音
	bool InitRecording();
	//开始录音
	bool StartRecording();
	//停止录音
	bool StopRecording();

public:
	//初始化播放
	bool InitPlayout();

private:
	//更新录音延迟
	void UpdateRecordingDelay();

protected:
     //开始通知函数
     virtual bool OnThreadStartEvent();
     //线程停止通知函数
     virtual bool OnThreadStopEvent();
     
     //Inside Function
protected:
     //线程体函数
     virtual bool RepetitionRun();


protected:
	//录音缓冲函数
	void RecorderSimpleBufferQueueCallbackHandler(SLAndroidSimpleBufferQueueItf queueItf);
	//录音缓冲回调函数
	static void RecorderSimpleBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf,void *pContext);

private:
	//加载AAC编码库
	bool LoadAacLibrary();
	//编码音频数据
	int EncodeAacData(int8_t *pAudioData,DWORD dwDataSize);

};

#endif //__BOWEN_HU_OPENSL_AUDIO_MANAGER_HEAD_FILE__
