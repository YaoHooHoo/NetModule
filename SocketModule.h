// //////////////////////////////////////////////////////////////////////////////////////
// Socket module head file SocketModule.h
#ifndef __DEIRLYM_SOCKET_MODULE_HEAD_FILE__
#define __DEIRLYM_SOCKET_MODULE_HEAD_FILE__

#include "localdef.h"

// //////////////////////////////////////////////////////////////////////////////////////
//define socket state
#define SOCK_SUCCESS                0                   //Socket success
#define SOCK_CREATEFAIL             1                   //Create socket fail
#define SOCK_SETIPFAIL               2                   //Set server ip fail
#define SOCK_CONNFAIL               4                   //Socket connect fail
#define SOCK_SELECTFAIL             8                   //Select return fail
#define SOCK_CONN_TIMEOUT           16                  //Socket connect time out
// ////////////////////////////////////////////////////////////////////////////////////////
//connection state
enum enSocketState
{
    SocketState_NoConnect,   
    SocketState_Connecting,
    SocketState_Connected,
};

enum enNetStatus
{
	NS_CONNECTING=1,					//连接中
	NS_CONNECTED,						//连接完成
	NS_SOCKETCLOSE,						//网络关闭
	NS_CONNECTFAILD,					//连接失败
	NS_BEGIN_LOGIN,						//开始登录
	NS_LOGIN_SUCCESS,					//登录成功
	NS_LOGIN_FAILD,						//登录失败
};

// //////////////////////////////////////////////////////////////////////////////////////////
#define VER_IClientSocket       INTERFACE_VERSION(7,1)
//Socket module interface
class IClientSocket: public IUnknownEx
{
public:
    //Set interface
    virtual bool SetSocketSink(IUnknownEx * pIUnknownEx)=0;
    //Get socket interface
    virtual void * GetSocketSink(DWORD dwQueryVer)=0;
    //Get sent space time
    virtual DWORD GetLastSendTick()=0;
    //Get Recv space
    virtual DWORD GetLastRecvTick()=0;
    //Get send count
    virtual DWORD GetSendPacketCount()=0;
    //Get Recv count
    virtual DWORD GetRecvPacketCount()=0;
    //Get socket state
    virtual enSocketState GetConnectState()=0;    
    //Connect to server
    virtual bool ConnectToServer(LPCTSTR szServerIP,WORD wPort)=0;
    //Send data function
    virtual bool SendData(CMD_Command & cmd)=0;
    //Send data function
    virtual bool SendData(CMD_Command & cmd,void * pData,DWORD dwDataSize)=0;
    //Close socket
    virtual bool CloseSocket(bool bNotify)=0; 
    //Start socket
    virtual bool StartSocket()=0;
    //Stop socket 
    virtual bool StopSocket()=0;
};

// /////////////////////////////////////////////////////////////////////////////////////////
#define VER_IClientSocketSink       INTERFACE_VERSION(8,1)
//Client socket sink
class IClientSocketSink: public IUnknownEx
{
public:
    //Socket connection message
    virtual bool OnSocketConnect(int iErrorCode,LPCTSTR pszErrorDesc,IClientSocket *pIClientSocket)=0;
    //Socket read message
    virtual bool OnSocketRead(CMD_Command Command,void * pBuffer,DWORD dwDataSize,IClientSocket *pIClientSocket)=0;
    //Socket close message
    virtual bool OnSocketClose(IClientSocket *pIClientSocket,bool bCloseByServer)=0;
	//Thread Stop notify
	virtual void OnTheadStopNotify()=0;
};

#endif //__DEIRLYM_SOCKET_MODULE_HEAD_FILE__
