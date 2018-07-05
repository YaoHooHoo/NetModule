//////////////////////////////////////////////////////////////////////////////////////////
//	私有编解码实现文件 EncOrDec.cpp
////////////////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "OIT/JNI/EncApi"
#include "log.h"
#include "localdef.h"
#include "EncOrDec.h"

//////////////////////////////////////////////////////////////////////////////////
#define KEY_LEN		10
BYTE g_EncodeKey[]="OitVss5878";
///////////////////////////////////////////////////////////////////////////////////////////
//加密
int EncCheckCode(LPCTSTR lpszSrcStr,CHAR * pCheckCode,int nBuffSize)
{
	if(pCheckCode==NULL)
		return 0;
	if(lpszSrcStr==NULL)
		return 0;

	SYSTEMTIME st;
	GetLocalTime(&st);

	int nMonth=st.wMonth+'V';
	int nDay=st.wDay+'W';
	int nHour=st.wHour+'X';
	int nMinute=st.wMinute+'Y';
	int nSecond=st.wSecond+'Z';
	const int EXSIZE=5;
	int nLength=lstrlen(lpszSrcStr);
	int nMaxLen=nLength+EXSIZE;
	if(nBuffSize<=nMaxLen*2)
		return 0;

	BYTE *pszTmp=new BYTE[nMaxLen+1];
	if(pszTmp==NULL) return 0;
	memset(pszTmp,0,nMaxLen+1);
	int i=0;
	for(;i<nLength;i++)
	{
		int nLatter=lpszSrcStr[i];
		nLatter^=nMonth;
		nLatter^=nDay;
		nLatter^=nHour;
		nLatter^=nMinute;
		nLatter^=nSecond;
		pszTmp[i]=nLatter;
	}
	int nIndex = i;
	pszTmp[nIndex++]=nMonth;
	pszTmp[nIndex++]=nDay;
	pszTmp[nIndex++]=nHour;
	pszTmp[nIndex++]=nMinute;
	pszTmp[nIndex++]=nSecond;
	for(i=0;i<5;i++)
	{
		int nTemp=pszTmp[i];
		pszTmp[i]=pszTmp[nMaxLen-i-1];
		pszTmp[nMaxLen-i-1]=nTemp;
	}
	CHAR * pEncCode=pCheckCode;
	for(i=0;i<nMaxLen;i++)
	{
		int nLatter=pszTmp[i];
		for(int j=0;j<KEY_LEN;j++)
		{
			int nPass=g_EncodeKey[j];
			nLatter^=nPass;
		}
#ifndef _WIN32
		sprintf(pEncCode,"%02x",nLatter);
#else
		_snprintf_c(pEncCode,3,"%02x",nLatter);
#endif
		pEncCode+=2;
	}
	*pEncCode=0;
	delete [] pszTmp;

	return lstrlen(pCheckCode);
}

int GetHex(int nHigh,int nLow)
{
	if (nHigh >= 'a' && nHigh <= 'z')
    {
        nHigh -= 'a';
        nHigh += 10;
    }
    else
    {
        nHigh -= '0';
    }

    if (nLow >= 'a' && nLow <= 'z')
    {
        nLow -= 'a';
        nLow += 10;
    }
    else
    {
        nLow -= '0';
    }

    return nHigh * 16 + nLow;
}

//解码数据
int DecCheckCode(const CHAR * pCheckCode,CHAR *pDecText,int nBuffSize)
{
	if(pCheckCode==NULL)
		return 0;

	int nEncLength=lstrlen(pCheckCode);
	if(nEncLength%2!=0) return 0;
	int nMaxLen=nEncLength/2;
	if(nBuffSize<=nMaxLen) return 0;

	BYTE *pszEncCode=new BYTE[nEncLength+1]; 
	if(pszEncCode==NULL) return 0;
	memset(pszEncCode,0,nEncLength+1);
	
	int i=0;
	for(;i<nEncLength;i++)
	{
		int nHigh=pCheckCode[i<<1];
		int nLow=pCheckCode[(i<<1)+1];
		int nLatter=GetHex(nHigh,nLow);
		for(int j=0;j<KEY_LEN;j++)
		{
			int cbPass=g_EncodeKey[j];
			nLatter^=cbPass;
		}
		pszEncCode[i]=nLatter;
	}
	for(i=0;i<5;i++)
	{
		int nTemp=pszEncCode[i];
		pszEncCode[i]=pszEncCode[nMaxLen-i-1];
		pszEncCode[nMaxLen-i-1]=nTemp;
	}

	int nIndex = nMaxLen-i;

    int nVar1 = pszEncCode[nIndex++];
    int nVar2 = pszEncCode[nIndex++];
    int nVar3 = pszEncCode[nIndex++];
    int nVar4 = pszEncCode[nIndex++];
    int nVar5 = pszEncCode[nIndex++];
	nMaxLen-=5;
	
	for(i=0;i<nMaxLen;i++)
	{
		int nTmpVar=pszEncCode[i];
		nTmpVar^=nVar1;
		nTmpVar^=nVar2;
		nTmpVar^=nVar3;
		nTmpVar^=nVar4;
		nTmpVar^=nVar5;
		pDecText[i]=nTmpVar;
	}
	pDecText[i]=0;

	delete [] pszEncCode;
	return nMaxLen;
}
