//////////////////////////////////////////////////////////////////////////////////////////
//		��Ƶ������ͷ�ļ� VideoEncodeThread.h
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
	X264HANDLE			m_x264Handle;				//������
	BYTE				*m_pFrameData;				//��Ƶ֡����
	BYTE				*m_pTmpBuffer;				//��ʱ����
	BYTE				*m_pEncodeImg;				//����ͼƬ����
	BYTE				*m_pZoomBuff;				//���Ż���
	CThreadLock			m_EncodeLock;				//������

private:
	int					m_nImgWidth;				//ͼƬ��
	int					m_nImgHeight;				//ͼƬ��
	int					m_nImgSize;					//ͼƬ��С
	int					m_nFrameCX;					//�ɼ�֡��
	int					m_nFrameCY;					//�ɼ�֡��
	int					m_nFrameSize;				//֡��С
	int					m_nZoomCX;					//���ſ�
	int					m_nZoomCY;					//���Ÿ�
	int					m_nZoomSize;				//���Ŵ�С
	int					m_nOffsetX;					//Xƫ����
	int					m_nOffsetY;					//Yƫ����

private:
	sem_t               m_Semt;						//�ź���

private:
	IMideaDataSink		*m_pIMideaDataSink;			//�ص��ӿ�

public:
	CVideoEncodeThread();
	~CVideoEncodeThread();

public:
	//������Ƶ��С
	void SetVideoSize(int codecx,int codecy,int screencx,int screency);
	//��ʼ��
	bool InitEncodeThread(IMideaDataSink *pIMideaDataSink);
	//��ʼ�����߳�
	bool StartVideoThread();
	//ֹͣ�����߳�
	bool StopVideoThread();
	//���֡ͼƬ
	bool AddFrameImage(BYTE *pImageData,int cx,int cy);

protected:
     //��ʼ֪ͨ����
     virtual bool OnThreadStartEvent();
     //�߳�ֹ֪ͣͨ����
     virtual bool OnThreadStopEvent();
     
     //Inside Function
protected:
     //�߳��庯��
     virtual bool RepetitionRun();

public:
	 //��ȡx264ͷ
	virtual int GetSpsPpsData(BYTE *pHeadData,int nBufferSize);
	//��ȡAACͷ
	virtual int GetAacHeadData(BYTE *pHeadData,int nBufferSize);
};

//////////////////////////////////////////////////////////////////////////
//˫���Բ�ֵͼƬ����
void ResizeLinearImgZoom(BYTE* pSrc,int sw,int sh,BYTE* pDst,int dw,int dh);
void ResizeImage(unsigned char* pSrc,int src_w,int src_h,unsigned char* pDst,int dst_w, int dst_h);
#endif //__BOWEN_HU_VIDEO_ENC_THREAD_HEAD_FILE__
