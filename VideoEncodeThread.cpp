//////////////////////////////////////////////////////////////////////////////////////////
//		视频编码类实现文件 VideoEncodeThread.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/VideoThread"
#include "log.h"
#include "VideoEncodeThread.h"
#include <math.h>

/////////////////////////////////////////////////////
int		*m_arr_x1;					//缩放查询表
int		*m_arr_x2;					//缩放查询表
float	*m_arr_fx1;					//缩放查询表
///////////////////////////////////////////////////////////////////////////////////////////
CVideoEncodeThread::CVideoEncodeThread()
{
	m_x264Handle=NULL;
	m_pFrameData=NULL;
	m_pTmpBuffer=NULL;
	m_pEncodeImg=NULL;
	m_pZoomBuff=NULL;
	m_nFrameSize=0;
	m_nImgSize=0;
	m_pIMideaDataSink=NULL;

	m_nImgWidth=V_WIDTH;
	m_nImgHeight=V_HEIGHT;
	m_nFrameCX=V_WIDTH;
	m_nFrameCY=V_HEIGHT;

	m_nOffsetX=0;
	m_nOffsetY=0;

	m_arr_x1=NULL;
	m_arr_x2=NULL;
	m_arr_fx1=NULL;
}

CVideoEncodeThread::~CVideoEncodeThread()
{
	if(m_pFrameData!=NULL)
	{
		free(m_pFrameData);
	}
	if(m_pZoomBuff!=NULL)
	{
		free(m_pZoomBuff);
	}
	if(m_pTmpBuffer!=NULL)
	{
		free(m_pTmpBuffer);
	}
	if(m_pEncodeImg!=NULL)
	{
		free(m_pEncodeImg);
	}
	if(m_arr_x1!=NULL) delete [] m_arr_x1;
	if(m_arr_x2!=NULL) delete [] m_arr_x2;
	if(m_arr_fx1!=NULL) delete [] m_arr_fx1;
 
	sem_destroy(&m_Semt);
}

//设置视频大小
void CVideoEncodeThread::SetVideoSize(int codecx,int codecy,int screencx,int screency)
{
	LOGI("----=========---SetVideoSize:code size:%dx%d,screen size:%dx%d",codecx,codecy,screencx,screency);

	if(screencx==0 || screency==0)
		return;
	if(codecx==0 || codecy==0)
		return;

	m_nFrameCX=screencx;
	m_nFrameCY=screency;
	if(codecx>V_WIDTH)
	{
		m_nImgWidth=V_WIDTH;
	}
	else
	{
		m_nImgWidth=codecx;
	}
	double dbScreen=(double)screencx/(double)screency;

	m_nImgWidth=((int)(m_nImgWidth/16))*16;	
	m_nImgHeight=m_nImgWidth/dbScreen;
	m_nImgHeight=((int)m_nImgHeight/16)*16;

	double dbImage=(double)m_nImgWidth/(double)m_nImgHeight;
	//m_nZoomCX
	if(dbScreen>dbImage)
	{
		m_nZoomCY=m_nImgHeight;
		m_nZoomCX=(int)(m_nZoomCY*dbScreen);
	}
	else
	{
		m_nZoomCX=m_nImgWidth;
		m_nZoomCY=(int)(m_nZoomCX/dbScreen);
	}

	m_nOffsetX=(m_nImgWidth-m_nZoomCX)/2;
	m_nOffsetY=(m_nImgHeight-m_nZoomCY)/2;
	LOGI("----=========---SetVideoSize success:%dx%d,zoom size:%dx%d,offset:x:%d,y:%d",m_nImgWidth,m_nImgHeight,m_nZoomCX,m_nZoomCY,m_nOffsetX,m_nOffsetY);
}

//初始化
bool CVideoEncodeThread::InitEncodeThread(IMideaDataSink *pIMideaDataSink)
{
	if(pIMideaDataSink==NULL) return false;

	LOGI("----=========---InitEncodeThread");

	m_pIMideaDataSink=pIMideaDataSink;

	int nRet=sem_init(&m_Semt,0,0);
    if(nRet==-1)
    {
		LOGE("----=========---Initializate sem_t object fail");
        return false;
    }
	LOGI("----=========---InitEncodeThread success");
	return true;
}

