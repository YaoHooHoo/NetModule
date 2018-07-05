//////////////////////////////////////////////////////////////////////////////////////////
//		Android apensl es 录音实现文件 opensl_audio.cpp
//		Author:	bowen.hu
//		Time:	2016-06-09
///////////////////////////////////////////////////////////////////////////////////////////
#include "MediaManager.h"
#include "EncOrDec.h"
#include "localdef.h"

#define LOG_TAG "OIT/JNI/jnicpp"
#include "log.h"
#include "utils.h"

const int MAXTABLE=256;
BYTE g_cbTable[MAXTABLE];
////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////
JavaVM *myVm=NULL;
static jobject j_libYXL;
static jobject eventHandlerInstance = NULL;
CMediaManager *g_pMediaManager=NULL;
BOOL g_bRuning=FALSE;
MASKAREA g_MaskArea={0};
///////////////////////////////////////////////////////////
jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	LOGE("-----<<<<<<<<<<<<<<<<<<<------JNI_OnLoad---------------");
    myVm = vm;
	g_bRuning=FALSE;
    return JNI_VERSION_1_2;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
	myVm=NULL;

	LOGE("-----<<<<<<<<<<<<<<<<<<<------JNI_OnUnload---------------");
}

//删除事件句柄
void Java_com_oit_slaudio_AudioManage_detachEventHandler(JNIEnv *env, jobject thiz)
{
    if (eventHandlerInstance != NULL) {
        env->DeleteGlobalRef(eventHandlerInstance);
        eventHandlerInstance = NULL;
    }
}

//设置事件句柄
void Java_com_oit_slaudio_AudioManage_setEventHandler(JNIEnv *env, jobject thiz, jobject eventHandler)
{
    if (eventHandlerInstance != NULL) {
        env->DeleteGlobalRef(eventHandlerInstance);
        eventHandlerInstance = NULL;
    }

    eventHandlerInstance = getEventHandlerReference(env, thiz, eventHandler);
}

//创建服务
jboolean Java_com_oit_slaudio_AudioManage_createAvtMedia(JNIEnv* env, jclass clazz,jstring lbsSvr,jstring mediaPath)
{
	LOGI("---->>>>>>>---start createAvtMedia function");

	char szServerIP[512]="";
	char szMediaPath[MAX_PATH]="";
	const char* pServerIp = env->GetStringUTFChars(lbsSvr, 0);
	if(pServerIp==NULL || pServerIp[0]==0)
	{
		LOGE("-----------lbs server ip is empty");
		OnNotifyScreenCap(JNI_SC_1001);
	} 
	strcpy(szServerIP,pServerIp);
	env->ReleaseStringUTFChars(lbsSvr,pServerIp);

	const char* pFilePath = env->GetStringUTFChars(mediaPath, 0);
	if(pFilePath==NULL || pFilePath[0]==0)
	{
		LOGE("-----------mediaPath is empty");
	}
	strcpy(szMediaPath,pFilePath);
	env->ReleaseStringUTFChars(mediaPath,pFilePath);
	if(g_pMediaManager!=NULL)
	{
		LOGE("-----------AvManage exists");
		return JNI_TRUE;
	}

	LOGI("---->>>>>>>---start create media manager object");
    if(g_pMediaManager==NULL)
	{
		g_pMediaManager=new CMediaManager();
		if(g_pMediaManager==NULL)
		{
			OnNotifyScreenCap(JNI_SC_1002);
			return JNI_FALSE;
		}
	}
	if(!g_pMediaManager->InitManager(szServerIP,szMediaPath))
			return JNI_FALSE;

	g_bRuning=FALSE;
	LOGI("---->>>>>>>---create audio manager object successed");
	srand(time(NULL));
	for(int i=0;i<MAXTABLE;i++)
	{
		g_cbTable[i]=rand()%MAXTABLE;
	}

    return JNI_TRUE;
}

