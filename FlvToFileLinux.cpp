/////////////////////////////////////////////////////////////////////////////////////////
//		Linux 系统下 Flv 文件存储类实现文件 FlvToFileLinux.h
//		hubo 2014-12-25
/////////////////////////////////////////////////////////////////////////////////////////
#include "FlvToFileLinux.h"

/////////////////////////////////////////////////////////////////////////////////////////
CFlvToFileLinux::CFlvToFileLinux(void)
{
	m_szFileName[0]=0;
	RestFlvParam();
}

CFlvToFileLinux::~CFlvToFileLinux(void)
{
}

//重置参数
void CFlvToFileLinux::RestFlvParam()
{
	memset(&m_flvHeader,0,sizeof(m_flvHeader));
	
	m_fileInfo.duration.amfType=AMFT_NUMBER;
	m_fileInfo.duration.dbValue=0.0;
	strcpy_s(m_fileInfo.duration.szName,"duration");

	m_fileInfo.ffAudio.audiocodecid.amfType=AMFT_NUMBER;
	m_fileInfo.ffAudio.audiocodecid.dbValue=10.0;			//音频解码器ID
	strcpy_s(m_fileInfo.ffAudio.audiocodecid.szName,"audiocodecid");
	m_fileInfo.ffAudio.audiodatarate.amfType=AMFT_NUMBER;
	m_fileInfo.ffAudio.audiodatarate.dbValue=128.0;			//音频码率
	strcpy_s(m_fileInfo.ffAudio.audiodatarate.szName,"audiodatarate");
	m_fileInfo.ffAudio.audiosamplerate.amfType=AMFT_NUMBER;
	m_fileInfo.ffAudio.audiosamplerate.dbValue=44100.0;		//音频采样率
	strcpy_s(m_fileInfo.ffAudio.audiosamplerate.szName,"audiosamplerate");
	m_fileInfo.ffAudio.audiosamplesize.amfType=AMFT_NUMBER;
	m_fileInfo.ffAudio.audiosamplesize.dbValue=16.0;		//音频采样大小
	strcpy_s(m_fileInfo.ffAudio.audiosamplesize.szName,"audiosamplesize");
	m_fileInfo.ffAudio.stereo.amfType=AMFT_BOOLLEAN;
	m_fileInfo.ffAudio.stereo.bValue=1;						//音频是否立体声
	strcpy_s(m_fileInfo.ffAudio.stereo.szName,"stereo");

	m_fileInfo.ffVideo.width.amfType=AMFT_NUMBER;
	m_fileInfo.ffVideo.width.dbValue=DST_V_CX;
	strcpy_s(m_fileInfo.ffVideo.width.szName,"width");
	m_fileInfo.ffVideo.height.amfType=AMFT_NUMBER;
	m_fileInfo.ffVideo.height.dbValue=DST_V_CY;
	strcpy_s(m_fileInfo.ffVideo.height.szName,"height");
	m_fileInfo.ffVideo.videodatarate.amfType=AMFT_NUMBER;
	m_fileInfo.ffVideo.videodatarate.dbValue=400;
	strcpy_s(m_fileInfo.ffVideo.videodatarate.szName,"videodatarate");
	m_fileInfo.ffVideo.framerate.amfType=AMFT_NUMBER;
	m_fileInfo.ffVideo.framerate.dbValue=24.0;
	strcpy_s(m_fileInfo.ffVideo.framerate.szName,"framerate");
	m_fileInfo.ffVideo.videocodecid.amfType=AMFT_NUMBER;
	m_fileInfo.ffVideo.videocodecid.dbValue=7.0;
	strcpy_s(m_fileInfo.ffVideo.videocodecid.szName,"videocodecid");

	m_fileInfo.duration.amfType=AMFT_NUMBER;
	m_fileInfo.duration.dbValue=0.0;
	strcpy_s(m_fileInfo.filesize.szName,"filesize");

	m_dwPrevTagSize=0;
	m_dwVideoTime=0;
	m_dwAudioTime=0;
	m_nDelayTime=0;

	m_bHaveVideo=false;
	m_bHaveAudio=false;
}