//开始编码线程
bool CVideoEncodeThread::StartVideoThread()
{
	LOGI("----1=========---StartVideoThread");

	x264_enc_create enc;
	memset(&enc,0,sizeof(enc));
	enc.width=m_nImgWidth;
	enc.height=m_nImgHeight;
	enc.i_keyint_max=25;
	LOGI("----2=========---X264EncCreate");
	//enc.num_threads=4;
	m_x264Handle=X264EncCreate(&enc);
	if(m_x264Handle==NULL)
	{
		LOGE("----=========---X264EncCreate fail");
		return false;
	}
	m_nImgSize=m_nImgWidth*m_nImgHeight*3;
	m_nFrameSize=m_nFrameCX*m_nFrameCY*3;
	m_nZoomSize=m_nZoomCX*m_nZoomCY*3;
	if(m_pFrameData!=NULL)
	{
		LOGI("----3,3=========---free frame buffer");
		free(m_pFrameData);
		m_pFrameData=NULL;
	}
	LOGI("----3=========---malloc frame buffer");
	m_pFrameData=(BYTE *)malloc(m_nFrameSize);
	if(m_pFrameData==NULL)
	{
		LOGE("----=========---m_pFrameData malloc faild");
		return false;
	}
	 
	if(m_pZoomBuff!=NULL)
	{
		LOGI("----4,4=========---free zoom buffer");
		free(m_pZoomBuff);
		m_pZoomBuff=NULL;
	}
	LOGI("----4=========---malloc zoom buffer");
	m_pZoomBuff=(BYTE *)malloc(m_nFrameSize);
	if(m_pZoomBuff==NULL)
	{
		LOGE("----=========---m_pZoomBuff malloc faild");
		return false;
	}
	 
	if(m_pTmpBuffer!=NULL)
	{
		free(m_pTmpBuffer);
		m_pTmpBuffer=NULL;
	}
	LOGI("----5=========---malloc tmp buffer");
	m_pTmpBuffer=(BYTE *)malloc(m_nZoomSize);
	if(m_pTmpBuffer==NULL)
	{
		LOGE("----=========---m_pTmpBuffer malloc faild");
		return false;
	}
	
	if(m_pEncodeImg!=NULL)
	{
		LOGI("----6,6=========---free encoder buffer");
		free(m_pEncodeImg);
		m_pEncodeImg=NULL;
	}
	LOGI("----6=========---malloc encoder buffer");
	m_pEncodeImg=(BYTE *)malloc(m_nImgSize);
	if(m_pEncodeImg==NULL)
	{
		LOGE("----=========---m_pEncodeImg malloc faild");
		return false;
	}
	if(m_arr_x1!=NULL) delete [] m_arr_x1;
	if(m_arr_x2!=NULL) delete [] m_arr_x2;
	if(m_arr_fx1!=NULL) delete [] m_arr_fx1;

	m_arr_x1=new int[m_nZoomCX];
	m_arr_x2=new int[m_nZoomCX];
	m_arr_fx1=new float[m_nZoomCX];

	int x0=0;
	int w1=m_nZoomCX;
	int h1=m_nZoomCY;
	int w0=m_nFrameCX;
	int h0=m_nFrameCY;
	float fw = float(w0-1) / (w1-1);
    float fh = float(h0-1) / (h1-1);
	for(int x=0; x<w1; x++)
    {
        x0 = x*fw;
        m_arr_x1[x] = int(x0);
        m_arr_x2[x] = int(x0+0.5f);
        m_arr_fx1[x] = x0 - m_arr_x1[x];
    }

	LOGI("----7=========---StartThread encoder thread");
	bool bSuccess=StartThread();
	if(!bSuccess)
	{
		LOGE("----=========---Video Encode Thread faild");
	}
	else
	{
		LOGI("----=========---StartVideoThread success,ImageSize:%u,FrameSize:%u,ZoomSize:%u",m_nImgSize,m_nFrameSize,m_nZoomSize);
	}

	return bSuccess;
}

