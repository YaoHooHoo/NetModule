//////////////////////////////////////////////////////////////////////////////////////////
//		��������ͬ����ͷ�ļ� NetDataSyncSink.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ClientSocket.h"
#include "localdef.h"
#include "FlvToFileLinux.h"
#include "TimerEngine.h"
#include "../AudioModule/voice_engine/VoiceEngine.h"

#define WRITE_FLV			0					//д��FLV�ļ�
///////////////////////////////////////////////////////////////////////////////////////////
class CNetDataSyncSink: public IClientSocketSink,public IMideaDataSink,public IAudioDataSink,public ITimerSink
{
private:
	BOOL						m_bLogined;					//��¼��־
	DWORD						m_dwChannelID;				//Ƶ��ID
	DWORD						m_dwRoomIndex;				//����ID
	DWORD						m_dwUserID;					//�û�ID
	DWORD						m_dwAvsConnNum;					//Avserver���Ӵ���
	CONNECTSVRTYPE				m_cst;						//���ӷ���������
	WORD						m_wVssPort;					//��Ƶ�������˿�
	char						m_szVssIp[IPLENGHT];		//��Ƶ������ip
	char						m_szMediaPath[MAX_PATH];	//�Ӳ��ļ�·��

private:
	CClientSocket				m_ClientSocket;				//�������
	IClientSocket				*m_pIClientSocket;			//����ӿ�
	INetNotifySink				*m_pINetNotifySink;			//����֪ͨ�ӿ�
	IAudioManager				*m_pIAudioManager;			//��Ƶ����ӿ�

private:
	int							m_vcx;						//��Ƶ��
	int							m_vcy;						//��Ƶ��

private:
	BYTE						*m_pSpsData;					//Sps����
	DWORD						m_dwSpsSize;					//Sps���ݴ�С
	POINT						m_veOffset;						//ͼ��ƫ����

private:
#if WRITE_FLV
	CFlvToFileLinux				m_FlvToFile;				//Flv�ļ�����	
#endif
	
public:
	CNetDataSyncSink(void);
	~CNetDataSyncSink(void);

	   //base interface 
public:
    //Release object
    virtual bool Release() { if (IsValid()) delete this; return true; }
    //object is valid
    virtual bool IsValid() { return this!=NULL?true:false; }
    //Query interface
    virtual void * QueryInterface(DWORD dwQueryVer);

public:
	//Socket connection message
    virtual bool OnSocketConnect(int iErrorCode,LPCTSTR pszErrorDesc,IClientSocket *pIClientSocket);
    //Socket read message
    virtual bool OnSocketRead(CMD_Command Command,void * pBuffer,DWORD dwDataSize,IClientSocket *pIClientSocket);
    //Socket close message
    virtual bool OnSocketClose(IClientSocket *pIClientSocket,bool bCloseByServer);
	//Thread Stop notify
	virtual void OnTheadStopNotify();
	//��ʱ���¼�
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam);

public:
	//��ʼ���߳�
	bool InitDataSyncThread(INetNotifySink *pINetNotifySink,const char *pMediaPath);
	//������Ƶ��С
	void SetVideoSize(int cx,int cy){m_vcx=cx;m_vcy=cy;}
	//���ӵ�LB������
	bool ConnectToServer(CONNECTSVRTYPE cst);
	//���������߳�
	bool StartNetThread();
	//�ر���������
	bool CloseConnect();
	//������Ƶ
	bool SendVideoData(BYTE *pVideoData,int nDataSize);
	//������Ƶ
	bool SendAudioData(BYTE *pAudioData,int nDataSize);
	//�����ı�����
	bool SendTextData(const char *pTxtData);
	//������Ƶ����ӿ�
	bool SetAudioManager(IAudioManager *pIAudioManager);

public:
	//¼�����ݻص�
	virtual bool OnAacAudioData(const void * pAacData,DWORD dwDataSize);
	//��Ƶ���ݻص�
	virtual bool On264VideoData(BYTE *pVideoData,int nDataSize);
	//��ƵSPS+PPSͷ����
	virtual bool OnS264SpsPpsData(BYTE *pHeadData,int nDataSize);

private:
	//�����������ַ
	bool RequestSvrIPPort();
	//��¼��������
	bool LoginToAvsSvr();
	//�л�����IP
	void SwitchLoadServerIpPort();

	//��Ϣ������
private:
	//������������
	bool OnProcActiveTest(IClientSocket * pIClientSocket);
	//���������������ַResponse
	bool OnProcRequestSvrIPPortResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//�����¼������Response
	bool OnProcLoginToServerResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//����Զ�̿����¼�����
	bool OnProcCtrlEventData(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//����ֹͣ��������֪ͨ
	bool OnProcStopHelperNotify(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//����ͻ���Ƶ
	bool OnProcSerivceAudio(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
};

/////////////////////////////////////////////////////////////////////////////
bool OnNotifyScreenCap(int nEventID);