//设置文件信息
bool CFlvToFileLinux::SetFileInfo(BYTE cbVer,bool bHaveVideo,bool bHaveAudio)
{
	m_flvHeader.cbHaveVideo=(BYTE)bHaveVideo;
	m_flvHeader.cbHaveAudio=(BYTE)bHaveAudio;
	m_flvHeader.cbVersion=cbVer;
	m_flvHeader.dwHeadLength=htonl(9);
	m_flvHeader.szType[0]='F';
	m_flvHeader.szType[1]='L';
	m_flvHeader.szType[2]='V';

	m_bHaveVideo=bHaveVideo;
	m_bHaveAudio=bHaveAudio;

	return true;
}

//设置视频信息
bool CFlvToFileLinux::SetVideoInfo(int cx,int cy,double cbDataRate,int nFrameRate)
{
	m_fileInfo.ffVideo.width.dbValue=cx;
	m_fileInfo.ffVideo.height.dbValue=cy;
	m_fileInfo.ffVideo.videodatarate.dbValue=cbDataRate;
	m_fileInfo.ffVideo.framerate.dbValue=nFrameRate;
	
	return true;
}

//设置音频信息
bool CFlvToFileLinux::SetAudioInfo(int nChannelNum,int nSampleRate)
{
	m_fileInfo.ffAudio.audiosamplerate.dbValue=nSampleRate;
	m_fileInfo.ffAudio.stereo.bValue=(nChannelNum>1);
	
	return true;
}

//创建文件
bool CFlvToFileLinux::CreateFlvFile(LPCTSTR lpszFileName)
{
	if(lpszFileName==NULL) return false;
	if(lpszFileName[0]==0) return false;
	if(m_flvHeader.szType[0]!='F') return false;

	m_dwPrevTagSize=0;
	m_nDelayTime=0;

	//创建文件
	bool bOpened=m_FlvFile.Open(lpszFileName,CMyFile::CREATE|CMyFile::WRITE|CMyFile::READ);
	if(!bOpened) return false;
	//写入头
	DWORD dwWrited=0;
	m_FlvFile.Write(&m_flvHeader,sizeof(m_flvHeader));
	
	m_dwStartTime=::GetTickCount();
	m_dwLastTime=m_dwStartTime;

	m_nDelayTime=500;
	m_fileInfo.filesize.dbValue+=dwWrited;

	int nSampleRate=(int)m_fileInfo.ffAudio.audiosamplerate.dbValue;
	m_cbRateIndex=GetTagRateIndex(nSampleRate);

	//写入文件信息tag
	if(!WriteFileInfoTag()) return false;

	if(m_bHaveAudio)
	{
		int nChannelNum=(m_fileInfo.ffAudio.stereo.bValue)?2:1;
		//m_nDelayTime+=((1024*1000)/(nSampleRate*nChannelNum*2))*3;
		SetAacHeader();
	}

	strcpy(m_szFileName,lpszFileName);

	return true;
}

//设置延时
void CFlvToFileLinux::SetDelayTime(DWORD dwDelayTime)
{
	m_nDelayTime+=dwDelayTime;
}