//设置业务线类型
void Java_com_oit_slaudio_AudioManage_setBusinessType(JNIEnv* env, jclass clazz,jint nType,jstring extValue)
{
	g_GlobalData.nBusinessType=nType;
	const char* pExtValue = env->GetStringUTFChars(extValue, 0);
	if(pExtValue!=NULL)
	{
		int nValLen=lstrlen(pExtValue);
		if(nValLen<=WNAME_LEN)
		{
			strcpy(g_GlobalData.szExtVal,pExtValue);
		}
	}
	env->ReleaseStringUTFChars(extValue,pExtValue);
}

//启动服务
void Java_com_oit_slaudio_AudioManage_startAvtMedia(JNIEnv* env, jclass clazz,jint codecx,jint codecy,jint screencx,jint screency,jstring jobNum,jstring mobile)
{
	bool bMayRun=true;

	LOGI("---->>>>>>>----------------start media manager service...");

	const char* pJogValue = env->GetStringUTFChars(jobNum, 0);
	if(pJogValue==NULL || pJogValue[0]==0)
	{
		LOGE("-----------Service job number is empty");
		OnNotifyScreenCap(JNI_SC_1015);
		bMayRun=false;
	} 
	
	int nJobLength=lstrlen(pJogValue);
	if(nJobLength<MAX_NAME)
	{
		strcpy(g_GlobalData.szJobNum,pJogValue);
	}
	else
	{
		OnNotifyScreenCap(JNI_SC_1015);
		bMayRun=false;
	}
	env->ReleaseStringUTFChars(jobNum,pJogValue);

	g_GlobalData.wReqJobFaildCount=0;
	g_GlobalData.wReReqJobCount=0;

	const char* pMobileValue = env->GetStringUTFChars(mobile, 0);
	if(pMobileValue==NULL || pMobileValue[0]==0)
	{
		LOGE("-----------User mobile number is empty");
		OnNotifyScreenCap(JNI_SC_1016);
		bMayRun=false;
	}
	int nMobileLen=lstrlen(pMobileValue);
	if(nMobileLen<WNAME_LEN)
	{
		strcpy(g_GlobalData.szMobile,pMobileValue);
	}
	else
	{
		OnNotifyScreenCap(JNI_SC_1016);
		bMayRun=false;
	}
	env->ReleaseStringUTFChars(mobile,pMobileValue);

    if(bMayRun && g_pMediaManager!=NULL)
	{
		LOGI("---->>>>>>>---Start run service...");
		if(g_pMediaManager->StartService(codecx,codecy,screencx,screency))
			g_bRuning=TRUE;
	}
	else
	{
		LOGI("---->>>>>>>---Cannot start media manager ...");
		OnNotifyScreenCap(JNI_SC_1003);
	}
}

//停止服务
void Java_com_oit_slaudio_AudioManage_stopAvtMedia(JNIEnv* env, jclass clazz)
{
	g_bRuning=FALSE;
	LOGI("---->>>>>>>---stoping service...");
	if(g_pMediaManager!=NULL)
	{
		LOGI("---->>>>>>>---call g_pMediaManager->StopService...");
		g_pMediaManager->StopService();
	}
	else
	{
		OnNotifyScreenCap(JNI_SC_1019);
	}
}

//Destroy the 对象
void Java_com_oit_slaudio_AudioManage_destroyAvtMedia(JNIEnv* env, jclass clazz)
{
	g_bRuning=FALSE;
	if(g_pMediaManager!=NULL)
	{
		g_pMediaManager->StopService();
		g_pMediaManager->StopAudioService(true);
		delete g_pMediaManager;
		g_pMediaManager=NULL;
	}
}