//停止编码线程
bool CVideoEncodeThread::StopVideoThread()
{
	m_bRun=false;
	sem_post(&m_Semt);
	CThreadLockHandle LockHandle(&m_EncodeLock);
	
	bool bStoped=StopThread(1);
	if(m_x264Handle!=NULL)
	{
		X264EncDestroy(m_x264Handle);
		m_x264Handle=NULL;
	}
	return bStoped;
}

//添加帧图片
bool CVideoEncodeThread::AddFrameImage(BYTE *pImageData,int cx,int cy)
{
	if(!m_bRun) return true;
	CThreadLockHandle LockHandle(&m_EncodeLock);
	
	if(cx!=m_nFrameCX || cy!=m_nFrameCY) 
	{
		LOGE("-----------AddFrameImage Image size:cx is big,cx--cy:%d--%d",cx,cy);
		return false;
	}
	memcpy(m_pFrameData,pImageData,m_nFrameSize);
	LockHandle.UnLock();
	sem_post(&m_Semt);

	return true;
}

//开始通知函数
bool CVideoEncodeThread::OnThreadStartEvent()
{
	srand((unsigned)time(NULL));
	//获取SpsPps数据
	const int BUFSIZE=1024;
	BYTE cbSpsPps[BUFSIZE];
	int nRet=GetSpsPpsData(cbSpsPps,BUFSIZE);
	if(nRet>0 && m_pIMideaDataSink!=NULL)
	{
		m_pIMideaDataSink->OnS264SpsPpsData(cbSpsPps,nRet);
	}

	return true;
}

//线程停止通知函数
bool CVideoEncodeThread::OnThreadStopEvent()
{
	return true;
}
     
//线程体函数
bool CVideoEncodeThread::RepetitionRun()
{
	sem_wait(&m_Semt);
	if(!m_bRun) return false;
	CThreadLockHandle LockHandle(&m_EncodeLock);
	memcpy(m_pZoomBuff,m_pFrameData,m_nFrameSize);
	int nSrcBand=0;
	int nDstBand=0;
	int nTop=m_nOffsetY;
	int nDstHeight=0;
	BYTE *pOffSrc=NULL;
	BYTE *pOffDst=NULL;

	LockHandle.UnLock();

	BYTE *pSrcData=m_pZoomBuff;
	if(m_nImgSize!=m_nFrameSize)
	{
		//缩放图片		
		ResizeLinearImgZoom(m_pZoomBuff,m_nFrameCX,m_nFrameCY,m_pTmpBuffer,m_nZoomCX,m_nZoomCY);
		pSrcData=m_pTmpBuffer;
		if(m_nOffsetX>0)
		{
			nSrcBand=m_nZoomCX*3;
			nDstBand=m_nImgWidth*3;
			pOffSrc=m_pTmpBuffer;
			nDstHeight=m_nZoomCY;
			if(nTop<0)
			{
				pOffSrc+=nSrcBand*abs(nTop);	//向下偏移 
				nTop=0;
				nDstHeight=m_nImgHeight;
			}
			pOffDst=m_pZoomBuff+nDstBand*nTop;
			for(int y=0;y<nDstHeight;y++)
			{
				memcpy(pOffDst+m_nOffsetX,pOffSrc,nSrcBand);	//拷贝到偏移位置
				pOffSrc+=nSrcBand;
				pOffDst+=nDstBand;
			}
			pSrcData=m_pZoomBuff;
		}
		else if(m_nOffsetY>0)
		{
			pOffSrc=m_pTmpBuffer;
			pOffDst=m_pZoomBuff+nDstBand*nTop;
			memcpy(pOffDst,pOffSrc,m_nZoomSize);
			pSrcData=m_pZoomBuff;
		}
		else if(m_nOffsetY<0)
		{
			nSrcBand=m_nZoomCX*3;
			nDstBand=m_nImgWidth*3;
			pOffSrc=m_pTmpBuffer;
			pOffSrc+=nSrcBand*abs(nTop);	//向下偏移 
			pOffDst=m_pZoomBuff;
			memcpy(pOffDst,pOffSrc,m_nImgSize);
			pSrcData=m_pZoomBuff;		 
		}
		else if(m_nOffsetX<0)
		{
			nSrcBand=m_nZoomCX*3;
			nDstBand=m_nImgWidth*3;
			pOffSrc=m_pTmpBuffer;
			nDstHeight=m_nZoomCY;
			if(nTop<0)
			{
				pOffSrc+=nSrcBand*abs(nTop);	//向下偏移
				nTop=0;
				nDstHeight=m_nImgHeight;
			}
			pOffDst=m_pZoomBuff+nDstBand*nTop;
			int nLeft=abs(m_nOffsetX);
			for(int y=0;y<nDstHeight;y++)
			{
				memcpy(pOffDst,pOffSrc+nLeft,nDstBand);	//拷贝到偏移位置
				pOffSrc+=nSrcBand;
				pOffDst+=nDstBand;
			}
			pSrcData=m_pZoomBuff;
		}
	}
	if(!m_bRun) return false;
	/*for(int i=0;i<m_nImgWidth*m_nImgHeight;i+=3)
	{
		m_pTmpBuffer[i]=200+rand()%50;
		m_pTmpBuffer[i+1]=rand()%128;
		m_pTmpBuffer[i+2]=200+rand()%50;
	}*/

	//H264编码
	int nOutSize=m_nFrameSize;
	int nRet=X264EncodeFrame(m_x264Handle,pSrcData,m_pEncodeImg,&nOutSize);
	if(!m_bRun) return false;
	if(nRet>0 && m_pIMideaDataSink!=NULL)
	{
		BYTE *pEncData=(BYTE *)m_pEncodeImg;
		if(nRet>4 && pEncData[0]==0 && pEncData[1]==0 && pEncData[2]==0 && pEncData[3]==1)
		{
			pEncData+=4;
			nRet-=4;
		}
		m_pIMideaDataSink->On264VideoData(pEncData,nRet);
	}

	return m_bRun;
}