//写入文件信息tag
bool CFlvToFileLinux::WriteFileInfoTag()
{
	int nTagSize=0;
	 
	int nCount=2;		//AMF2个数
	int nAudioCount=sizeof(m_fileInfo.ffAudio)/sizeof(INFOITEM);
	int nVideoCount=sizeof(m_fileInfo.ffVideo)/sizeof(INFOITEM);
	if(m_bHaveAudio)
	{
		nCount+=nAudioCount;
	}
	if(m_bHaveVideo)
	{
		nCount+=nVideoCount;
	}

	INFOITEM *pItem=NULL;

	BYTE *pTagSizePos=NULL;
	BYTE cbTagData[2048];

	BYTE *pData=cbTagData;
	pData=AMF_EncodeInt32(pData,0);	//前一个tag大小
	*pData++=FT_SCRIPT;
	pTagSizePos=pData;
	pData=AMF_EncodeInt24(pData,nTagSize-15);	//tag数据大小
	pData=AMF_EncodeInt24(pData,0);				//时间戳
	*pData++=0;									//时间戳高位
	pData=AMF_EncodeInt24(pData,0);				//流ID
	pData=AMF_EncodeString(pData,"onMetaData");	//写入函数名

	*pData++=AMFT_ECMA_ARRAY;					//数组类型
	pData=AMF_EncodeInt32(pData,nCount);		//数组元数个数

	//添加时长
	pData=AMF_EncodeNamedNumber(pData,m_fileInfo.duration.szName,m_fileInfo.duration.dbValue);
	//添加音频信息
	if(m_bHaveAudio)
	{
		pItem=(INFOITEM *)&m_fileInfo.ffAudio;
		for(int i=0;i<nAudioCount;i++)
		{
			switch(pItem->amfType)
			{
			case AMFT_NUMBER:
				pData=AMF_EncodeNamedNumber(pData,pItem->szName,pItem->dbValue);
				break;
			case AMFT_BOOLLEAN:
				pData=AMF_EncodeNamedBoolean(pData,pItem->szName,pItem->bValue);
				break;
			case AMFT_STRING:
				pData=AMF_EncodeNamedString(pData,pItem->szName,pItem->szValue);
				break;
			}
			pItem++;
		}
	}
	//添加视频信息
	if(m_bHaveVideo)
	{
		pItem=(INFOITEM *)&m_fileInfo.ffVideo;
		for(int i=0;i<nVideoCount;i++)
		{
			switch(pItem->amfType)
			{
			case AMFT_NUMBER:
				pData=AMF_EncodeNamedNumber(pData,pItem->szName,pItem->dbValue);
				break;
			case AMFT_BOOLLEAN:
				pData=AMF_EncodeNamedBoolean(pData,pItem->szName,pItem->bValue);
				break;
			case AMFT_STRING:
				pData=AMF_EncodeNamedString(pData,pItem->szName,pItem->szValue);
				break;
			}
			pItem++;
		}
	}
	//添加文件大小
	pData=AMF_EncodeNamedNumber(pData,m_fileInfo.filesize.szName,m_fileInfo.filesize.dbValue);

	*pData++=0;
	*pData++=0;
	*pData++=AMFT_OBJECT_END;

	int nDataSize=(int)(pData-cbTagData);
	
	m_dwPrevTagSize=nDataSize-4;
	nTagSize=nDataSize-15;
	AMF_EncodeInt24(pTagSizePos,nTagSize);

	//写入文件
	m_FlvFile.SeekMidFromFirst(9);
	m_FlvFile.Write(cbTagData,nDataSize);
	DWORD dwWrited=nDataSize;

	m_fileInfo.filesize.dbValue+=dwWrited;

	return true;
}

//关闭文件
bool CFlvToFileLinux::CloseFlvFile()
{
	//写入结尾
	DWORD dwLastTime=m_dwLastTime;
	BYTE cbBuffer[3]={0,0,0};
	WriteVideoFrame(AVCPT_SEQEND,cbBuffer,3);
	DWORD dwWrited1=0;
	DWORD dwTagSize=htonl(m_dwPrevTagSize);
	m_FlvFile.Write(&dwTagSize,4);
	m_fileInfo.filesize.dbValue+=4;

	DWORD dwTime=dwLastTime-m_dwStartTime;
	m_fileInfo.duration.dbValue=dwTime/1000.0;

	WriteFileInfoTag();
	m_FlvFile.Close();

	RestFlvParam();

	return true;
}

//写Tag头
BYTE *CFlvToFileLinux::WriteTagHeader(BYTE *pData,BYTE cbTagType,DWORD dwDataSize,DWORD dwTime)
{
	int nID=cbTagType==FT_VIDEO?0:1;
	pData=AMF_EncodeInt32(pData,(int)m_dwPrevTagSize);	//前一个tag大小
	*pData++=cbTagType;
	pData=AMF_EncodeInt24(pData,(int)dwDataSize);	//tag数据大小
	pData=AMF_EncodeInt24(pData,dwTime);			//时间戳
	*pData++=0;										//时间戳高位
	pData=AMF_EncodeInt24(pData,nID);				//流ID
	return pData;
}

