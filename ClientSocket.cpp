// ///////////////////////////////////////////////////////////////////////////////////
// Client socket class implement file ClientSocket.cpp
// //////////////////////////////////////////////////////////////////////////////////
#include "ClientSocket.h"
#define LOG_TAG "OIT/JNI/OpenSLAudio"
#include "log.h"

// //////////////////////////////////////////////////////////////////////////////////
//construct function
CClientSocket::CClientSocket()
{
    m_bCloseByServer=false;
    m_SocketState=SocketState_NoConnect;
    m_pIClientSocketSink=NULL;
    m_hSocket=INVALID_SOCKET;
    m_wRecvSize=0;
    m_dwSendTickCount=0;
    m_dwRecvTickCount=0;
    m_dwSendPacketCount=0;
    m_dwRecvPacketCount=0;
    m_wServerPort=9100;
    strcpy(m_szServerIP,"127.0.0.1");
    m_nMaxfd=-1;
}
CClientSocket::~CClientSocket()
{
    CloseSocket(false);
}
//Query interface
void * CClientSocket::QueryInterface(DWORD dwQueryVer)
{
    QUERYINTERFACE(IClientSocket,dwQueryVer);
    QUERYINTERFACE_IUNKNOWNEX(IClientSocket,dwQueryVer);

    return NULL;
}
//Set interface
bool CClientSocket::SetSocketSink(IUnknownEx * pIUnknownEx)
{
    //ASSERT(pIUnknownEx!=NULL);
    m_pIClientSocketSink=(IClientSocketSink *)pIUnknownEx->QueryInterface(VER_IClientSocketSink);
    //ASSERT(m_pIClientSocketSink!=NULL);
    return (m_pIClientSocketSink!=NULL);
}
//Get socket interface
void * CClientSocket::GetSocketSink(DWORD dwQueryVer)
{
    if(m_pIClientSocketSink==NULL) return NULL;
    return m_pIClientSocketSink->QueryInterface(dwQueryVer);
}

//Connect to server
bool CClientSocket::ConnectToServer(LPCTSTR szServerIP,WORD wPort)
{
    strcpy(m_szServerIP,szServerIP);
    m_wServerPort=wPort;
    m_dwState=THREAD_CONNECT;
	
    return true;
}

//发送单个包数据
inline bool CClientSocket::SendSingleData(CMD_Command & cmd,const void * pData,WORD wDataSize,WORD wPackNumber)
{
	//构造数据
	WORD wMaxSize=wDataSize+sizeof(CMD_Head)*2;
	BYTE *pNetData=new BYTE[wMaxSize];
	if(pNetData==NULL) return false;

	WORD wSendSize=0;
    CMD_Head * pHead=(CMD_Head *)pNetData;
	
	pHead->dwCommandID=htonl(cmd.dwCommandID);
	pHead->dwSequenceID=htonl(cmd.dwSequenceID);  
	if (wDataSize>0)
	{
		//ASSERT(pData!=NULL);
		memcpy(pHead+1,pData,wDataSize);
	}

	wSendSize=sizeof(CMD_Head)+wDataSize;
	pHead->wPackLength=wSendSize;
	pHead->wPackNumber=wPackNumber;
	pHead->dwTotalLength=htonl(pHead->dwTotalLength);

	//发送数据
	bool bSendSuccess= SendBuffer(pNetData,wSendSize);
	delete [] pNetData;
	return bSendSuccess;
}

//Send data function
bool CClientSocket::SendData(CMD_Command & cmd)
{    
    if (m_hSocket==INVALID_SOCKET) return false;
    if (m_SocketState!=SocketState_Connected) return false;
    
    BYTE cbDataBuffer[sizeof(CMD_Head)*2];
    CMD_Head * pHead=(CMD_Head *)cbDataBuffer;
	pHead->wPackLength=CMD_HEAD_SIZE;
	pHead->wPackNumber=0;
    pHead->dwTotalLength=htonl(pHead->dwTotalLength);
    pHead->dwSequenceID=htonl(cmd.dwSequenceID);
    pHead->dwCommandID=htonl(cmd.dwCommandID);

    CThreadLockHandle LockHandle(&m_BigDataLock);

    return SendBuffer(cbDataBuffer,CMD_HEAD_SIZE);
}

