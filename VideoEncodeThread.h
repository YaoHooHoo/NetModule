//////////////////////////////////////////////////////////////////////////////////////////
//		视频编码类头文件 VideoEncodeThread.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_VIDEO_ENC_THREAD_HEAD_FILE__
#define __BOWEN_HU_VIDEO_ENC_THREAD_HEAD_FILE__

#include "localdef.h"
#include "../x264Include/libx264Encode.h"

///////////////////////////////////////////////////////////////////////////////////////////
class CVideoEncodeThread:public CServiceThread,public IVideoAudioHead
{
private:
	X264HANDLE			m_x264Handle;				//编码句柄
	BYTE				*m_pFrameData;				//视频帧缓冲
	BYTE				*m_pTmpBuffer;				//临时缓冲
	BYTE				*m_pEncodeImg;				//编码图片数据
	BYTE				*m_pZoomBuff;				//缩放缓冲
	CThreadLock			m_EncodeLock;				//编码锁

private:
	int					m_nImgWidth;				//图片宽
	int					m_nImgHeight;				//图片高
	int					m_nImgSize;					//图片大小
	int					m_nFrameCX;					//采集帧宽
	int					m_nFrameCY;					//采集帧高
	int					m_nFrameSize;				//帧大小
	int					m_nZoomCX;					//缩放宽
	int					m_nZoomCY;					//缩放高
	int					m_nZoomSize;				//缩放大小
	int					m_nOffsetX;					//X偏移量
	int					m_nOffsetY;					//Y偏移量

private:
	sem_t               m_Semt;						//信号量

private:
	IMideaDataSink		*m_pIMideaDataSink;			//回调接口

public:
	CVideoEncodeThread();
	~CVideoEncodeThread();

public:
	//设置视频大小
	void SetVideoSize(int codecx,int codecy,int screencx,int screency);
	//初始化
	bool InitEncodeThread(IMideaDataSink *pIMideaDataSink);
	//开始编码线程
	bool StartVideoThread();
	//停止编码线程
	bool StopVideoThread();
	//添加帧图片
	bool AddFrameImage(BYTE *pImageData,int cx,int cy);

protected:
     //开始通知函数
     virtual bool OnThreadStartEvent();
     //线程停止通知函数
     virtual bool OnThreadStopEvent();
     
     //Inside Function
protected:
     //线程体函数
     virtual bool RepetitionRun();

public:
	 //获取x264头
	virtual int GetSpsPpsData(BYTE *pHeadData,int nBufferSize);
	//获取AAC头
	virtual int GetAacHeadData(BYTE *pHeadData,int nBufferSize);
};

//////////////////////////////////////////////////////////////////////////
//双线性插值图片缩放
void ResizeLinearImgZoom(BYTE* pSrc,int sw,int sh,BYTE* pDst,int dw,int dh);
void ResizeImage(unsigned char* pSrc,int src_w,int src_h,unsigned char* pDst,int dst_w, int dst_h);
#endif //__BOWEN_HU_VIDEO_ENC_THREAD_HEAD_FILE__
