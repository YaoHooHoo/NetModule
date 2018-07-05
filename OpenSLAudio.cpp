//////////////////////////////////////////////////////////////////////////////////////////
//		Android OpenSLAudio音频管理类实现文件 OpenSLAudio.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/OpenSLAudio"
#include "log.h"
#include "OpenSLAudio.h"

///////////////////////////////////////////////////////////////////////////////////////////
COpenSLAudio::COpenSLAudio()
{
	m_bInited=false;
	m_bRecInited=false;
	m_bRecording=false;
	engineObject=NULL;
	engineEngine=NULL;
	recorderObject=NULL;

	m_AacHandle=NULL;
	m_pcbPcmBuffer=NULL;
	m_dwPcmDataSize=0;

	m_bInitPlay=false;
	m_bIsPlaying=false;
	m_bPlayoutSpecified=false;
	m_bIsSpeadkerInited=false;
}

COpenSLAudio::~COpenSLAudio()
{
	StopRecording();

	if(engineObject!=NULL)
	{
		(*engineObject)->Destroy(engineObject);
		engineObject=NULL;
		engineEngine=NULL;
	}
	
	if(m_pcbPcmBuffer!=NULL)
	{
		free(m_pcbPcmBuffer);
	}

	m_bInited=false;
}

//初始化
bool COpenSLAudio::InitOpenSLAudio(IMideaDataSink *pIMideaDataSink)
{
	LOGI("---->>>>>>>---InitOpenSLAudio");

	if(pIMideaDataSink==NULL) return false;

	m_pIMideaDataSink=pIMideaDataSink;

	CThreadLockHandle LockHandle(&m_AudioLock);

	if(m_bInited)
	{
		LOGI("---->>>>>>>---InitOpenSLAudio was inited");
		return true;
	}

	SLEngineOption EngineOption[] = {
		{SL_ENGINEOPTION_THREADSAFE, static_cast<SLuint32>(SL_BOOLEAN_TRUE) },
	};
	//创建引擎对象
	//int32_t res = slCreateEngine(&engineObject, 0, EngineOption, 0, NULL, NULL);		//20180127	Bowen.hu
	int32_t res = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to create SL Engine Object: err:%d",res);
		return false;
	}

	//申请内存
	if ((*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE)!= SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to Realize SL Engine");
		return false;
	}

	//获取音频引擎接口
	if ((*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine) != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to get SL Engine interface");
		return false;
	}

	LOGI("---->>>>>>>---InitOpenSLAudio success");
	m_bInited=true;

	return true;
}