//Send data function
bool CClientSocket::SendData(CMD_Command & cmd,void * pData,DWORD dwDataSize)
{    
    if (m_hSocket==INVALID_SOCKET)
	{
		LOGE("---------**--------m_hSocket==INVALID_SOCKET");
        return false;
	}
    
    if (m_SocketState!=SocketState_Connected)
	{
		LOGE("---------**--------m_SocketState!=SocketState_Connected");
        return false;  
	}
    
	CThreadLockHandle LockHandle(&m_BigDataLock);

   //效验大小
	if (dwDataSize<=SOCKET_PACKAGE)
	{
		return SendSingleData(cmd,pData,(WORD)dwDataSize,0);
	}
    
    WORD wPackCount=(WORD)((dwDataSize+SOCKET_PACKAGE-1)/SOCKET_PACKAGE);
	WORD wPackIndex=1;
	WORD wPackNumber=0;
	WORD wSendSize=0;
	const BYTE *pSrcData=(const BYTE *)pData;
	for(WORD i=0;i<wPackCount;i++)
	{
		wSendSize=(WORD)(dwDataSize>=SOCKET_PACKAGE?SOCKET_PACKAGE:dwDataSize);
		wPackNumber=(wPackIndex<<8)|wPackCount;
		SendSingleData(cmd,pSrcData,wSendSize,wPackNumber);
		pSrcData+=wSendSize;
		dwDataSize-=wSendSize;
		wPackIndex++;
	}

    return true;
}
//Close socket
bool CClientSocket::CloseSocket(bool bNotify)
{   
    bool bClose=(m_hSocket!=INVALID_SOCKET);
    m_SocketState=SocketState_NoConnect;
    if (m_hSocket!=INVALID_SOCKET)
    {        
        close(m_hSocket);
        m_hSocket=INVALID_SOCKET;
        m_SocketState=SocketState_NoConnect;
    }
        
    m_wRecvSize=0;    
    m_dwState=THREAD_WAIT;
    m_nMaxfd=-1;
    FD_ZERO(&m_rfds);

	m_dwSendTickCount=0;
    m_dwRecvTickCount=0;
    m_dwSendPacketCount=0;
    m_dwRecvPacketCount=0;

#ifdef _DEBUG
    printf("Socket is closed,Notify=%d,bClose=%d,m_pIClientSocketSink=0x%x\n",bNotify,bClose,m_pIClientSocketSink);
#endif
    if ((bNotify==true)&&(bClose==true)&&(m_pIClientSocketSink!=NULL))
    {
        //ASSERT(m_pIClientSocketSink!=NULL);
        try { m_pIClientSocketSink->OnSocketClose(this,m_bCloseByServer); }
        catch (...) {}
    }
   
    m_bCloseByServer=false;

    return true;
}
//Start socket
bool CClientSocket::StartSocket()
{
	m_dwState=THREAD_WAIT;
    return StartThread();
}
//Stop socket 
bool CClientSocket::StopSocket()
{
	m_bRun=false;
    return StopThread();
}
//Socet connect
LRESULT CClientSocket::OnSocketNotifyConnect(WPARAM wParam, LPARAM lParam)
{
    lParam=0;
    int iErrorCode=wParam;
    if (iErrorCode==0) 
    {
        m_SocketState=SocketState_Connected;
        m_dwState=THREAD_READ;
    }
    else
    {
        CloseSocket(false);
        m_dwState=THREAD_WAIT;
    }

    TCHAR szErrorDesc[128]=TEXT("");
    GetConnectError(iErrorCode,szErrorDesc,sizeof(szErrorDesc));
    if(m_pIClientSocketSink!=NULL)
        m_pIClientSocketSink->OnSocketConnect(iErrorCode,szErrorDesc,this);   

    return 1;    
}
//Socket read
LRESULT CClientSocket::OnSocketNotifyRead(WPARAM wParam, LPARAM lParam)
{    
    //ASSERT(m_pIClientSocketSink!=NULL);
    wParam=0;
    lParam=0;
    struct timeval tv;
    tv.tv_sec=1;
    tv.tv_usec=0;
    FD_ZERO(&m_rfds);
    FD_SET(m_hSocket,&m_rfds);    
    int nRetVal=select(m_nMaxfd+1,&m_rfds,NULL,NULL,&tv); 
	if(!m_bRun) return 0;	//线程退出
    if(nRetVal==-1)
    {
        printf("Exit,select error!,%s\n",strerror(errno)); 
        CloseSocket(true);
    }    
    else if(nRetVal==0) return 1;
    else if(FD_ISSET(m_hSocket,&m_rfds))
    {                   
        try
        {            
            int iRetCode=recv(m_hSocket,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);            
            if(iRetCode<=0)
            {                
                OnSocketNotifyClose(0,0);
                return 0;
            }
            #ifdef _DEBUG
            printf("Recv data size is %d\n",iRetCode);
            #endif
            m_wRecvSize+=iRetCode;    
            WORD wPacketSize=0;          
            
            CMD_Head * pHead=(CMD_Head *)m_cbRecvBuf;     
            //WORD wHeadSize=CMD_HEAD_SIZE; 
            while (m_wRecvSize>=CMD_HEAD_SIZE)
            { 
				CMD_Command Command;
				CMD_Head head;
				memcpy(&head,pHead,sizeof(head));
				head.dwTotalLength=ntohl(head.dwTotalLength);
				wPacketSize=head.wPackLength;
				Command.wPackNumber=head.wPackNumber;
                //ASSERT(wPacketSize<=(SOCKET_PACKAGE+CMD_HEAD_SIZE));                
                if (wPacketSize>(SOCKET_PACKAGE+CMD_HEAD_SIZE)) throw "Data package is big";
                if (m_wRecvSize<wPacketSize) return 1;                
                m_dwRecvPacketCount++;
               
                Command.dwCommandID=ntohl(pHead->dwCommandID);                
                Command.dwSequenceID=ntohl(pHead->dwSequenceID);

				if(Command.wPackNumber==0)
				{
					memcpy(m_cbDataBuffer,m_cbRecvBuf,wPacketSize);                                                            
					WORD wDataSize=wPacketSize-CMD_HEAD_SIZE;
					BYTE * pDataBuffer=m_cbDataBuffer+CMD_HEAD_SIZE;

					//printf("Sink functoin adress is 0x%x\n",m_pIClientSocketSink);                
					bool bSuccess=m_pIClientSocketSink->OnSocketRead(Command,pDataBuffer,wDataSize,(IClientSocket*)this);                
                
					if (bSuccess==false) throw "Net data package process fail";
				}

				m_wRecvSize-=wPacketSize;
				memmove(m_cbRecvBuf,m_cbRecvBuf+wPacketSize,m_wRecvSize);
            }            
        }
        catch(...)
        {
            printf("========Socket exception\n");
            CloseSocket(true);
        }        
    }
    
    return 1;
}
//Socket close
LRESULT CClientSocket::OnSocketNotifyClose(WPARAM wParam, LPARAM lParam)
{
    wParam=0;
    lParam=0;
    m_bCloseByServer=true;
    CloseSocket(true);
    return 1;
}
//Send data
bool CClientSocket::SendBuffer(void * pBuffer, WORD wSendSize)
{    
    //ASSERT(wSendSize!=0);
    //ASSERT(pBuffer!=NULL);
        
	char szMsg[128]="";
    WORD wSended=0;
    while (wSended<wSendSize)
    {
        int iErrorCode=send(m_hSocket,(char *)pBuffer+wSended,wSendSize-wSended,0);
        if (iErrorCode==SOCKET_ERROR)
        {
            if(errno!=EINPROGRESS)
            {               
                return true;
            }
            LOGE("---------**---------Send faild,[%d][%s]\n",errno,strerror(errno));

            return false;
        }
        wSended+=iErrorCode;
    }
    //m_dwSendTickCount=GetTickCount()/1000L;

    return true;
}
//Thread start notify
bool CClientSocket::OnThreadStartEvent()
{
    return true;
}
//线程停止通知函数
bool CClientSocket::OnThreadStopEvent()
{
	if(m_pIClientSocketSink!=NULL)
		m_pIClientSocketSink->OnTheadStopNotify();
	return true;
}

