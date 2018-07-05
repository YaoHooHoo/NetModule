//////////////////////////////////////////////////////////////////////////////////////////
//		网络数据同步类实现文件 NetDataSyncSink.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/NetDataSync"
#include "log.h"
#include "NetDataSyncSink.h"
#include "TimerEngine.h"

///////////////////////////////////////////////////////////////////////////////////////////
CNetDataSyncSink::CNetDataSyncSink(void)
{
	m_bLogined=false;
	m_pIClientSocket=NULL;

	m_wVssPort=0;
	m_szVssIp[0]=0;
	m_dwChannelID=123456;
 
	m_pINetNotifySink=NULL;
	m_pIClientSocket=NULL;
	m_pIAudioManager=NULL;

	m_vcx=DST_V_CX;
	m_vcy=DST_V_CY;

	m_dwSpsSize=0;
	m_pSpsData=NULL;

	strcpy(m_szMediaPath,"/sdcard/download/");
}

CNetDataSyncSink::~CNetDataSyncSink(void)
{
	if(m_pSpsData!=NULL)
	{
		delete [] m_pSpsData;
		m_pSpsData=NULL;
		m_dwSpsSize=0;
	}
}

//Query interface
void * CNetDataSyncSink::QueryInterface(DWORD dwQueryVer)
{
	QUERYINTERFACE(IClientSocketSink,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IClientSocketSink,dwQueryVer);
	return NULL;
}

//初始化线程
bool CNetDataSyncSink::InitDataSyncThread(INetNotifySink *pINetNotifySink,const char *pMediaPath)
{
	m_pINetNotifySink=pINetNotifySink;
	strcpy(m_szMediaPath,pMediaPath);
	int nLength=strlen(m_szMediaPath);
	if(nLength<3) return false;
	if(m_szMediaPath[nLength-1]!='/')
		strcat(m_szMediaPath,"/");

	IUnknownEx *pIUnknownEx=GET_MYSELF_INTERFACE(IUnknownEx);
	m_ClientSocket.SetSocketSink(pIUnknownEx);

	return (m_pINetNotifySink!=NULL);
}

//设置音频管理接口
bool CNetDataSyncSink::SetAudioManager(IAudioManager *pIAudioManager)
{
	m_pIAudioManager=pIAudioManager;
	return (m_pIAudioManager!=NULL);
}

//Socket connection message
bool CNetDataSyncSink::OnSocketConnect(int iErrorCode,LPCTSTR pszErrorDesc,IClientSocket *pIClientSocket)
{
	LOGI("---->>>>>>>---OnSocketConnect was called,ErrCode:%d,ConnType:%d",iErrorCode,m_cst);
	if(iErrorCode!=0)
	{
		switch(m_cst)
		{
		case CST_LBSVR:
			{
				SwitchLoadServerIpPort();	//切换ip
				SETTIMER(IDT_CONN_SERVER,5000,1,CST_LBSVR,(ITimerSink *)this);
				LOGE(TEXT("Connect to lbs faild,%s,5s try!\n"),pszErrorDesc);
				OnNotifyScreenCap(JNI_SC_1011);
			}
			break; 
		case CST_SWTSVR:
			{
				int nConnType=CST_SWTSVR;
				if(++m_dwAvsConnNum>=3)
				{
					nConnType=CST_LBSVR;
					m_dwAvsConnNum=0;
				}
				//重新请求
				SETTIMER(IDT_CONN_SERVER,5000,1,CST_SWTSVR,(ITimerSink *)this);		//连接定时器
				LOGE(TEXT("Connect to vss faild,%s,5s try\n"),pszErrorDesc);
				OnNotifyScreenCap(JNI_SC_1012);
			}
			break;
		}
		
		return false;
	}

	bool bSuccess=true;
	switch(m_cst)
	{
	case CST_LBSVR:
		bSuccess=RequestSvrIPPort();
		break;
	case CST_SWTSVR:
		bSuccess=LoginToAvsSvr();
		break;
	}

	return bSuccess;
}

