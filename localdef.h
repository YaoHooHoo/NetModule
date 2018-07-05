////////////////////////////////////////////////////////////////////////////////////
//		ý��jni ����ȫ�ֶ����ļ� localdef.h
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

#define BUFF_SIZE		64				//�����С
#define RECONN_TIME		5				//����ʱ��(s)
#define SVR_URL			"192.168.9.10"
#define SVR_PORT		8990

#define DT_NET_MODE		1				//�������ݴ���ģʽ
#define DT_TO_FILE		1				//д���ļ�

///////////////////////////////////////////////////////////////////////////
#define IPLENGHT			32			//IP����
#define LBSMAXNUM			4			//���ط����������

//��ʱ������
#define IDT_CONN_SERVER					0x9001					//���ӵ�������id

////////////////////////////////////////////////////////////
//���ط�������Ϣ
typedef struct tagLbsInfo
{
	WORD			wConnCount;				//���Ӵ���
	WORD			wLbsPort;				//���ط������˿�
	TCHAR			szLbsIp[IPLENGHT];		//������IP
}LBSINFO;
//ȫ�����ݽṹ
typedef struct tagGlobalData
{
	int							nBusinessType;				//ҵ��������
	BOOL						bHaveVoice;					//����Ƶ
	DWORD						dwServiceCode;				//�ͷ���
	DWORD						dwClientCode;				//�ͻ���
	LBSINFO						mLbsInfo;					//��ǰ���ط�������Ϣ
	WORD						wLbsCount;					//���ط���������
	WORD						wReReqJobCount;				//�ظ��ɹ�����
	WORD						wReqJobFaildCount;			//�ɹ�ʧ�ܴ���
	TCHAR						szJobNum[MAX_NAME];			//����
	TCHAR						szMobile[WNAME_LEN];		//�ֻ���
	LBSINFO						mLbsList[LBSMAXNUM];
	TCHAR						szExtVal[WNAME_LEN];		//��չ����
}GLOBALDATA;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

////////////// FLV ���� ///////////////////////////////////////////////////
#define MYTRACE TRACE

#define DST_V_CX		1280//1600	//1280		//Ŀ����Ƶ��
#define DST_V_CY		720//900		//720			//Ŀ����Ƶ��

//////////////////////////////////////////////////////////////////////////////
#define SAMPLE_RATE		16000		//44100
#define CHANNEL_NUM		1
#define AAC_MAIN		1			//AAC-MAIN
#define AAC_LC			2			//AAC-LC

//��������
#define VAL_STR 0 
#define VAL_INT 1

//	FLV tag ����
#define FT_AUDIO	0x08			//��Ƶ
#define FT_VIDEO	0x09			//��Ƶ
#define FT_SCRIPT	0x12			//�ű�
#define FT_TEXT		0xA0			//�ı�

typedef enum emAMFType
{
	AMFT_NUMBER,					//��ֵ(8Byte)
	AMFT_BOOLLEAN,					//����ֵ(1Byte)
	AMFT_STRING,					//�ַ���(2Byte����+�ַ�������)
	AMFT_OBJECT,					//����(�������AMF��������)
	AMFT_MOVCLIP,					//
	AMFT_NULL,						//��ֵ
	AMFT_UNDEFINED,					//δ��������
	AMFT_REFERENCE,					//��������
	AMFT_ECMA_ARRAY,				//ECMA��������
	AMFT_OBJECT_END,				//����
	AMFT_STRICT_ARRAY=10,			//Strict��������
	AMFT_DATE,						//��������
	AMFT_LONGSTRING,				//���ַ���(4Byte����+�ַ�������)
}AMFTYPE;

//��Ƶ��������
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

//��Ƶ֡����
typedef enum emVFrameType
{
	VFT_KEYFRAME=1,				//�ؼ�֡
	VFT_NOKEYFRAME,				//�ǹؼ�֡
	VFT_263NOKEYFRAME,			//h263�ǹؼ�֡
	VFT_SERVERKEYFRAME,			//���������ɹؼ�֡
	VFT_VIDEOINFO,				//��Ƶ��Ϣ
}VFRAMETYPE;

//AVC������
typedef enum emAVCPType
{
	AVCPT_SEQHEADER,			//AVCSequence Header
	AVCPT_NALU,					//AVC NALU ����
	AVCPT_SEQEND,				//AVC end ofsequence
}AVCPTYPE;