//添加视频帧
bool CFlvToFileLinux::WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize)
{
	DWORD dwTime=::GetTickCount();
	DWORD dwTime1=dwTime;
	DWORD dwTime2=dwTime1-m_dwLastTime;
	m_dwLastTime=dwTime1;
	dwTime1-=m_dwStartTime;
	if(dwTime1>m_dwVideoTime)
		dwTime1-=m_dwVideoTime;
	else
		dwTime1=0;
	if(dwTime2>m_dwVideoTime)
		dwTime2-=m_dwVideoTime;
	else
		dwTime2=0;
		
	bool bSuccess=WriteVideoFrame(avpt,pVideopData,dwDataSize,dwTime1,dwTime2);
	m_dwVideoTime=::GetTickCount()-dwTime;
	return bSuccess;
}

//添加视频帧
bool CFlvToFileLinux::WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime)
{
	CThreadLockHandle LockHandle(&m_FileLock);
	BYTE cbBuffer[64];
	BYTE *pData=cbBuffer;

	m_dwLastTime=m_dwStartTime+dwTagTime;

	DWORD dwTagSize=dwDataSize;
	if(avpt==AVCPT_NALU)
		dwTagSize+=9;
	else
		dwTagSize+=2;

	if(avpt==AVCPT_SEQHEADER)
	{
		dwTagTime=0;
		dwPackTime=0;
	}
	
	#if OUT_DEBUG
        TCHAR szBuffer[1024];
        wsprintf(szBuffer,TEXT("++++++WriteVideoFrame,TagTime:%ld\n"),dwTagTime);
        OutputDebugString(szBuffer);
	#endif // !_DEBUG

	pData=WriteTagHeader(pData,FT_VIDEO,dwTagSize,dwTagTime);

	BYTE cbFrameType=*pVideopData;
	
	cbFrameType&=0x7;
	if(cbFrameType==5 || cbFrameType==6 || avpt==AVCPT_SEQHEADER || avpt==AVCPT_SEQEND)
		cbFrameType=1;
	else
		cbFrameType=2;
	
	*pData++=(cbFrameType<<4)|VCT_AVC;
	*pData++=(BYTE)avpt;
	if(avpt==AVCPT_NALU)
	{
		pData=AMF_EncodeInt24(pData,(int)dwPackTime);	//时间
		pData=AMF_EncodeInt32(pData,(int)dwDataSize);	//数据大小
	}
		
	int nHeadSize=(int)(pData-cbBuffer);
	//写入文件
	DWORD dwWrited1=nHeadSize;
	DWORD dwWrited2=dwDataSize;
	m_FlvFile.Write(cbBuffer,nHeadSize);
	m_FlvFile.Write(pVideopData,dwDataSize);
	
	m_dwPrevTagSize=nHeadSize+dwDataSize-4;

	m_fileInfo.filesize.dbValue+=dwWrited1;
	m_fileInfo.filesize.dbValue+=dwWrited2;

	return true;
}