//Socket read message
bool CNetDataSyncSink::OnSocketRead(CMD_Command Command,void * pBuffer,DWORD dwDataSize,IClientSocket *pIClientSocket)
{
	switch(Command.wSubCmd)
	{
	case TSVR_SERVICE_AUDIO:
		return OnProcSerivceAudio(Command.dwSequenceID,pBuffer,dwDataSize);
	case TSVR_T_BB_EVENT:
		return OnProcCtrlEventData(Command.dwSequenceID,pBuffer,dwDataSize);
	case TSVR_USER_LOGIN_RESP:
		return OnProcLoginToServerResp(Command.dwSequenceID,pBuffer,dwDataSize);
	case LBS_CLIENT_RES_SVRIP_RESP:
		return OnProcRequestSvrIPPortResp(Command.dwSequenceID,pBuffer,dwDataSize);
	case TSVR_STOP_HELPER:
		return OnProcStopHelperNotify(Command.dwSequenceID,pBuffer,dwDataSize);
	case ACITVE_TEST:
		return OnProcActiveTest(pIClientSocket);
	}
	return true;
}

//Socket close message
bool CNetDataSyncSink::OnSocketClose(IClientSocket *pIClientSocket,bool bCloseByServer)
{
	m_bLogined=FALSE;
	if(m_cst==CST_SWTSVR)
	{
		LOGE(TEXT("Server was closed,5s try..."));
		SETTIMER(IDT_CONN_SERVER,5000,1,CST_SWTSVR,(ITimerSink *)this);
	}
	return true;
}

//Thread Stop notify
void CNetDataSyncSink::OnTheadStopNotify()
{
	m_ClientSocket.CloseSocket(false);
}

//定时器事件
bool CNetDataSyncSink::OnEventTimer(WORD wTimerID,WPARAM wBindParam)
{
	if(wTimerID==IDT_CONN_SERVER)
	{
		CONNECTSVRTYPE ct=(CONNECTSVRTYPE)wBindParam;
		LOGI(TEXT("---------OnEventTimer waw called,ConnectToServer(%d)..."),ct);
		ConnectToServer(ct);
	}

	return true;
}

//连接到LB服务器
bool CNetDataSyncSink::ConnectToServer(CONNECTSVRTYPE cst)
{
	LOGI("---->>>>>>>---Start ConnectToServer call...");

	ASSERT(m_pIClientSocket!=NULL);
	if(m_pIClientSocket==NULL) return false;
	CHAR szSvrUrl[64]="";
	WORD wSvrPort=0;
	m_cst=cst;
	switch(cst)
	{
	case CST_LBSVR:
		m_pIClientSocket->CloseSocket(false);
		strcpy(szSvrUrl,g_GlobalData.mLbsInfo.szLbsIp);
		wSvrPort=g_GlobalData.mLbsInfo.wLbsPort;
		LOGI("Start connected to lbs,ServerIp:%s:%d",szSvrUrl,wSvrPort);
		OnNotifyScreenCap(JNI_SC_2001);
		break;
	case CST_SWTSVR:
		m_bLogined=FALSE;
		strcpy(szSvrUrl,m_szVssIp);
		wSvrPort=m_wVssPort;
		LOGI("Start connected to vss,ServerIp:%s:%d",szSvrUrl,wSvrPort);
		OnNotifyScreenCap(JNI_SC_2002);
		break;
	default:
		return false;
	}

	return m_pIClientSocket->ConnectToServer(szSvrUrl,wSvrPort);
}

