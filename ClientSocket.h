// /////////////////////////////////////////////////////////////////////////////////////
//  Socket client class head file ClientSocket.h
// /////////////////////////////////////////////////////////////////////////////////////
#ifndef __DEIRLYM_CLIENT_SOCKET_HEAD_FILE__
#define __DEIRLYM_CLIENT_SOCKET_HEAD_FILE__

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>

#include "SocketModule.h"

//////////////////////////////////////////////////////////////////////
//日志输出
void JniOutputLog(const char * pLogMsg);

//define thread state
#define THREAD_WAIT                  0
#define THREAD_CONNECT              1
#define THREAD_READ                  2

#define SOCK_TIMEOUT                30                  //Socket connect timeout
// //////////////////////////////////////////////////////////////////////////////////////
//Net connect class
class CClientSocket: public IClientSocket,public CServiceThread
{
    //State validate
protected:
    DWORD                           m_dwState;                      //Thread state
    bool                            m_bCloseByServer;                   //Close mode
    enSocketState                   m_SocketState;                        //Connect state
    IClientSocketSink               *m_pIClientSocketSink;                   //Sink interface						
    
protected:
    int                             m_nMaxfd;                           //Max socket value
    fd_set                          m_rfds;                             //SOCKET list
    TCHAR                           m_szServerIP[64];                  //Server ip address
    WORD                            m_wServerPort;                     //Server listent port
    SOCKET                          m_hSocket;                        //Socket connect handle
    WORD                            m_wRecvSize;                       //Recv length
    BYTE                            m_cbRecvBuf[SOCKET_BUFFER];       //Recv buffer
    BYTE                            m_cbDataBuffer[SOCKET_PACKAGE+sizeof(CMD_Head)];            //process buffer
    
    //Count validate
protected:    
    DWORD                           m_dwSendTickCount;                  //Send count
    DWORD                           m_dwRecvTickCount;                  //Recv count
    DWORD                           m_dwSendPacketCount;                //Send packet count
    DWORD                           m_dwRecvPacketCount;                //Recv packet count

private:
	CThreadLock						m_BigDataLock;					//Bigdata send lock object
    
    //function define
public:
    //construct function
    CClientSocket();
    virtual ~CClientSocket();
    
    //base interface 
public:
    //Release object
    virtual bool Release() { if (IsValid()) delete this; return true; }
    //object is valid
    virtual bool IsValid() { return this!=NULL?true:false; }
    //Query interface
    virtual void * QueryInterface(DWORD dwQueryVer);

    //interface function
public:
    //Set interface
    virtual bool SetSocketSink(IUnknownEx * pIUnknownEx);
    //Get socket interface
    virtual void * GetSocketSink(DWORD dwQueryVer);
    //Get sent space time
    virtual DWORD GetLastSendTick(){return m_dwSendTickCount;}
    //Get Recv space
    virtual DWORD GetLastRecvTick(){return m_dwRecvTickCount;}
    //Get send count
    virtual DWORD GetSendPacketCount(){return m_dwSendPacketCount;}
    //Get Recv count
    virtual DWORD GetRecvPacketCount(){return m_dwRecvPacketCount;}
    //Get socket state
    virtual enSocketState GetConnectState(){return m_SocketState;}    
    //Connect to server
    virtual bool ConnectToServer(LPCTSTR szServerIP,WORD wPort);
    //Send data function
    virtual bool SendData(CMD_Command & cmd);
    //Send data function
    virtual bool SendData(CMD_Command & cmd,void * pData,DWORD dwDataSize);
    //Close socket
    virtual bool CloseSocket(bool bNotify);
    //Start socket
    virtual bool StartSocket();
    //Stop socket 
    virtual bool StopSocket();

    //process function
protected:
    //Socet connect
    LRESULT OnSocketNotifyConnect(WPARAM wParam, LPARAM lParam);
    //Socket read
    LRESULT OnSocketNotifyRead(WPARAM wParam, LPARAM lParam);
    //Socket close
    LRESULT OnSocketNotifyClose(WPARAM wParam, LPARAM lParam);

    //Helper function
private:
	//发送单个包数据
	inline bool SendSingleData(CMD_Command & cmd,const void * pData,WORD wDataSize,WORD wPackNumber);
    //Send data
    bool SendBuffer(void * pBuffer, WORD wSendSize);
    //Connect to server
    bool ConnectToSvr();
    //Get error describle
    void GetConnectError(int iErrorCode, LPTSTR pszBuffer, WORD wBufferSize);

protected:
    //Thread start notify
    virtual bool OnThreadStartEvent();
	//线程停止通知函数
     virtual bool OnThreadStopEvent();
    //Thread body function
    virtual bool RepetitionRun();    
};

#endif //__DEIRLYM_CLIENT_SOCKET_HEAD_FILE__