//添加音频帧
bool CFlvToFileLinux::WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize)
{
	CThreadLockHandle LockHandle(&m_FileLock);

	DWORD dwTime=::GetTickCount();
	DWORD dwTime1=dwTime;	 
	DWORD dwTime2=dwTime1-m_dwLastTime;
	m_dwLastTime=dwTime1;
	dwTime1-=m_dwStartTime;

	if(dwTime1>m_dwAudioTime)
		dwTime1-=m_dwAudioTime;
	else
		dwTime1=0;
	if(dwTime2>m_dwAudioTime)
		dwTime2-=m_dwAudioTime;
	else
		dwTime2=0;
			
	if(aacpt==AACPT_SEQUENCE)
		dwTime1=0;

	#if OUT_DEBUG
        TCHAR szBuffer[1024];
        wsprintf(szBuffer,TEXT("------WriteAudioFrame,TagTime:%ld\n"),dwTime1);
        OutputDebugString(szBuffer);
	#endif // !_DEBUG

	DWORD dwTagSize=dwDataSize;
	dwTagSize+=2;

	BYTE cbBuffer[64];
	BYTE *pData=cbBuffer;
	pData=WriteTagHeader(pData,FT_AUDIO,dwTagSize,dwTime1);

	BYTE cbFrameType=10;
	cbFrameType<<=4;
	cbFrameType|=(m_cbRateIndex<<2);
	cbFrameType|=(1<<1);		//16bit
	BYTE cbChannel=m_fileInfo.ffAudio.stereo.bValue;//CHANNEL_NUM==1?0:1;
	cbFrameType|=cbChannel;
	*pData++=cbFrameType;
	*pData++=(BYTE)aacpt;
		
	int nHeadSize=(int)(pData-cbBuffer);
	//写入文件
	DWORD dwWrited1=nHeadSize;
	DWORD dwWrited2=dwDataSize;
	m_FlvFile.Write(cbBuffer,nHeadSize);
	m_FlvFile.Write(pAudiopData,dwDataSize);

	m_dwPrevTagSize=nHeadSize+dwDataSize-4;

	m_fileInfo.filesize.dbValue+=dwWrited1;
	m_fileInfo.filesize.dbValue+=dwWrited2;

	return true;
}

//添加音频帧
bool CFlvToFileLinux::WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime)
{
	CThreadLockHandle LockHandle(&m_FileLock);
			
	m_dwLastTime=m_dwStartTime+dwTagTime;

	if(aacpt==AACPT_SEQUENCE)
		dwTagTime=0;

	#if OUT_DEBUG
        TCHAR szBuffer[1024];
        wsprintf(szBuffer,TEXT("------WriteAudioFrame,TagTime:%ld\n"),dwTime1);
        OutputDebugString(szBuffer);
	#endif // !_DEBUG

	DWORD dwTagSize=dwDataSize;
	dwTagSize+=2;

	BYTE cbBuffer[64];
	BYTE *pData=cbBuffer;
	pData=WriteTagHeader(pData,FT_AUDIO,dwTagSize,dwTagTime);

	BYTE cbFrameType=10;
	cbFrameType<<=4;
	cbFrameType|=(m_cbRateIndex<<2);
	cbFrameType|=(1<<1);		//16bit
	BYTE cbChannel=m_fileInfo.ffAudio.stereo.bValue;//CHANNEL_NUM==1?0:1;
	cbFrameType|=cbChannel;
	*pData++=cbFrameType;
	*pData++=(BYTE)aacpt;
		
	int nHeadSize=(int)(pData-cbBuffer);
	//写入文件
	DWORD dwWrited1=nHeadSize;
	DWORD dwWrited2=dwDataSize;
	m_FlvFile.Write(cbBuffer,nHeadSize);
	m_FlvFile.Write(pAudiopData,dwDataSize);

	m_dwPrevTagSize=nHeadSize+dwDataSize-4;

	m_fileInfo.filesize.dbValue+=dwWrited1;
	m_fileInfo.filesize.dbValue+=dwWrited2;

	return true;
}

//添加文本
bool CFlvToFileLinux::WriteTextFrame(const char *pTxtData)
{
	CThreadLockHandle LockHandle(&m_FileLock);

	DWORD dwTime=::GetTickCount();
	DWORD dwTime1=dwTime;	 
	DWORD dwTime2=dwTime1-m_dwLastTime;
	m_dwLastTime=dwTime1;
	dwTime1-=m_dwStartTime;
	
	DWORD dwDataSize=(DWORD)strlen(pTxtData);
	DWORD dwTagSize=dwDataSize;
	
	BYTE cbBuffer[64];
	BYTE *pData=cbBuffer;
	pData=WriteTagHeader(pData,FT_TEXT,dwTagSize,dwTime1);
				
	int nHeadSize=(int)(pData-cbBuffer);
	//写入文件
	DWORD dwWrited1=nHeadSize;
	DWORD dwWrited2=dwDataSize;
	m_FlvFile.Write(cbBuffer,nHeadSize);
	m_FlvFile.Write(pTxtData,dwDataSize);

	m_dwPrevTagSize=nHeadSize+dwDataSize-4;

	m_fileInfo.filesize.dbValue+=dwWrited1;
	m_fileInfo.filesize.dbValue+=dwWrited2;

	return true;
}