//启动网络线程
bool CNetDataSyncSink::StartNetThread()
{
#if DT_NET_MODE
	if(g_GlobalData.mLbsInfo.wLbsPort==0)
	{
		LOGE("----xxxx---Server port is invalid.");
		return false;
	}
	if(m_ClientSocket.GetConnectState()!=SocketState_NoConnect)
		return true;

	m_pIClientSocket=(IClientSocket *)&m_ClientSocket;
	m_bLogined=false;
	if(!m_ClientSocket.IsRuning())
	{
		if(!m_ClientSocket.StartSocket())
		{
			LOGE("Network thread start faild");
			return false;
		}
		else
		{
			LOGE("Network thread start success");
		}
	}	
	return true;
#else 
	#if WRITE_FLV
	static int nFileIndex=0;
	SYSTEMTIME st;
	GetLocalTime(&st);
	DWORD dwTime=(DWORD)time(NULL);
	char szFileName[MAX_PATH]="";
	sprintf(szFileName,"%s%x_%d%02d%02d%02d%02d%02d.flv",m_szMediaPath,dwTime,
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	m_FlvToFile.SetVideoInfo(m_vcx,m_vcy,9000,5);
	m_FlvToFile.SetFileInfo(1,TRUE,FALSE);	
	//m_FlvToFile.SetAudioInfo(m_nChannel,m_bSampleRate);
	m_FlvToFile.CreateFlvFile(szFileName);
	#endif
	return true;
#endif
}

//关闭网络连接
bool CNetDataSyncSink::CloseConnect()
{
	m_bLogined=FALSE;

	#if WRITE_FLV
	m_FlvToFile.CloseFlvFile();
	#endif
	m_ClientSocket.StopSocket();
	sleep(2);	//20180127 Bowen.hu 解决Android 7.0 以上停止时闪退问题
	m_ClientSocket.CloseSocket(false);
	
	return true;
}


//请求服务器地址
bool CNetDataSyncSink::RequestSvrIPPort()
{
	LOGI("Connect to lbs success，Request avss server ip and port...");
	OnNotifyScreenCap(JNI_SC_2003);
	BYTE cbBuffer[512];
	BYTE *pData=cbBuffer;
	WORD wSendSize=0;

	BYTE cbJobNumLen=(BYTE)lstrlen(g_GlobalData.szJobNum);
	BYTE cbMobileLen=(BYTE)lstrlen(g_GlobalData.szMobile);
	BYTE cbExtValLen=(BYTE)lstrlen(g_GlobalData.szExtVal);
	
	pData=WriteByte(pData,cbJobNumLen,wSendSize);
	pData=WriteFromBuffer(pData,g_GlobalData.szJobNum,cbJobNumLen,wSendSize);
	pData=WriteByte(pData,cbMobileLen,wSendSize);
	pData=WriteFromBuffer(pData,g_GlobalData.szMobile,cbMobileLen,wSendSize);
	pData=WriteWord(pData,(WORD)g_GlobalData.nBusinessType,wSendSize);
	pData=WriteByte(pData,cbExtValLen,wSendSize);
	pData=WriteFromBuffer(pData,g_GlobalData.szExtVal,cbExtValLen,wSendSize);	//扩展字段
	
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=LBS_CLIENT_RES_SVRIP;
	cmd.dwSequenceID=0;
	
	return m_pIClientSocket->SendData(cmd,cbBuffer,wSendSize);
}

//处理请求服务器地址Response
bool CNetDataSyncSink::OnProcRequestSvrIPPortResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize)
{
	ASSERT(dwDataSize>=10);
	if(dwDataSize<10) return false;

	struct in_addr addrin;
	DWORD dwSvrType=0;
	DWORD dwSvrIp=0;
	DWORD dwSerivceCode=0;
	WORD wReReqJobCount=0;
	BYTE *pData=(BYTE *)pDataBuffer;
	pData=ReadDword(pData,dwSvrIp);
	pData=ReadWord(pData,m_wVssPort);
	pData=ReadDword(pData,dwSerivceCode);
	pData=ReadWord(pData,wReReqJobCount);

	g_GlobalData.wReReqJobCount=wReReqJobCount;		//重复派工次数
	addrin.s_addr=dwSvrIp;

	CHAR *pTmpIP=inet_ntoa(addrin);
	LOGI(TEXT("Get vss ip and port:[%s:%d][ReReqJobCount:%d]"),pTmpIP,m_wVssPort,wReReqJobCount);

	if(dwSerivceCode==0)
	{
		if(wReReqJobCount>0 && g_GlobalData.wReqJobFaildCount++<g_GlobalData.wReReqJobCount)
		{
			LOGE(TEXT("Get vss service code faild，agin request to lvs..."));
			//重复请求派工
			SETTIMER(IDT_CONN_SERVER,100,1,ST_LBSVR,(ITimerSink *)this);		//重新连接到LBS服务器定时器
		}
		else
		{
			LOGE(TEXT("Get vss service code faild，please agin..."));
			OnNotifyScreenCap(JNI_SC_1014);
		}
		return false;		 	
	}

	if(dwSvrIp==0)
	{ 
		LOGE(TEXT("Get vss ip faild，please agin..."));		
		OnNotifyScreenCap(JNI_SC_1009);
		return false;
	}
	if(m_wVssPort==0)
	{
		LOGE(TEXT("Get vss port faild，please agin..."));
		OnNotifyScreenCap(JNI_SC_1009);
		return false;		 	
	}
	

	lstrcpy(m_szVssIp,pTmpIP);
	g_GlobalData.dwServiceCode=dwSerivceCode;

	LOGI(TEXT("Get vss ip and port success..."));
	OnNotifyScreenCap(JNI_SC_2004);
	SETTIMER(IDT_CONN_SERVER,25,1,ST_SWTSVR,(ITimerSink *)this);		//连接到媒体流服务器定时器
	return false;
}