//获取x264头
int CVideoEncodeThread::GetSpsPpsData(BYTE *pHeadData,int nBufferSize)
{
	return X264GetSPSorPPSHead(m_x264Handle,pHeadData,nBufferSize);
}

//获取AAC头
int CVideoEncodeThread::GetAacHeadData(BYTE *pHeadData,int nBufferSize)
{
	return true;
}

////////////////////////////////////////////////////////////
//双线性插值图片缩放
void ResizeLinearImgZoom(BYTE* pSrc,int sw,int sh,BYTE* pDst,int dw,int dh)
{
    int w0 = sw;
    int h0 = sh;
	const int pix0=3;
    int pitch0 = sw*pix0;
	
    int w1 = dw;
    int h1 = dh;
	const int pix1=3;
    int pitch1 = dw*pix1;

    BYTE *p1 = pDst;
    float x0, y0;
    int y1, y2, x1, x2;
    float fx1, fx2, fy1, fy2;
    
    float fh = float(h0-1) / (h1-1);

    int* arr_x1 = m_arr_x1;
    int* arr_x2 = m_arr_x2;
    float* arr_fx1 = m_arr_fx1;

    for(int y=0; y<h1; y++)
    {
        y0 = y*fh;
        y1 = int(y0);
        y2 = int(y0+0.5f);
        fy1 = y0-y1; 
        fy2 = 1.0f - fy1;        
        for(int x=0; x<w1; x++)
        {
            x1 = arr_x1[x];
            x2 = arr_x2[x];
            fx1 = arr_fx1[x];
            fx2 = 1.0f-fx1;

            float s1 = fx2*fy2;
            float s2 = fx1*fy2;
            float s3 = fx1*fy1;
            float s4 = fx2*fy1;
            BYTE* p11 = pSrc + pitch0*y1 + pix0*x1;
            BYTE* p12 = pSrc + pitch0*y1 + pix0*x2;
            BYTE* p21 = pSrc + pitch0*y2 + pix0*x1;
            BYTE* p22 = pSrc + pitch0*y2 + pix0*x2;
            
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1++;
			p11++;
			p12++;
			p21++;
			p22++;
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1++;
			p11++;
			p12++;
			p21++;
			p22++;
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3); 
			p1++;

        }
        p1 = pDst + y*pitch1;
    }
}