//保存截屏
void Java_com_oit_slaudio_AudioManage_screenCapImage(JNIEnv* env, jclass clazz,jintArray imgBuf1,jint w1,jint h1)
{
	if(!g_bRuning) return;
	if(g_pMediaManager==NULL)
	{
		LOGE("-----------g_pMediaManager is null");
		return;
	}
	//LOGI("-----------Java_com_oit_slaudio_AudioManage_screenCapImage,w1:%d,x1:%d,x2:%d",w1,g_MaskArea.x1,g_MaskArea.x2);
	//TESTTIME_START;
	
	int size1=w1*h1;
	int stepSize=w1*3;
	int *pszImg1=(int *)malloc(size1*sizeof(int)+64);
	env->GetIntArrayRegion(imgBuf1,0,size1,(jint *)pszImg1);
	
	const int MAXVAL=156;
	const float MINRATE=0.5f;
	const float MAXRATE=(1-MINRATE);
	const int nMaskVal=(int)(128*MAXRATE);
	const int MODVAL=2;
	int nMaskCX=g_MaskArea.x2-g_MaskArea.x1;
	int nAllVal=0;
	int nAllCount=0;
	int nLastTop=0;
	int nFixTop=0;
	bool bMask=false;
	int nStartPos=nLastTop*w1+g_MaskArea.x1;
	int nEndPos=nStartPos+nMaskCX;
	int *pSrc=pszImg1;
	unsigned char *pDst1=(BYTE *)pszImg1;
	
	for(int i=0;i<size1;i++)	//转为RGB888
	{
		int rgb=*pSrc;
		if(g_MaskArea.y2>0 && nLastTop>=g_MaskArea.y1 && nLastTop<=g_MaskArea.y2)
		{
			if(i>=nStartPos && i<nEndPos)
			{
				nAllVal+=rgb;
				nAllCount++;
				rgb=(int)(rgb*MINRATE+nAllVal/nAllCount*MAXRATE);
				bMask=true;
			}
		}
		pDst1[2]=(unsigned char)((rgb>>16)&0xff);
		pDst1[1]=(unsigned char)((rgb>>8)&0xff);
		pDst1[0]=(unsigned char)(rgb&0xff);
		if(bMask)
		{
			rgb=(pDst1[2]+pDst1[1]+pDst1[0])/3;
			rgb=(int)(rgb*MINRATE+nMaskVal);
			if(rgb>MAXVAL) rgb=MAXVAL;
			pDst1[2]=(BYTE)rgb;
			pDst1[1]=(BYTE)rgb;
			pDst1[0]=(BYTE)rgb;
		}
		pDst1+=3;
		pSrc++;
		nLastTop=i/w1;
		if(nFixTop!=nLastTop)
		{
			nFixTop=nLastTop;
			nStartPos=nLastTop*w1+g_MaskArea.x1;
			nEndPos=nStartPos+nMaskCX;
			nAllVal=0;
			nAllCount=0;
		}
		bMask=false;
	}
	
	BYTE *pTmpLine=(BYTE *)malloc(size1*3);
	int nHalfHight=h1/2;
	BYTE *pDstTop=(BYTE*)pszImg1;
	BYTE *pDstBot=pDstTop+(size1-w1)*3;
	for(int h=0;h<nHalfHight;h++)
	{
		memcpy(pTmpLine,pDstTop,stepSize);
		memcpy(pDstTop,pDstBot,stepSize);
		memcpy(pDstBot,pTmpLine,stepSize);
		pDstTop+=stepSize;
		pDstBot-=stepSize;
	}
	free(pTmpLine);
	if(g_bRuning)
	{
		g_pMediaManager->AddScreenImage((BYTE *)pszImg1,w1,h1);
	}
	//TESTTIME_END("AddScreenImage");
		
	free(pszImg1);
}

//保存截屏
void Java_com_oit_slaudio_AudioManage_screenCapImage1(JNIEnv* env,jclass clazz,jbyteArray imgBuf1,jint w1,jint h1)
{
	if(!g_bRuning) return;
	if(g_pMediaManager==NULL)
	{
		LOGE("-----------g_pMediaManager is null");
		return;
	}
	
	int size1=w1*h1;
	int nByteSize=size1*3;
	int stepSize=w1*3;
	BYTE *pszImg1=(BYTE *)malloc(nByteSize+64);
	env->GetByteArrayRegion(imgBuf1,0,nByteSize,(jbyte *)pszImg1);
 
	BYTE *pTmpLine=(BYTE *)malloc(stepSize+12);
	int nHalfHight=h1/2;
	BYTE *pDstTop=(BYTE*)pszImg1;
	BYTE *pDstBot=pDstTop+(size1-w1)*3;
	for(int h=0;h<nHalfHight;h++)
	{
		memcpy(pTmpLine,pDstTop,stepSize);
		memcpy(pDstTop,pDstBot,stepSize);
		memcpy(pDstBot,pTmpLine,stepSize);
		pDstTop+=stepSize;
		pDstBot-=stepSize;
	}
	free(pTmpLine);
	if(g_bRuning)
	{
		g_pMediaManager->AddScreenImage((BYTE *)pszImg1,w1,h1);
	}
	//TESTTIME_END("AddScreenImage");
		
	free(pszImg1);
}

