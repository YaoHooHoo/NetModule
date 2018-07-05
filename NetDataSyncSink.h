//////////////////////////////////////////////////////////////////////////////////////////
//		网络数据同步类头文件 NetDataSyncSink.h
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ClientSocket.h"
#include "localdef.h"
#include "FlvToFileLinux.h"
#include "TimerEngine.h"
#include "../AudioModule/voice_engine/VoiceEngine.h"

#define WRITE_FLV			0					//写入FLV文件
///////////////////////////////////////////////////////////////////////////////////////////
class CNetDataSyncSink: public IClientSocketSink,public IMideaDataSink,public IAudioDataSink,public ITimerSink
{
private:
	BOOL						m_bLogined;					//登录标志
	DWORD						m_dwChannelID;				//频道ID
	DWORD						m_dwRoomIndex;				//房间ID
	DWORD						m_dwUserID;					//用户ID
	DWORD						m_dwAvsConnNum;					//Avserver连接次数
	CONNECTSVRTYPE				m_cst;						//连接服务器类型
	WORD						m_wVssPort;					//视频服务器端口
	char						m_szVssIp[IPLENGHT];		//视频服务器ip
	char						m_szMediaPath[MAX_PATH];	//视步文件路径

private:
	CClientSocket				m_ClientSocket;				//网络对象
	IClientSocket				*m_pIClientSocket;			//网络接口
	INetNotifySink				*m_pINetNotifySink;			//网络通知接口
	IAudioManager				*m_pIAudioManager;			//音频管理接口

private:
	int							m_vcx;						//视频宽
	int							m_vcy;						//视频高

private:
	BYTE						*m_pSpsData;					//Sps数据
	DWORD						m_dwSpsSize;					//Sps数据大小
	POINT						m_veOffset;						//图像偏移量

private:
#if WRITE_FLV
	CFlvToFileLinux				m_FlvToFile;				//Flv文件对象	
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
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID,WPARAM wBindParam);

public:
	//初始化线程
	bool InitDataSyncThread(INetNotifySink *pINetNotifySink,const char *pMediaPath);
	//设置视频大小
	void SetVideoSize(int cx,int cy){m_vcx=cx;m_vcy=cy;}
	//连接到LB服务器
	bool ConnectToServer(CONNECTSVRTYPE cst);
	//启动网络线程
	bool StartNetThread();
	//关闭网络连接
	bool CloseConnect();
	//发送视频
	bool SendVideoData(BYTE *pVideoData,int nDataSize);
	//发送音频
	bool SendAudioData(BYTE *pAudioData,int nDataSize);
	//发送文本数据
	bool SendTextData(const char *pTxtData);
	//设置音频管理接口
	bool SetAudioManager(IAudioManager *pIAudioManager);

public:
	//录音数据回调
	virtual bool OnAacAudioData(const void * pAacData,DWORD dwDataSize);
	//视频数据回调
	virtual bool On264VideoData(BYTE *pVideoData,int nDataSize);
	//视频SPS+PPS头数据
	virtual bool OnS264SpsPpsData(BYTE *pHeadData,int nDataSize);

private:
	//请求服务器地址
	bool RequestSvrIPPort();
	//登录到服务器
	bool LoginToAvsSvr();
	//切换负载IP
	void SwitchLoadServerIpPort();

	//消息处理函烽
private:
	//处理心跳测试
	bool OnProcActiveTest(IClientSocket * pIClientSocket);
	//处理请求服务器地址Response
	bool OnProcRequestSvrIPPortResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//处理登录服务器Response
	bool OnProcLoginToServerResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//处理远程控制事件数据
	bool OnProcCtrlEventData(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//处理停止辅助服务通知
	bool OnProcStopHelperNotify(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
	//处理客户音频
	bool OnProcSerivceAudio(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize);
};

/////////////////////////////////////////////////////////////////////////////
bool OnNotifyScreenCap(int nEventID);