//初始化录音
bool COpenSLAudio::InitRecording()
{
	LOGI("---->>>>>>>---start InitRecording");

	CThreadLockHandle LockHandle(&m_AudioLock);

	if(!m_bInited)
	{
		LOGE("------------Not initialized");
		return false;
	}
	if(m_bRecording)
	{
		LOGE("------------Recording already started");
		return false;
	}
	if(m_bRecInited)
	{
		LOGE("------------Recording already initialized");
		return false;
	}
	if(engineObject==NULL || engineEngine==NULL)
	{
		LOGE("------------Recording object is NULL");
		return false;
	}
	SLDataLocator_IODevice micLocator = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
		SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
	SLDataSource audio_source = { &micLocator, NULL };

	SLDataLocator_AndroidSimpleBufferQueue simple_buf_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
			static_cast<SLuint32>(N_REC_QUEUE_BUFFERS)
		};
	SLDataSink audio_sink = { &simple_buf_queue, &record_pcm_ };

	record_pcm_.formatType = SL_DATAFORMAT_PCM;
	record_pcm_.numChannels = N_REC_CHANNELS;
	if (N_REC_SAMPLES_PER_SEC == 44000)
	{
		record_pcm_.samplesPerSec = 44100 * 1000;
	} 
	else 
	{
		record_pcm_.samplesPerSec = N_REC_SAMPLES_PER_SEC * 1000;
	}
	record_pcm_.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	record_pcm_.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
	if (1 == record_pcm_.numChannels)
	{
		record_pcm_.channelMask = SL_SPEAKER_FRONT_CENTER;
	}
	else if (2 == record_pcm_.numChannels)
	{
		record_pcm_.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	}
	else
	{
		LOGE("------------%d rec channels not supported",N_REC_CHANNELS);
	}
	record_pcm_.endianness = SL_BYTEORDER_LITTLEENDIAN;

	const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE };		//20180127 Bowen.hu		
	const SLboolean req[1] = {SL_BOOLEAN_TRUE };	//20180127 Bowen.hu		

	int32_t res = -1;
	res = (*engineEngine)->CreateAudioRecorder(engineEngine,
                                                 &recorderObject,
                                                 &audio_source,
                                                 &audio_sink,
                                                 1,
                                                 id,
                                                 req);
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to create Recorder");
		return false;
	}

	// Realizing the recorder in synchronous mode.
	res = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
	if (res != SL_RESULT_SUCCESS) 
	{
		LOGE("------------failed to realize Recorder");
		return false;
	}

	// Get the RECORD interface - it is an implicit interface
	res = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD,static_cast<void*>(&recorderRecord));
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to get Recorder interface");
		return false;
	}

	// Get the simpleBufferQueue interface
	res = (*recorderObject)->GetInterface(recorderObject,SL_IID_ANDROIDSIMPLEBUFFERQUEUE,static_cast<void*>(&recorderBufferQueue));
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to get Recorder Simple Buffer Queue");
		return false;
	}
	res = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue,RecorderSimpleBufferQueueCallback,this);
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to register Recorder Callback");
		return false;
	}

	LOGI("---->>>>>>>---InitRecording success");
	m_bRecInited=true;

	return true;
}

//开始录音
bool COpenSLAudio::StartRecording()
{
	LOGI("---->>>>>>>---start StartRecording");
	CThreadLockHandle LockHandle(&m_AudioLock);

	LoadAacLibrary();

	if(!m_bRecInited)
	{
		LOGE("------------Recording not initialized");
		return false;
	}
	if(m_bRecording)
	{
		LOGE("------------Recording already started");
		return false;
	}
	if(recorderRecord==NULL)
	{
		LOGE("------------RecordITF is NULL");
		return false;
	}
	if(recorderBufferQueue==NULL)
	{
		LOGE("------------Recorder Simple Buffer Queue is NULL");
		return false;
	}

	memset(m_rec_buf, 0, sizeof(m_rec_buf));
	memset(m_rec_buf, 0, sizeof(m_rec_enc_buf));
	uint32_t num_bytes =N_REC_CHANNELS * sizeof(int16_t) * N_REC_SAMPLES_PER_SEC / 100;

	while (!m_RecQueue.empty())
		m_RecQueue.pop();
	while(!m_EncodeQueue.empty())
		m_EncodeQueue.pop();
	while(!m_ReadyQueue.empty())
		m_ReadyQueue.pop();

	for(int i=0;i<N_REC_QUEUE_BUFFERS;i++)
	{
		m_ReadyQueue.push(m_rec_enc_buf[i]);
	}

	int32_t res = -1;
	for (int i = 0; i < N_REC_QUEUE_BUFFERS; ++i)
	{
		// We assign 10ms buffer to each queue, size given in bytes.
		res = (*recorderBufferQueue)->Enqueue(
			recorderBufferQueue,
			static_cast<void*>(m_rec_buf[i]),
			num_bytes);

		if (res != SL_RESULT_SUCCESS)
		{
			LOGE("------------Recorder Enqueue failed:%d,%d", i, res);
			break;
		} 
		else
		{
			m_RecQueue.push(m_rec_buf[i]);
		}
	}
	// Record the audio
	res = (*recorderRecord)->SetRecordState(recorderRecord,SL_RECORDSTATE_RECORDING);
	if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------failed to start recording");
		return false;
	}

	if(!StartThread())
	{
		LOGE("------------failed to start the rec audio thread");
		return false;
	}

	LOGI("---->>>>>>>---StartRecording success");
	m_bRecording=true;

	return true;
}

