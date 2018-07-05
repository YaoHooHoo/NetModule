/////////////////////////////////////////////////////////////////////////////////////////
//		Linux 系统下 Flv 文件存储类头文件 FlvToFileLinux.h
//		hubo 2014-12-25
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef __DEIRLYM_LINUX_BUILD_FLV_FILE_HEAD_FILE__
#define __DEIRLYM_LINUX_BUILD_FLV_FILE_HEAD_FILE__

#include "localdef.h"

/////////////////////////////////////////////////////////////////////////////////////////
class CFlvToFileLinux
{
private:
	bool				m_bHaveVideo;				//有视频
	bool				m_bHaveAudio;				//有音频
	CMyFile				m_FlvFile;					//文件句柄
	TCHAR				m_szFileName[MAX_PATH];		//文件名

private:
	FLVHEADER			m_flvHeader;				//文件头
	FLVFILEINFO			m_fileInfo;					//文件信息
	DWORD				m_dwPrevTagSize;			//上一tag大小
	DWORD				m_dwStartTime;				//开始时间
	DWORD				m_dwLastTime;				//最后时间
	DWORD				m_dwVideoTime;				//视频时间
	DWORD				m_dwAudioTime;				//音频时间

private:
	BYTE				m_cbRateIndex;				//采样率索引
	int					m_nDelayTime;				//延迟时间
	

private:
	CThreadLock			m_FileLock;					//文件锁

public:
	CFlvToFileLinux(void);
	~CFlvToFileLinux(void);

public:
	//设置文件信息
	bool SetFileInfo(BYTE cbVer,bool bHaveVideo,bool bHaveAudio);
	//设置视频信息
	bool SetVideoInfo(int cx,int cy,double cbDataRate,int nFrameRate);
	//设置音频信息
	bool SetAudioInfo(int nChannelNum,int nSampleRate);
	//创建文件
	bool CreateFlvFile(LPCTSTR lpszFileName);
	//关闭文件
	bool CloseFlvFile();
	//添加视频帧
	bool WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize);
	//添加视频帧
	bool WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime);
	//添加音频帧
	bool WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize);
	//添加音频帧
	bool WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime);
	//添加文本
	bool WriteTextFrame(const char *pTxtData);
	//设置延时
	void SetDelayTime(DWORD dwDelayTime);
	//获取文件名
	LPCTSTR GetFlvFileName(){return m_szFileName;}

private:
	//写入文件信息tag
	bool WriteFileInfoTag();
	//写Tag头
	BYTE *WriteTagHeader(BYTE *pData,BYTE cbTagType,DWORD dwDataSize,DWORD dwTime);
	//设置AAC头
	void SetAacHeader();
	//读取ADTS-AAC文件
	void ProcAacLCFile();
	//重置参数
	void RestFlvParam();

};

////////////////////////////////////////////////////////////////////////////
BYTE *AMF_EncodeInt16(BYTE *output,short nVal);
BYTE *AMF_EncodeInt24(BYTE *output, int nVal);
BYTE *AMF_EncodeInt32(BYTE *output,int nVal);
BYTE *AMF_EncodeString(BYTE *output,LPCSTR lpszValue);
BYTE *AMF_EncodeNumber(BYTE *output,double dVal);
BYTE *AMF_EncodeBoolean(BYTE *output,int bVal);
BYTE *AMF_EncodeNamedString(BYTE *output,LPCSTR lpszName,LPCSTR lpszValue);
BYTE *AMF_EncodeNamedNumber(BYTE *output,LPCSTR lpszName,double dVal);
BYTE *AMF_EncodeNamedBoolean(BYTE *output,LPCSTR lpszName,int bVal);
BYTE GetRateIndex(int nSample);
BYTE GetTagRateIndex(int nSample);

#endif //__DEIRLYM_LINUX_BUILD_FLV_FILE_HEAD_FILE__