//登录到vss服务器
bool CNetDataSyncSink::LoginToAvsSvr()
{
	LOGI(TEXT("Start login to vss..."));
	OnNotifyScreenCap(JNI_SC_2005);

	BYTE cbMobileLen=(BYTE)lstrlen(g_GlobalData.szMobile);

	BYTE cbBuffer[256];
	BYTE *pData=cbBuffer;

	WORD wSendSize=0;
	pData=WriteDword(pData,g_GlobalData.dwServiceCode,wSendSize);
	pData=WriteByte(pData,cbMobileLen,wSendSize);
	pData=WriteFromBuffer(pData,g_GlobalData.szMobile,cbMobileLen,wSendSize);
	pData=WriteWord(pData,(WORD)g_GlobalData.nBusinessType,wSendSize);

	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=TSVR_USER_LOGIN;
	cmd.dwSequenceID=0;

	return m_ClientSocket.SendData(cmd,cbBuffer,wSendSize);
}

//处理心跳测试
bool CNetDataSyncSink::OnProcActiveTest(IClientSocket * pIClientSocket)
{
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=ACITVE_TEST_RESP;
	cmd.dwSequenceID=0;
	m_ClientSocket.SendData(cmd);

	return true;
}

//处理登录Response
bool CNetDataSyncSink::OnProcLoginToServerResp(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize)
{
	int nResult=0;
	BOOL bHaveVoice=0;
	DWORD dwRoomIndex=0;
	//LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---1"));
	BYTE *pData=(BYTE *)pDataBuffer;
	pData=ReadInt32(pData,nResult);
	pData=ReadInt32(pData,bHaveVoice);
	if(nResult!=0)
	{
		LOGE(TEXT("Login to vss faild,err code：%d"),nResult);
		TCHAR szMsg[MAX_PATH]=TEXT("");
		if(nResult==CONFIRM_TIMEOUT)
		{
			sprintf(szMsg,TEXT("request helper was timeout,ReqJobCount:%d,faildCount:%d"),g_GlobalData.wReReqJobCount,g_GlobalData.wReqJobFaildCount);
			if(g_GlobalData.wReReqJobCount>0 && g_GlobalData.wReqJobFaildCount++<g_GlobalData.wReReqJobCount) //repeat quest helper
			{
				SETTIMER(IDT_CONN_SERVER,100,1,ST_LBSVR,(ITimerSink *)this);		//连接到媒体流服务器定时器
				return false;	//close socket
			}
		}
		else if(nResult==SERVICE_NOT_EXIST)
			lstrcpy(szMsg,TEXT("service code was not exists"));
		else 
			lstrcpy(szMsg,TEXT("servicer is busy,please wait a moment..."));
		LOGE(szMsg);
		OnNotifyScreenCap(JNI_SC_1009);
		return false;
	}
	//LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---2"));
	m_bLogined=TRUE;

	if(m_dwSpsSize>0)	//发送SpsPps数据
	{
		CMD_Command cmd={0};
		cmd.wMainCmd=ALL_MAIN_CMD;
		cmd.wSubCmd=TSVR_S_H264_SPS;
		m_pIClientSocket->SendData(cmd,m_pSpsData,(WORD)m_dwSpsSize);
	}
	LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---3"));
	//启动录屏
	bool bSuccess=m_pINetNotifySink->StartAVCapThread();
	//LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---4"));
	if(bSuccess)
	{
		if(bHaveVoice)
		{
			LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---4--1"));
			g_GlobalData.bHaveVoice=bHaveVoice;
			//开始启动音频组件
			m_pINetNotifySink->StartAudioService();
			LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---4--2"));
		}
		LOGI(TEXT("请求接入到客服成功"));
		OnNotifyScreenCap(JNI_SC_2006);
	}
	else
	{
		
		LOGE(TEXT("请求接入到客服失败"));
		OnNotifyScreenCap(JNI_SC_1013);
	}
	//LOGW(TEXT("*********----------->>>>>>>>>>>>>>OnProcLoginToServerResp---4"));
	return true;
}