//停止录音
bool COpenSLAudio::StopRecording()
{
	LOGI("---->>>>>>>---start StopRecording");
	CThreadLockHandle LockHandle(&m_AudioLock);
	if(!m_bRecInited)
	{
		LOGE("------------Recording is not initialized");
		return false;
	}
	if(recorderObject!=NULL || recorderRecord!=NULL)
	{
		int32_t res = (*recorderRecord)->SetRecordState(
          recorderRecord,
          SL_RECORDSTATE_STOPPED);

		if (res != SL_RESULT_SUCCESS)
		{
			LOGE("------------failed to stop recording");
			return false;
		}
		res = (*recorderBufferQueue)->Clear(recorderBufferQueue);
		if (res != SL_RESULT_SUCCESS)
		{
			LOGE("------------failed to clear recorder buffer queue");
			return false;
		}
		// Destroy the recorder object
		(*recorderObject)->Destroy(recorderObject);
		recorderObject = NULL;
		recorderRecord = NULL;
	}

	StopThread(1);

	if(m_AacHandle!=NULL)
	{
		CloseAacEndcoder(m_AacHandle);
	}

	m_bRecInited=false;
	m_bRecording=false;

	LOGI("---->>>>>>>---StopRecording finished");

	return true;
}

//开始通知函数
bool COpenSLAudio::OnThreadStartEvent()
{
	return true;
}

//线程停止通知函数
bool COpenSLAudio::OnThreadStopEvent()
{
	return true;
}
     
//线程体函数
bool COpenSLAudio::RepetitionRun()
{
	usleep(20);
	const unsigned int num_samples = N_REC_SAMPLES_PER_SEC / 100;
    const unsigned int num_bytes =
        N_REC_CHANNELS * num_samples * sizeof(int16_t);
    const unsigned int total_bytes = num_bytes;
    int8_t buf[REC_MAX_TEMP_BUF_SIZE_PER_10ms];
	if(m_bRecording && m_bRun) 
	{
		CThreadLockHandle LockHandle(&m_AudioLock);
		if(m_EncodeQueue.size()<=0)
		{
			LockHandle.UnLock();
			usleep(500);
			return m_bRun;
		}
		int8_t* audio = m_EncodeQueue.front();
		m_EncodeQueue.pop();
		memcpy(buf, audio, total_bytes);
		memset(audio, 0, total_bytes);
		m_ReadyQueue.push(audio);
		LockHandle.UnLock();

		UpdateRecordingDelay();
		//开始编码
		EncodeAacData(buf,num_bytes);
	}	

	return m_bRun;
}

//录音缓冲函数
void COpenSLAudio::RecorderSimpleBufferQueueCallbackHandler(SLAndroidSimpleBufferQueueItf queueItf)
{
	if(!m_bRecording) return;

	const unsigned int num_samples = N_REC_SAMPLES_PER_SEC / 100;
    const unsigned int num_bytes =
        N_REC_CHANNELS * num_samples * sizeof(int16_t);
    const unsigned int total_bytes = num_bytes;
    int8_t* audio;

	CThreadLockHandle LockHandle(&m_AudioLock);

	//LOGI("------====------get Audio data:%d",num_bytes);
	audio = m_RecQueue.front();
    m_RecQueue.pop();

	//投递到编码缓冲
	m_EncodeQueue.push(audio);

	if(m_ReadyQueue.size()<=0)
	{
		m_RecError=1;
		LOGE("------------Audio Rec thread buffers underrun");
	}
	else
	{
		audio=m_ReadyQueue.front();
		m_ReadyQueue.pop();
	}

	int32_t res = (*queueItf)->Enqueue(queueItf,audio,total_bytes);
    if (res != SL_RESULT_SUCCESS)
	{
		LOGE("------------recorder callback Enqueue failed, %d", res);
		m_RecWarning = 1;
		return;
    } 
	else
	{
		m_RecQueue.push(audio);
    }
}

//录音缓冲回调函数
void COpenSLAudio::RecorderSimpleBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf,void *pContext)
{
	COpenSLAudio* audio_device =static_cast<COpenSLAudio*>(pContext);
	audio_device->RecorderSimpleBufferQueueCallbackHandler(queueItf);
}

