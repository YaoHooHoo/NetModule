////////////////////////////////////////////////////////////////////////////////////
//		媒体jni 本地全局定义文件 localdef.h
/////////////////////////////////////////////////////////////////////////////////////
#ifndef __BOWEN_HU_LOCAL_DEF_HEAD_FILE__
#define __BOWEN_HU_LOCAL_DEF_HEAD_FILE__

#include "../common/globaldef.h"
#include "../common/MemManage.h"
#include "../common/cmddef.h"
#include "../common/ServiceThread.h"
#include "../common/myFile.h"
#include "../common/ProtocolFunction.h"
#include <string>

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////
#define V_WIDTH		640
#define V_HEIGHT	480

#define BUFF_SIZE		64				//缓冲大小
#define RECONN_TIME		5				//重连时间(s)
#define SVR_URL			"192.168.9.10"
#define SVR_PORT		8990

#define DT_NET_MODE		1				//网络数据传输模式
#define DT_TO_FILE		1				//写入文件

///////////////////////////////////////////////////////////////////////////
#define IPLENGHT			32			//IP长度
#define LBSMAXNUM			4			//负载服务器最大数

//定时器定义
#define IDT_CONN_SERVER					0x9001					//连接到服务器id

////////////////////////////////////////////////////////////
//负载服务器信息
typedef struct tagLbsInfo
{
	WORD			wConnCount;				//连接次数
	WORD			wLbsPort;				//负载服务器端口
	TCHAR			szLbsIp[IPLENGHT];		//服务器IP
}LBSINFO;
//全局数据结构
typedef struct tagGlobalData
{
	int							nBusinessType;				//业务线类型
	BOOL						bHaveVoice;					//有音频
	DWORD						dwServiceCode;				//客服号
	DWORD						dwClientCode;				//客户号
	LBSINFO						mLbsInfo;					//当前负载服务器信息
	WORD						wLbsCount;					//负载服务器数量
	WORD						wReReqJobCount;				//重复派工次数
	WORD						wReqJobFaildCount;			//派工失败次数
	TCHAR						szJobNum[MAX_NAME];			//工号
	TCHAR						szMobile[WNAME_LEN];		//手机号
	LBSINFO						mLbsList[LBSMAXNUM];
	TCHAR						szExtVal[WNAME_LEN];		//扩展数据
}GLOBALDATA;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

////////////// FLV 定义 ///////////////////////////////////////////////////
#define MYTRACE TRACE

#define DST_V_CX		1280//1600	//1280		//目标视频宽
#define DST_V_CY		720//900		//720			//目标视频高

//////////////////////////////////////////////////////////////////////////////
#define SAMPLE_RATE		16000		//44100
#define CHANNEL_NUM		1
#define AAC_MAIN		1			//AAC-MAIN
#define AAC_LC			2			//AAC-LC

//参数类型
#define VAL_STR 0 
#define VAL_INT 1

//	FLV tag 类型
#define FT_AUDIO	0x08			//音频
#define FT_VIDEO	0x09			//视频
#define FT_SCRIPT	0x12			//脚本
#define FT_TEXT		0xA0			//文本

typedef enum emAMFType
{
	AMFT_NUMBER,					//数值(8Byte)
	AMFT_BOOLLEAN,					//布尔值(1Byte)
	AMFT_STRING,					//字符串(2Byte长度+字符串数据)
	AMFT_OBJECT,					//对象(包含多个AMF类型数据)
	AMFT_MOVCLIP,					//
	AMFT_NULL,						//空值
	AMFT_UNDEFINED,					//未定义类型
	AMFT_REFERENCE,					//引用类型
	AMFT_ECMA_ARRAY,				//ECMA数组类型
	AMFT_OBJECT_END,				//结束
	AMFT_STRICT_ARRAY=10,			//Strict数组类型
	AMFT_DATE,						//日期类型
	AMFT_LONGSTRING,				//长字符串(4Byte长度+字符串数据)
}AMFTYPE;

//视频编码类型
typedef enum emVCodecType
{
	VCT_JPEG=1,
	VCT_H263,
	VCT_SCREENVIDEO,
	VCT_VP6,
	VCT_VP6ALPHA,
	VCT_SCREENVIDEO2,
	VCT_AVC,					//h264
}VCODECTYPE;

//视频帧类型
typedef enum emVFrameType
{
	VFT_KEYFRAME=1,				//关键帧
	VFT_NOKEYFRAME,				//非关键帧
	VFT_263NOKEYFRAME,			//h263非关键帧
	VFT_SERVERKEYFRAME,			//服务器生成关键帧
	VFT_VIDEOINFO,				//视频信息
}VFRAMETYPE;

//AVC包类型
typedef enum emAVCPType
{
	AVCPT_SEQHEADER,			//AVCSequence Header
	AVCPT_NALU,					//AVC NALU 数据
	AVCPT_SEQEND,				//AVC end ofsequence
}AVCPTYPE;

//AAC包类型
typedef enum emAACPType
{
	AACPT_SEQUENCE,				//AAC Sequence Header
	AACPT_RAW,					//AAC 数据
}AACPTYPE;