void ResizeLinearImgZoom4(BYTE* pSrc,int sw,int sh,BYTE* pDst,int dw,int dh)
{
    int w0 = sw;
    int h0 = sh;
	const int pix0=4;
    int pitch0 = sw*pix0;
	
    int w1 = dw;
    int h1 = dh;
	const int pix1=4;
    int pitch1 = dw*pix1;

    BYTE *p1 = pDst;

    float fw = float(w0-1) / (w1-1);
    float fh = float(h0-1) / (h1-1);

    float x0, y0;
    int y1, y2, x1, x2;
    float fx1, fx2, fy1, fy2;
    
    int* arr_x1 = new int[w1];
    int* arr_x2 = new int[w1];
    float* arr_fx1 = new float[w1];

    for(int x=0; x<w1; x++)
    {
        x0 = x*fw;
        arr_x1[x] = int(x0);
        arr_x2[x] = int(x0+0.5f);
        arr_fx1[x] = x0 - arr_x1[x];
    }

    for(int y=0; y<h1; y++)
    {
        y0 = y*fh;
        y1 = int(y0);
        y2 = int(y0+0.5f);
        fy1 = y0-y1; 
        fy2 = 1.0f - fy1;        
        for(int x=0; x<w1; x++)
        {
            x1 = arr_x1[x];
            x2 = arr_x2[x];
            fx1 = arr_fx1[x];
            fx2 = 1.0f-fx1;

            float s1 = fx2*fy2;
            float s2 = fx1*fy2;
            float s3 = fx1*fy1;
            float s4 = fx2*fy1;            
            BYTE* p11 = pSrc + pitch0*y1 + pix0*x1;
            BYTE* p12 = pSrc + pitch0*y1 + pix0*x2;
            BYTE* p21 = pSrc + pitch0*y2 + pix0*x1;
            BYTE* p22 = pSrc + pitch0*y2 + pix0*x2;
            
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);    p1++;    p11++; p12++; p21++; p22++;
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);    p1++;    p11++; p12++; p21++; p22++;
            *p1 = BYTE((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);  p1++;
			*p1 =0;	p1++;
        }
        p1 = pDst + y*pitch1;
    }

    delete []arr_x1;
    delete []arr_x2;
    delete []arr_fx1;
}

//////////////////////////////////////////////////////////////////
static void _ieInterpImageBilinear8UC1_Ver3_RowFilter(unsigned char* src, long* dst, int len, int* leftIdx, int* rightIdx, long* weight, int shift)
{
    int i;
    for(i = 0; i < len - 4; i+=4) {
        *dst++ = ((1<<shift) - weight[i])*src[leftIdx[i]] + weight[i]*src[rightIdx[i]];
        *dst++ = ((1<<shift) - weight[i+1])*src[leftIdx[i+1]] + weight[i+1]*src[rightIdx[i+1]];
        *dst++ = ((1<<shift) - weight[i+2])*src[leftIdx[i+2]] + weight[i+2]*src[rightIdx[i+2]];
        *dst++ = ((1<<shift) - weight[i+3])*src[leftIdx[i+3]] + weight[i+3]*src[rightIdx[i+3]];
  
    }
    for( ; i < len; ++i) {
        *dst++ = ((1<<shift) - weight[i])*src[leftIdx[i]] + weight[i]*src[rightIdx[i]];
    }
}