//AAC������
typedef enum emAACPType
{
	AACPT_SEQUENCE,				//AAC Sequence Header
	AACPT_RAW,					//AAC ����
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

//Flv�ļ�ͷ
#pragma pack(push,1)
typedef struct tagFlvHeader
{
	CHAR		szType[3];				//�ļ�����
	BYTE		cbVersion;				//�汾��Ϣ
	union
	{
		BYTE		cbStreamInfo;
		struct
		{
			BYTE		cbHaveVideo:1;			//�Ƿ�����Ƶ������Ϣ(ǰ5λ����,����Ϊ0;��6λ��ʾ�Ƿ���Ƶ����7λ����������Ϊ0;��8λ��ʾ�Ƿ�����Ƶ)
			BYTE		cbReseve7:1;			//����(1bit)
			BYTE		cbHaveAudio:1;			//�Ƿ�����Ƶ
			BYTE		cbReseve5:5;			//����(5bit)		
		};
	};
	DWORD		dwHeadLength;			//ͷ����
}FLVHEADER;

//Tagͷ
typedef struct tagTagHeader
{
	DWORD		dwPrevTagSize;			//ǰһ��tag��С
	BYTE		cbTagType;				//Tag����(8:��Ƶ,9:��Ƶ;18:�ű�����(AMF��),����������)
	DWORD		dwDataSize;				//���ݳ���
	DWORD		dwTimeStamp;			//ʱ���
	BYTE		cbHighTime;				//ʱ���λֵ(��3���ֽ�ʱ�䲻����ʱʹ�ô��ֶ�)
	DWORD		dwStreamID;				//��ID����Ϊ0
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

//Flv�ļ���Ƶ��Ϣ
typedef struct tagFlvFileAudio
{
	INFOITEM	audiocodecid;			//��Ƶ������ID
	INFOITEM	audiodatarate;			//��Ƶ����
	INFOITEM	audiosamplerate;		//��Ƶ������
	INFOITEM	audiosamplesize;		//��Ƶ������С
	INFOITEM	stereo;					//��Ƶ�Ƿ�������
}FLVFILEAUDIO;

//Flv�ļ���Ƶ��Ϣ
typedef struct tagFlvFileVideo
{
	INFOITEM	width;					//��Ƶ��
	INFOITEM	height;					//��Ƶ��
	INFOITEM	videodatarate;			//��Ƶ����
	INFOITEM	framerate;				//֡��
	INFOITEM	videocodecid;			//��Ƶ������ID
}FLVFILEVIDEO;

//Flv�ļ���Ϣ�ṹ
typedef struct tagFlvFileInfo
{
	FLVFILEAUDIO	ffAudio;				//��Ƶ��Ϣ
	FLVFILEVIDEO	ffVideo;				//��Ƶ��Ϣ
	INFOITEM		duration;				//ʱ�� 
	INFOITEM		filesize;				//�ļ���С

}FLVFILEINFO;

//Mask����
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
	//��Ƶ���ݻص�
	virtual bool On264VideoData(BYTE *pVideoData,int nDataSize)=0;
	//��ƵSPS+PPSͷ����
	virtual bool OnS264SpsPpsData(BYTE *pHeadData,int nDataSize)=0;
	
};
//����Ƶͷ�ӿ�
struct IVideoAudioHead
{
	//��ȡx264ͷ
	virtual int GetSpsPpsData(BYTE *pHeadData,int nBufferSize)=0;
	//��ȡAACͷ
	virtual int GetAacHeadData(BYTE *pHeadData,int nBufferSize)=0;
};

//����֪ͨ�ӿ�
struct INetNotifySink
{
	//��ʼ��Ƶ�ɼ�
	virtual bool StartAVCapThread()=0;
	//��ʼ������Ƶ���
	virtual bool StartAudioService()=0;
	//ֹͣ��Ƶ���
	virtual bool StopAudioService(bool bStopRec)=0;
};

///////////////////////////////////////////////////////////////////////////////
typedef list<string>					CDevList;					//�豸�б�
typedef list<string>::iterator			CDevListIt;					//�豸�б�
typedef vector<string>					CStringVec;					//�ַ���Vec
typedef vector<string>::iterator		CStringVecIt;				//�ַ���Vec
/////////////////////////////////////////////////////////////////////////////////////////////
extern GLOBALDATA g_GlobalData;

#endif	// __BOWEN_HU_LOCAL_DEF_HEAD_FILE__