//处理远程控制事件数据
bool CNetDataSyncSink::OnProcCtrlEventData(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize)
{
	return true;
}

//处理停止辅助服务通知
bool CNetDataSyncSink::OnProcStopHelperNotify(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize)
{
	if(dwDataSize!=12) return true;

	DWORD dwClientCode=0;
	DWORD dwServiceCode=0;
	BYTE *pData=(BYTE *)pDataBuffer;
	pData=ReadDword(pData,dwClientCode);
	pData=ReadDword(pData,dwServiceCode);

	TCHAR szMsg[MAX_PATH]=TEXT("");
	mysprintf(szMsg,MAX_PATH,TEXT("Servicer was %u stop service."),dwServiceCode);
	LOGE(szMsg);

	//通知JAVA层
	OnNotifyScreenCap(JNI_SC_2007);
	return true;
}

//处理客户音频
bool CNetDataSyncSink::OnProcSerivceAudio(DWORD dwSequenceID,void *pDataBuffer,DWORD dwDataSize)
{
	if(m_pIAudioManager!=NULL)
	{
		m_pIAudioManager->PlayAacAudioData(pDataBuffer,(int)dwDataSize);
	}
	return true;
}

//录音数据回调
bool CNetDataSyncSink::OnAacAudioData(const void * pAacData,DWORD dwDataSize)
{
	if(!m_bLogined && !g_GlobalData.bHaveVoice) return true;

#if !DT_TO_FILE
	return true;
#endif 
#if DT_NET_MODE
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=TSVR_USER_AUDIO;
	cmd.dwSequenceID=PT_AUDIO;
	return m_ClientSocket.SendData(cmd,(void *)pAacData,dwDataSize);
#else
	#if WRITE_FLV
	/*BYTE cbType=PT_AUDIO;
	WORD wDataSize=(WORD)nDataSize;
	m_MyFile.Write(&cbType,sizeof(cbType));
	m_MyFile.Write(&wDataSize,sizeof(wDataSize));
	m_MyFile.Write(pAacData,nDataSize);*/

	//m_FlvToFile.WriteAudioFrame(AACPT_RAW,pAacData,nDataSize);
	#endif
	return true;
#endif 
}

//视频数据回调
bool CNetDataSyncSink::On264VideoData(BYTE *pVideoData,int nDataSize)
{
	if(!m_bLogined) return true;

#if !DT_TO_FILE
	return true;
#endif 
#if DT_NET_MODE
	
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=TSVR_USER_VIDEO;
	cmd.dwSequenceID=PT_VIDEO;
	return m_ClientSocket.SendData(cmd,pVideoData,(DWORD)nDataSize);
#else
	#if WRITE_FLV
	/*BYTE cbType=PT_VIDEO;
	WORD wDataSize=(WORD)nDataSize;
	m_MyFile.Write(&cbType,sizeof(cbType));
	m_MyFile.Write(&wDataSize,sizeof(wDataSize));
	m_MyFile.Write(pVideoData,nDataSize);*/
	m_FlvToFile.WriteVideoFrame(AVCPT_NALU,pVideoData,nDataSize);
	#endif
	return true;
#endif
}