//设置AAC头
void CFlvToFileLinux::SetAacHeader()
{
	int nChannelNum=(m_fileInfo.ffAudio.stereo.bValue)?2:1;
	int nSampleRate=(int)m_fileInfo.ffAudio.audiosamplerate.dbValue;
	BYTE cbRateIndex=GetRateIndex(nSampleRate);
	BYTE cbChannel=(BYTE)nChannelNum;	//声道
	BYTE cbAacType=AAC_MAIN;			//AAC_MAIN
	BYTE cbHeader[4]={0};
	cbHeader[0]=(cbAacType<<3)|(cbRateIndex>>1);
	cbHeader[1]=((cbRateIndex&0x01)<<7)|(cbChannel<<3);

	WriteAudioFrame(AACPT_SEQUENCE,cbHeader,2);
}

//////////////////////////////////////////////////////////////////////////////////
#ifndef __AMF_ENCODE_DEF__
BYTE *AMF_EncodeInt16(BYTE *output,short nVal)
{
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output+2;
}

BYTE *AMF_EncodeInt24(BYTE *output, int nVal)
{
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;
	return output+3;
}

BYTE *AMF_EncodeInt32(BYTE *output,int nVal)
{
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return output+4;
}

BYTE *AMF_EncodeString(BYTE *output,LPCSTR lpszValue)
{
	short nLength=strlen(lpszValue);     
	*output++ = AMFT_STRING;
	output = AMF_EncodeInt16(output,nLength);
	memcpy(output,lpszValue,nLength);
	output += nLength;

	return output;
}

BYTE *AMF_EncodeNumber(BYTE *output,double dVal)
{
	*output++ = AMFT_NUMBER;	/* type: Number */
	dVal=ntohdb64(dVal);
	memcpy(output, &dVal, 8);
	return output+8;
}

BYTE *AMF_EncodeBoolean(BYTE *output,int bVal)
{
	*output++ = AMFT_BOOLLEAN;
	*output++ = bVal ? 0x01 : 0x00;
	return output;
}

BYTE *AMF_EncodeNamedString(BYTE *output,LPCSTR lpszName,LPCSTR lpszValue)
{
	int nNameLen=strlen(lpszName); 
	output = AMF_EncodeInt16(output,nNameLen);

	memcpy(output,lpszName,nNameLen);
	output += nNameLen;

	return AMF_EncodeString(output,lpszValue);
}

BYTE *AMF_EncodeNamedNumber(BYTE *output,LPCSTR lpszName,double dVal)
{
	int nNameLen=strlen(lpszName);
	output = AMF_EncodeInt16(output,nNameLen);

	memcpy(output,lpszName,nNameLen);
	output += nNameLen;

	return AMF_EncodeNumber(output,dVal);
}

BYTE *AMF_EncodeNamedBoolean(BYTE *output,LPCSTR lpszName,int bVal)
{
	int nNameLen=strlen(lpszName);
	output = AMF_EncodeInt16(output,nNameLen);

	memcpy(output,lpszName,nNameLen);
	output +=nNameLen;

	return AMF_EncodeBoolean(output,bVal);
}

BYTE GetTagRateIndex(int nSample)
{
	switch(nSample)
	{
	case 5500: return 0;
	case 11025: return 1;
	case 22050: return 2;
	case 44100: return 3;
	}
	return 1;
}


BYTE GetRateIndex(int nSample)
{
	switch(nSample)
	{
	case 96000: return 0;
	case 88200: return 1;
	case 64000: return 2;
	case 48000: return 3;
	case 44100: return 4;
	case 32000: return 5;
	case 24000: return 6;
	case 22050: return 7;
	case 16000: return 8;
	case 12000: return 9;
	case 11025: return 10;
	case 8000: return 11;
	case 7350: return 12;
	case 5500: return 13;
	}
	return 7;
}
#endif	//__AMF_ENCODE_DEF__

//////////////////////////////////////////////////////////////////////