//更新录音延迟
void COpenSLAudio::UpdateRecordingDelay()
{
	m_nRecordingDelay = 10;
	const uint32_t noSamp10ms = N_REC_SAMPLES_PER_SEC / 100;
	m_nRecordingDelay += (N_REC_QUEUE_BUFFERS * noSamp10ms) /(N_REC_SAMPLES_PER_SEC / 1000);
}

//加载AAC编码库
bool COpenSLAudio::LoadAacLibrary()
{
	m_AacHandle=CreateAacEncoder(N_REC_SAMPLES_PER_SEC,N_REC_CHANNELS);	
	if(m_AacHandle==NULL)
	{
		LOGE("------------CreateAacEncoder failed");
	}

	m_pcbPcmBuffer=(BYTE *)::malloc(AAC_ENCODE_SIZE*2);

	return (m_AacHandle!=NULL);
}

//编码音频数据
int COpenSLAudio::EncodeAacData(int8_t *pAudioData,DWORD dwDataSize)
{
	BYTE cbAacBuffer[AAC_ENCODE_SIZE*2];
	DWORD dwLastSize=0;
	
	BYTE *pSrcData=(BYTE *)pAudioData;
	while(dwDataSize>0)
	{
		if(m_dwPcmDataSize+dwDataSize>=AAC_ENCODE_SIZE)
		{
			dwLastSize=AAC_ENCODE_SIZE-m_dwPcmDataSize;
		}
		else
		{
			dwLastSize=dwDataSize;
		}
		memcpy(m_pcbPcmBuffer+m_dwPcmDataSize,pSrcData,dwLastSize);
		pSrcData+=dwLastSize;
		m_dwPcmDataSize+=dwLastSize;
		dwDataSize-=dwLastSize;
		if(m_dwPcmDataSize>=AAC_ENCODE_SIZE)
		{
			//AAC编码
			int nRet=EndcodePcm2Aac(m_AacHandle,m_pcbPcmBuffer,AAC_ENCODE_SIZE,cbAacBuffer,sizeof(cbAacBuffer));
		 
			if(nRet>0 &&m_pIMideaDataSink!=NULL)	//编码成功
			{
				m_pIMideaDataSink->OnAacAudioData(cbAacBuffer,nRet);
			}
			//LOGI("-----<<<<<<<<-----Aac encode data size:%d",nRet);

			m_dwPcmDataSize-=AAC_ENCODE_SIZE;
			if(m_dwPcmDataSize>0)
			{
				memcpy(m_pcbPcmBuffer,m_pcbPcmBuffer+AAC_ENCODE_SIZE,m_dwPcmDataSize);
			}
		}
	}

	return 0;
}

/////////////////////////////////////////

bool COpenSLAudio::InitSpeaker() {
  
	if (m_bIsPlaying)
	{
		LOGE("------------Playout already started");
		return false;
	}

	if(!m_bPlayoutSpecified)
	{
		LOGE("------------Playout device is not specified");
		return false;
	}

	m_bIsSpeadkerInited = true;

	return true;
}

//初始化播放
bool COpenSLAudio::InitPlayout()
{
	LOGI("---->>>>>>>---start InitRecording");

	CThreadLockHandle LockHandle(&m_AudioLock);

	if(!m_bInited)
	{
		LOGE("------------Not initialized");
		return false;
	}

	if(m_bInitPlay)
	{
		LOGE("------------Playout already initialized");
		return true;
	}

	// Initialize the speaker
	if (InitSpeaker() == -1) 
	{
		LOGE("------------InitSpeaker() failed");
		return false;
	}

	if(engineObject==NULL || engineEngine==NULL)
	{
		LOGE("------------SLObject or Engiine is NULL");
		return false;
	}

	SLDataLocator_AndroidSimpleBufferQueue simple_buf_queue = {
    SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
    static_cast<SLuint32>(N_PLAY_QUEUE_BUFFERS)
	};

	SLDataSource audio_source = { &simple_buf_queue, &player_pcm_ };
	SLDataLocator_OutputMix locator_outputmix;
	SLDataSink audio_sink = { &locator_outputmix, NULL };

	// Create Output Mix object to be used by player.
	int32_t res = -1;


	return true;
}