//视频SPS+PPS头数据
bool CNetDataSyncSink::OnS264SpsPpsData(BYTE *pHeadData,int nDataSize)
{
#if !DT_TO_FILE
	return true;
#endif 
#if DT_NET_MODE
	if(!m_bLogined) return false;
	if((int)m_dwSpsSize!=nDataSize)	//保存Sps数据
	{
		if(m_pSpsData!=NULL)
		{
			delete [] m_pSpsData;
			m_pSpsData=NULL;
			m_dwSpsSize=0;
		}
		m_pSpsData=new BYTE[nDataSize+1];
		if(m_pSpsData!=NULL)
		{
			memcpy(m_pSpsData,pHeadData,nDataSize);
			m_dwSpsSize=nDataSize;
		}
		else
		{
			m_dwSpsSize=0;
		}
	}
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=TSVR_S_H264_SPS;
	cmd.dwSequenceID=PT_SPSPPS;
	return m_ClientSocket.SendData(cmd,pHeadData,(DWORD)nDataSize);
#else
	#if WRITE_FLV
	WORD wSpsSize=pHeadData[0];
	BYTE *pSPSHead=pHeadData+1;
	BYTE *pPPSHead=pSPSHead+wSpsSize;
	WORD wPpsSize=pPPSHead[0];
	pPPSHead++;

	int nIndex=3;
	BYTE cbBuffer[1024];
	BYTE *pBufferHead=cbBuffer;
	memset(pBufferHead,0,nIndex);
	pBufferHead[nIndex++]=0x01;
	pBufferHead[nIndex++]=pSPSHead[1];	//pEncData->pX264t->sps->i_profile_idc;
	pBufferHead[nIndex++]=pSPSHead[2];	//0xc0;
	pBufferHead[nIndex++]=pSPSHead[3];	//0x15;
	pBufferHead[nIndex++]=0xFF;
	pBufferHead[nIndex++]=0xE1;	
	pBufferHead[nIndex++]=0x00;
	pBufferHead[nIndex++]=(unsigned char)wSpsSize;
	memcpy(&pBufferHead[nIndex],pSPSHead,wSpsSize);
	nIndex+=wSpsSize;
	pBufferHead[nIndex++]=0x01;
	pBufferHead[nIndex++]=0x00;
	pBufferHead[nIndex++]=(unsigned char)wPpsSize;
	memcpy(&pBufferHead[nIndex],pPPSHead,wPpsSize);
	nIndex+=wPpsSize;

	m_FlvToFile.WriteVideoFrame(AVCPT_SEQHEADER,cbBuffer,nIndex);
	#endif
	return true;
#endif
}

//发送文本数据
bool CNetDataSyncSink::SendTextData(const char *pTxtData)
{
#if !DT_TO_FILE
	return true;
#endif 
#if DT_NET_MODE
	if(!m_bLogined) return false;

	WORD wLength=(WORD)strlen(pTxtData);
	CMD_Command cmd;
	cmd.wMainCmd=ALL_MAIN_CMD;
	cmd.wSubCmd=TSVR_USER_TXTMSG;
	cmd.dwSequenceID=PT_TEXT;
	return m_ClientSocket.SendData(cmd,(void *)pTxtData,wLength);
#else
	#if WRITE_FLV
	//BYTE cbType=PT_TEXT;
	//WORD wDataSize=(WORD)strlen(pTxtData);
	//m_MyFile.Write(&cbType,sizeof(cbType));
	//m_MyFile.Write(&wDataSize,sizeof(wDataSize));
	m_FlvToFile.WriteTextFrame(pTxtData);
	#endif
	return true;
#endif
}


//切换负载IP
void CNetDataSyncSink::SwitchLoadServerIpPort()
{
	WORD wConnNum=0xFFFF;
	WORD wServerPort=0;
	TCHAR szServerIP[IPLENGHT]=TEXT("");
 
	if(g_GlobalData.wLbsCount<=1) return;

	WORD wIndex=0;
	for(WORD i=0;i<g_GlobalData.wLbsCount;i++)
	{
		WORD wMinuNum=g_GlobalData.mLbsList[i].wConnCount; 
		if(wMinuNum<wConnNum)
		{
			wConnNum=wMinuNum;
			wIndex=i;
			lstrcpy(szServerIP,g_GlobalData.mLbsList[i].szLbsIp);
			wServerPort=g_GlobalData.mLbsList[i].wLbsPort;	
		}
	}
	 
	if(wServerPort!=0)
	{
		lstrcpy(g_GlobalData.mLbsInfo.szLbsIp,szServerIP);
		g_GlobalData.mLbsInfo.wLbsPort=wServerPort;
		g_GlobalData.mLbsList[wIndex].wConnCount++;
	}
}