#define IET_MAX(x,y) (x)>(y)?(x):(y)
#define IET_MIN(x,y) (x)>(y)?(y):(x)
#define IET_SWAP(x,y,tmp) (tmp)=(x);(x)=(y);(y)=(tmp);
void ResizeImage(unsigned char* pSrc,int src_w,int src_h,unsigned char* pDst,int dst_w, int dst_h)
{
    int i, j;
    int sw, sh, sstep;
    int dw, dh, dstep;
    unsigned char *sdata, *ddata;
    float horScaleRatio, verScaleRatio;
    long *rowBuf1, *rowBuf2;
    long *upLinePtr, *downLinePtr, *tempPtr;
    long *horWeight;
    int *horLeftIdx, *horRightIdx;
    int preVerUpIdx, preVerDownIdx;
    int shift = 8;
  
    sw=src_w;
    sh=src_h;
    sstep=24;
    sdata=pSrc;
    dw=dst_w;
    dh=dst_h;
    dstep=24;
    ddata=pDst;
  
    horScaleRatio = sw / (float)(dw);
    verScaleRatio = sh / (float)(dh);
  
    rowBuf1 = new long[dw];
    rowBuf2 = new long[dw];
    horWeight = new long[dw];
    horLeftIdx = new int[dw];
    horRightIdx = new int[dw];
  
  
    //col interpolation
  
    //计算目标图像像素横向的左右邻居序号，和权重。
    for(i = 0; i < dw; i++) {
        float pos = (i + 0.5f) * horScaleRatio;
        horLeftIdx[i] = (int)(IET_MAX(pos - 0.5f, 0));
        horRightIdx[i] = (int)(IET_MIN(pos + 0.5f, sw-1));
        horWeight[i] = (long) (fabs(pos - 0.5f - horLeftIdx[i]) * (1<<shift));
    }
  
    preVerUpIdx = -1;
    preVerDownIdx = -1;
    upLinePtr = rowBuf1;
    downLinePtr = rowBuf2;
    for(j = 0; j < dh; j++) {
        float pos = (j + 0.5f) * verScaleRatio;
        int verUpIdx = (int)(IET_MAX(pos - 0.5f, 0));
        int verDownIdx = (int)(IET_MIN(pos + 0.5f, sh-1));
        long verWeight = (long) (fabs(pos - 0.5f - verUpIdx) * (1<<shift));
  
        if(verUpIdx == preVerUpIdx && verDownIdx == preVerDownIdx) {
            ;
            //do nothing
        } else if(verUpIdx == preVerDownIdx) {
            IET_SWAP(upLinePtr, downLinePtr, tempPtr);
            _ieInterpImageBilinear8UC1_Ver3_RowFilter(sdata + sstep*verDownIdx, downLinePtr, dw, horLeftIdx, horRightIdx, horWeight, shift);
        } else {
            _ieInterpImageBilinear8UC1_Ver3_RowFilter(sdata + sstep*verUpIdx,   upLinePtr, dw, horLeftIdx, horRightIdx, horWeight, shift);
            _ieInterpImageBilinear8UC1_Ver3_RowFilter(sdata + sstep*verDownIdx, downLinePtr, dw, horLeftIdx, horRightIdx, horWeight, shift);
        }
  
        unsigned char* _ptr = ddata + dstep*j;
        for(i = 0; i < dw-4; i+=4) {
            *_ptr++ = (unsigned char) ( (((1<<shift) - verWeight)*upLinePtr[i] + verWeight*downLinePtr[i]) >> (2*shift) );
            *_ptr++ = (unsigned char) ( (((1<<shift) - verWeight)*upLinePtr[i+1] + verWeight*downLinePtr[i+1]) >> (2*shift) );
            *_ptr++ = (unsigned char) ( (((1<<shift) - verWeight)*upLinePtr[i+2] + verWeight*downLinePtr[i+2]) >> (2*shift) );
            *_ptr++ = (unsigned char) ( (((1<<shift) - verWeight)*upLinePtr[i+3] + verWeight*downLinePtr[i+3]) >> (2*shift) );
        }
        for(; i < dw; i++) {
            *_ptr++ = (unsigned char) ( (((1<<shift) - verWeight)*upLinePtr[i] + verWeight*downLinePtr[i]) >> (2*shift) );
        }
        preVerUpIdx = verUpIdx;
        preVerDownIdx = verDownIdx;
    }
    delete []rowBuf1;
    delete []rowBuf2;
    delete []horWeight;
    delete []horLeftIdx;
    delete []horRightIdx;
}