//Thread body function
bool CClientSocket::RepetitionRun()
{
    switch(m_dwState)
    {
    case THREAD_WAIT:
        {
            struct timespec delay;
			delay.tv_sec = 0;
			delay.tv_nsec = 500* 1000000; // 20 ms
			nanosleep(&delay,NULL);
            break;
        }
    case THREAD_CONNECT:
        {
            ConnectToSvr();
            break;
        }
    case THREAD_READ:
        {
            //printf("Socket waiting read...\n");
            OnSocketNotifyRead(0,0);
            break;
        }
    }
    return m_bRun;
}
//Connect to server
bool CClientSocket::ConnectToSvr()
{
    socklen_t sLen;
    struct sockaddr_in dest_addr;            
     
    struct timeval tv;
    int nRetVal;

	 if (m_hSocket!=INVALID_SOCKET)
    {        
        close(m_hSocket);
        m_hSocket=INVALID_SOCKET;
        m_SocketState=SocketState_NoConnect;
    }
    
    //Create socket
    if((m_hSocket=socket(PF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Create socket fail\n");
        OnSocketNotifyConnect(SOCK_CREATEFAIL,errno);
        return false;
    }
    fcntl(m_hSocket,F_SETFL,O_NDELAY);
            
    memset(&dest_addr,0,sizeof(dest_addr));
    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(m_wServerPort);    
    dest_addr.sin_addr.s_addr=inet_addr(m_szServerIP);    
    
    //Connect to server
    if(connect(m_hSocket,(struct sockaddr *)&dest_addr,sizeof(dest_addr))!=0)
    {
        if(errno!=EINPROGRESS)
        {
            printf("Connect to server faild\n");
            OnSocketNotifyConnect(SOCK_CONNFAIL,errno);
            return false;
        }
    }
       
    if((int)m_hSocket>m_nMaxfd)
        m_nMaxfd=(int)m_hSocket;
    
    FD_ZERO(&m_rfds);
    FD_SET(m_hSocket,&m_rfds);
    for(int i=0;i<SOCK_TIMEOUT;i++)
    {        
		tv.tv_sec=1;
		tv.tv_usec=0;
        nRetVal=select(m_nMaxfd+1,NULL,&m_rfds,NULL,&tv);
        if(nRetVal==-1)
        {
            printf("Exit,select error!,%s\n",strerror(errno));
            OnSocketNotifyConnect(SOCK_SELECTFAIL,errno);
            return false;
        }
        else if(nRetVal==0)
        {
            continue;
        }
        else if(FD_ISSET(m_hSocket,&m_rfds))
        {
            int error;
            sLen=sizeof(error);
            getsockopt(m_hSocket,SOL_SOCKET,SO_ERROR,&error,&sLen);
            if(error==0) //--connect success
            {
                printf("Connect to server success\n");                 
                OnSocketNotifyConnect(SOCK_SUCCESS,errno); 
                return true;
            }
            else
            {
                printf("Connect fail,%s\n",strerror(error)); 
                OnSocketNotifyConnect(SOCK_CONNFAIL,errno); 
                return false;           
            }            
        }        
    }
    OnSocketNotifyConnect(SOCK_CONN_TIMEOUT,errno); 
    return false;
}
//Get error describle
void CClientSocket::GetConnectError(int iErrorCode, LPTSTR pszBuffer, WORD wBufferSize)
{
    //Invalidate parameter
    //ASSERT(pszBuffer!=NULL);
    if (pszBuffer==NULL) return;

    //Error describle
    switch (iErrorCode)
    {
    case SOCK_SUCCESS:                    //no error
        {
            strncpy(pszBuffer,TEXT("oprator success"),wBufferSize);
            break;
        }
    case SOCK_CREATEFAIL:
        {
            strncpy(pszBuffer,TEXT("Create socket fail"),wBufferSize);
            break;
        }
    case SOCK_SETIPFAIL:    //IP Address format error
        {
            strncpy(pszBuffer,TEXT("Destination ip address format is error"),wBufferSize);
            break;
        }
    case SOCK_CONNFAIL:     //Connection refused
        {
            strncpy(pszBuffer,TEXT("Connection refused,Destination server is nonentity"),wBufferSize);
            break;
        }
    case SOCK_CONN_TIMEOUT:        //connection timeout
        {
            strncpy(pszBuffer,TEXT("Connection timeout"),wBufferSize);
            break;
        }    
    default:                //default error
        {
            sprintf(pszBuffer,TEXT("Connection errno%dplease query help"),iErrorCode);
            break;
        }
    }    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