//保存日志文本
void Java_com_oit_slaudio_AudioManage_addAvtText(JNIEnv* env, jclass clazz,jstring txtContent)
{
	if(!g_bRuning) return;

	const char* pTextData = env->GetStringUTFChars(txtContent, 0);
	if(pTextData==NULL || pTextData[0]==0)
	{
		LOGE("-----------addAvtText content is empty");
		return;
	} 

	g_pMediaManager->AddCaptureText(pTextData);
	LOGI("-----------addAvtText content [%s]",pTextData);
	env->ReleaseStringUTFChars(txtContent,pTextData);
}

//加密字符串
jstring Java_com_oit_slaudio_AudioManage_encryptString(JNIEnv* env, jclass clazz,jstring txtContent)
{
	const char* pEndText = env->GetStringUTFChars(txtContent, 0);
	if(pEndText==NULL || pEndText[0]==0)
	{
		LOGE("-----------txtContent is empty");
		return env->NewStringUTF("");
	} 
	
	int nSrcLength=lstrlen(pEndText);
	int nMaxSize=(nSrcLength+5)*2+10;
	char * pTmpText=new char [nMaxSize];
	if(pTmpText==NULL)
	{
		env->ReleaseStringUTFChars(txtContent,pEndText);
		return env->NewStringUTF("");
	}
	int nRetSize=EncCheckCode(pEndText,pTmpText,nMaxSize);
	if(nRetSize==0){
		delete [] pTmpText;
		return env->NewStringUTF("");
	}

	jstring jRetText = env->NewStringUTF(pTmpText);
	delete [] pTmpText;
	return jRetText;
}

//设置Mask区域
void Java_com_oit_slaudio_AudioManage_setMaskArea(JNIEnv* env, jclass clazz,jint x1,jint y1,jint x2,jint y2)
{
	 g_MaskArea.x1=x1;
	 g_MaskArea.y1=y1;
	 g_MaskArea.x2=x2;
	 g_MaskArea.y2=y2;
}

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OnNotifyScreenCap(int nEventID)
{
	if(myVm==NULL) return false;

	if (eventHandlerInstance == NULL)
        return false;

	JNIEnv *env=NULL;
    bool isAttached = false;

    if (myVm->GetEnv((void**) &env, JNI_VERSION_1_2) < 0)
	{
        if (myVm->AttachCurrentThread(&env, NULL) < 0)
            return false;
        isAttached = true;
    }
	if(env==NULL) return false;//org/chat/msg

	jclass cls = env->GetObjectClass(eventHandlerInstance);
    if (!cls)
	{
        LOGE("EventHandler: failed to get class reference");
        if (isAttached)
			myVm->DetachCurrentThread();
		return false;
    }

    /* Find the callback ID */
    jmethodID methodId = env->GetMethodID(cls, "onJniNotifyEvent", "(I)V");
    if(!methodId) 
	{ 
        LOGE("EventHandler: failed to get the onJniNotifyEvent method");
		if (isAttached)
			myVm->DetachCurrentThread();
		return false;
    }
	//LOGI("EventHandler: start notify onCouseEventNotify method,EventID:%d",nEventID);

	env->CallVoidMethod(eventHandlerInstance,methodId,nEventID);

	if (isAttached)
        myVm->DetachCurrentThread();

	return true;
}