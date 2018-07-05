/////////////////////////////////////////////////////////////////////////////////////////
//		Linux ϵͳ�� Flv �ļ��洢��ͷ�ļ� FlvToFileLinux.h
//		hubo 2014-12-25
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef __DEIRLYM_LINUX_BUILD_FLV_FILE_HEAD_FILE__
#define __DEIRLYM_LINUX_BUILD_FLV_FILE_HEAD_FILE__

#include "localdef.h"

/////////////////////////////////////////////////////////////////////////////////////////
class CFlvToFileLinux
{
private:
	bool				m_bHaveVideo;				//����Ƶ
	bool				m_bHaveAudio;				//����Ƶ
	CMyFile				m_FlvFile;					//�ļ����
	TCHAR				m_szFileName[MAX_PATH];		//�ļ���

private:
	FLVHEADER			m_flvHeader;				//�ļ�ͷ
	FLVFILEINFO			m_fileInfo;					//�ļ���Ϣ
	DWORD				m_dwPrevTagSize;			//��һtag��С
	DWORD				m_dwStartTime;				//��ʼʱ��
	DWORD				m_dwLastTime;				//���ʱ��
	DWORD				m_dwVideoTime;				//��Ƶʱ��
	DWORD				m_dwAudioTime;				//��Ƶʱ��

private:
	BYTE				m_cbRateIndex;				//����������
	int					m_nDelayTime;				//�ӳ�ʱ��
	

private:
	CThreadLock			m_FileLock;					//�ļ���

public:
	CFlvToFileLinux(void);
	~CFlvToFileLinux(void);

public:
	//�����ļ���Ϣ
	bool SetFileInfo(BYTE cbVer,bool bHaveVideo,bool bHaveAudio);
	//������Ƶ��Ϣ
	bool SetVideoInfo(int cx,int cy,double cbDataRate,int nFrameRate);
	//������Ƶ��Ϣ
	bool SetAudioInfo(int nChannelNum,int nSampleRate);
	//�����ļ�
	bool CreateFlvFile(LPCTSTR lpszFileName);
	//�ر��ļ�
	bool CloseFlvFile();
	//�����Ƶ֡
	bool WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize);
	//�����Ƶ֡
	bool WriteVideoFrame(AVCPTYPE avpt,const BYTE *pVideopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime);
	//�����Ƶ֡
	bool WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize);
	//�����Ƶ֡
	bool WriteAudioFrame(AACPTYPE aacpt,const BYTE *pAudiopData,DWORD dwDataSize,DWORD dwTagTime,DWORD dwPackTime);
	//����ı�
	bool WriteTextFrame(const char *pTxtData);
	//������ʱ
	void SetDelayTime(DWORD dwDelayTime);
	//��ȡ�ļ���
	LPCTSTR GetFlvFileName(){return m_szFileName;}

private:
	//д���ļ���Ϣtag
	bool WriteFileInfoTag();
	//дTagͷ
	BYTE *WriteTagHeader(BYTE *pData,BYTE cbTagType,DWORD dwDataSize,DWORD dwTime);
	//����AACͷ
	void SetAacHeader();
	//��ȡADTS-AAC�ļ�
	void ProcAacLCFile();
	//���ò���
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