typedef struct tagAMFValue
{
	WORD			wOffset;
	union
	{
		double		dbValue;
		BOOL		bValue;
		WORD		wValue;
		DWORD		dwValue;
	};
	string strValue;
}AMFVALUTE;

typedef struct AVal
{
	char *av_val;
	int av_len;
}AVal;

//Flv文件头
#pragma pack(push,1)
typedef struct tagFlvHeader
{
	CHAR		szType[3];				//文件类型
	BYTE		cbVersion;				//版本信息
	union
	{
		BYTE		cbStreamInfo;
		struct
		{
			BYTE		cbHaveVideo:1;			//是否有视频；流信息(前5位保留,必须为0;第6位表示是否不音频；第7位保留，必须为0;第8位表示是否有视频)
			BYTE		cbReseve7:1;			//保留(1bit)
			BYTE		cbHaveAudio:1;			//是否有音频
			BYTE		cbReseve5:5;			//保留(5bit)		
		};
	};
	DWORD		dwHeadLength;			//头长度
}FLVHEADER;

//Tag头
typedef struct tagTagHeader
{
	DWORD		dwPrevTagSize;			//前一个tag大小
	BYTE		cbTagType;				//Tag类型(8:音频,9:视频;18:脚本数据(AMF包),其他：保留)
	DWORD		dwDataSize;				//数据长度
	DWORD		dwTimeStamp;			//时间戳
	BYTE		cbHighTime;				//时间高位值(当3个字节时间不够用时使用此字段)
	DWORD		dwStreamID;				//流ID，总为0
}TAGHEADER;

#pragma pack(pop)

typedef struct tagInfoItem
{
	AMFTYPE		amfType;
	union
	{
		double		dbValue;
		BOOL		bValue;
		WORD		wValue;
		DWORD		dwValue;
	};
	CHAR		szName[24];
	CHAR		szValue[64];
}INFOITEM;

//Flv文件音频信息
typedef struct tagFlvFileAudio
{
	INFOITEM	audiocodecid;			//音频解码器ID
	INFOITEM	audiodatarate;			//音频码率
	INFOITEM	audiosamplerate;		//音频采样率
	INFOITEM	audiosamplesize;		//音频采样大小
	INFOITEM	stereo;					//音频是否立体声
}FLVFILEAUDIO;

//Flv文件视频信息
typedef struct tagFlvFileVideo
{
	INFOITEM	width;					//视频宽
	INFOITEM	height;					//视频高
	INFOITEM	videodatarate;			//视频码率
	INFOITEM	framerate;				//帧率
	INFOITEM	videocodecid;			//视频解码器ID
}FLVFILEVIDEO;

//Flv文件信息结构
typedef struct tagFlvFileInfo
{
	FLVFILEAUDIO	ffAudio;				//音频信息
	FLVFILEVIDEO	ffVideo;				//视频信息
	INFOITEM		duration;				//时长 
	INFOITEM		filesize;				//文件大小

}FLVFILEINFO;

//Mask区域
typedef struct tagMaskArea
{
	int			x1;
	int			y1;
	int			x2;
	int			y2;
}MASKAREA;

//////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32
#define strcpy_s	strcpy
#endif
//////////////////////////////////////////////////////////////////////////////////////
#define TESTTIME_START	struct timeval t_start__,t_end__;gettimeofday(&t_start__, NULL);
#define TESTTIME_END(desc) gettimeofday(&t_end__, NULL);	DWORD dwTime__=t_end__.tv_usec-t_start__.tv_usec; LOGI("----=========%s---Test run time:%d",desc,dwTime__);
/////////////////////////////////////////////////////////////////////////////////////
struct IMideaDataSink
{
	//视频数据回调
	virtual bool On264VideoData(BYTE *pVideoData,int nDataSize)=0;
	//视频SPS+PPS头数据
	virtual bool OnS264SpsPpsData(BYTE *pHeadData,int nDataSize)=0;
	
};
//音视频头接口
struct IVideoAudioHead
{
	//获取x264头
	virtual int GetSpsPpsData(BYTE *pHeadData,int nBufferSize)=0;
	//获取AAC头
	virtual int GetAacHeadData(BYTE *pHeadData,int nBufferSize)=0;
};

//网络通知接口
struct INetNotifySink
{
	//开始视频采集
	virtual bool StartAVCapThread()=0;
	//开始启动音频组件
	virtual bool StartAudioService()=0;
	//停止音频组件
	virtual bool StopAudioService(bool bStopRec)=0;
};

///////////////////////////////////////////////////////////////////////////////
typedef list<string>					CDevList;					//设备列表
typedef list<string>::iterator			CDevListIt;					//设备列表
typedef vector<string>					CStringVec;					//字符串Vec
typedef vector<string>::iterator		CStringVecIt;				//字符串Vec
/////////////////////////////////////////////////////////////////////////////////////////////
extern GLOBALDATA g_GlobalData;

#endif	// __BOWEN_HU_LOCAL_DEF_HEAD_FILE__
