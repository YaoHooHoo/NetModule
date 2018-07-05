/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "OIT/JNI/OpenSLAudio"
#include "log.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif
/* 
　　位图文件的组成 
　　结构名称 符 号 
　　位图文件头 (bitmap-file header) BITMAPFILEHEADER bmfh 
　　位图信息头 (bitmap-information header) BITMAPINFOHEADER bmih 
　　彩色表　(color table) RGBQUAD aColors[] 
　　图象数据阵列字节 BYTE aBitmapBits[] 
*/  
typedef struct bmp_header   
{  
    short twobyte           ;//两个字节，用来保证下面成员紧凑排列，这两个字符不能写到文件中  
    //14B  
    char bfType[2]          ;//!文件的类型,该值必需是0x4D42，也就是字符'BM'  
    unsigned int bfSize     ;//!说明文件的大小，用字节为单位  
    unsigned int bfReserved1;//保留，必须设置为0  
    unsigned int bfOffBits  ;//!说明从文件头开始到实际的图象数据之间的字节的偏移量，这里为14B+sizeof(BMPINFO)  
}BMPHEADER;  
  
typedef struct bmp_info  
{  
    //40B  
    unsigned int biSize         ;//!BMPINFO结构所需要的字数  
    int biWidth                 ;//!图象的宽度，以象素为单位  
    int biHeight                ;//!图象的宽度，以象素为单位,如果该值是正数，说明图像是倒向的，如果该值是负数，则是正向的  
    unsigned short biPlanes     ;//!目标设备说明位面数，其值将总是被设为1  
    unsigned short biBitCount   ;//!比特数/象素，其值为1、4、8、16、24、或32  
    unsigned int biCompression  ;//说明图象数据压缩的类型  
#define BI_RGB        0L    //没有压缩  
#define BI_RLE8       1L    //每个象素8比特的RLE压缩编码，压缩格式由2字节组成（重复象素计数和颜色索引）；  
#define BI_RLE4       2L    //每个象素4比特的RLE压缩编码，压缩格式由2字节组成  
#define BI_BITFIELDS  3L    //每个象素的比特由指定的掩码决定。  
    unsigned int biSizeImage    ;//图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0  
    int biXPelsPerMeter         ;//水平分辨率，用象素/米表示  
    int biYPelsPerMeter         ;//垂直分辨率，用象素/米表示  
    unsigned int biClrUsed      ;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）  
    unsigned int biClrImportant ;//对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。  
}BMPINFO;  
  
typedef struct tagRGBQUAD {  
    unsigned char rgbBlue;  
    unsigned char rgbGreen;  
    unsigned char rgbRed;  
    unsigned char rgbReserved;  
} RGBQUAD;  
  
typedef struct tagBITMAPINFO {  
    BMPINFO    bmiHeader;  
    //RGBQUAD    bmiColors[1];  
    unsigned int rgb[3];  
} BITMAPINFO;  
  
static int get_rgb888_header(int w, int h, BMPHEADER * head, BMPINFO * info)  
{  
    int size = 0;  
    if (head && info) {  
        size = w * h * 3;  
        memset(head, 0, sizeof(* head));  
        memset(info, 0, sizeof(* info));  
        head->bfType[0] = 'B';  
        head->bfType[1] = 'M';  
        head->bfOffBits = 14 + sizeof(* info);  
        head->bfSize = head->bfOffBits + size;  
        head->bfSize = (head->bfSize + 3) & ~3;//windows要求文件大小必须是4的倍数  
        size = head->bfSize - head->bfOffBits;  
          
        info->biSize = sizeof(BMPINFO);  
        info->biWidth = w;  
        info->biHeight = -h;  
        info->biPlanes = 1;  
        info->biBitCount = 24;  
        info->biCompression = BI_RGB;  
        info->biSizeImage = size;  
  
        printf("rgb888:%dbit,%d*%d,%d\n", info->biBitCount, w, h, head->bfSize);  
    }  
    return size;  
}  
  
static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)  
{  
    int size = 0;  
    if (head && info) {  
        size = w * h * 2;  
        memset(head, 0, sizeof(* head));  
        memset(info, 0, sizeof(* info));  
        head->bfType[0] = 'B';  
        head->bfType[1] = 'M';  
        head->bfOffBits = 14 + sizeof(* info);  
        head->bfSize = head->bfOffBits + size;  
        head->bfSize = (head->bfSize + 3) & ~3;  
        size = head->bfSize - head->bfOffBits;  
          
        info->bmiHeader.biSize = sizeof(info->bmiHeader);  
        info->bmiHeader.biWidth = w;  
        info->bmiHeader.biHeight = -h;  
        info->bmiHeader.biPlanes = 1;  
        info->bmiHeader.biBitCount = 16;  
        info->bmiHeader.biCompression = BI_BITFIELDS;  
        info->bmiHeader.biSizeImage = size;  
  
        info->rgb[0] = 0xF800;  
        info->rgb[1] = 0x07E0;  
        info->rgb[2] = 0x001F;  
  
        printf("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);  
    }  
    return size;  
}  
  
static int save_bmp_rgb565(FILE * hfile, int w, int h, void * pdata)  
{  
    int success = 0;  
    int size = 0;  
    BMPHEADER head;  
    BITMAPINFO info;  
      
    size = get_rgb565_header(w, h, &head, &info);  
    if (size > 0) {  
        fwrite(head.bfType, 1, 14, hfile);  
        fwrite(&info, 1, sizeof(info), hfile);  
        fwrite(pdata, 1, size, hfile);  
        success = 1;  
    }  
  
    return success;  
}  
  
static int save_bmp_rgb888(FILE * hfile, int w, int h, void * pdata)  
{  
    int success = 0;  
    int size = 0;  
    BMPHEADER head;  
    BMPINFO info;  
      
    size = get_rgb888_header(w, h, &head, &info);  
    if (size > 0) {  
        fwrite(head.bfType, 1, 14, hfile);  
        fwrite(&info, 1, sizeof(info), hfile);  
        fwrite(pdata, 1, size, hfile);  
        success = 1;  
    }  
      
    return success;  
}  
  
int save_bmp(const char * path, int w, int h, void * pdata, int bpp)  
{  
    int success = 0;  
    FILE * hfile = NULL;  
  
    do   
    {  
        if (path == NULL || w <= 0 || h <= 0 || pdata == NULL) {  
            printf("if (path == NULL || w <= 0 || h <= 0 || pdata == NULL)\n");  
            break;  
        }  
  
        remove(path);  
        hfile = fopen(path, "wb");  
        if (hfile == NULL) {  
            printf("open(%s) failed!\n", path);  
            break;  
        }  
  
        switch (bpp)  
        {  
        case 16:  
            success = save_bmp_rgb565(hfile, w, h, pdata);  
            break;  
        case 24:  
            success = save_bmp_rgb888(hfile, w, h, pdata);  
            break;  
		case 32:
			{
				int nImgSize=w*h;
				int8_t * pSrc=(int8_t*)pdata;
				int8_t *pDstData=(int8_t *)malloc(nImgSize*3);
				int8_t *pDst=pDstData;
				for(int i=0;i<nImgSize;i++)
				{
					pDst[0]=pSrc[0];
					pDst[1]=pSrc[1];
					pDst[2]=pSrc[2];
					pDst+=3;
					pSrc+=4;
				}
				success = save_bmp_rgb888(hfile, w, h, pDstData);  
				free(pDstData);
			}
			break;
        default:  
            printf("error: not support format!\n");  
            success = 0;  
            break;  
        }  
    } while (0);  
  
    if (hfile != NULL)  
        fclose(hfile);  
      
    return success;  
} 
//
//static status_t vinfoToPixelFormat(const fb_var_screeninfo& vinfo,
//        uint32_t* bytespp, uint32_t* f)
//{
//
//    switch (vinfo.bits_per_pixel) {
//        case 16:
//            *f = PIXEL_FORMAT_RGB_565;
//            *bytespp = 2;
//            break;
//        case 24:
//            *f = PIXEL_FORMAT_RGB_888;
//            *bytespp = 3;
//            break;
//        case 32:
//            // TODO: do better decoding of vinfo here
//            *f = PIXEL_FORMAT_RGBX_8888;
//            *bytespp = 4;
//            break;
//        default:
//            return BAD_VALUE;
//    }
//    return NO_ERROR;
//}
//
//void screencap(const char* dstfn)
//{      
//	LOGI("---->>>>>>>---screencap to file: %s",dstfn);
//    int fd = -1;      
//    void const* mapbase = MAP_FAILED;
//    ssize_t mapsize = -1;
//
//    void const* base = 0;
//    uint32_t w, s, h, f;
//    size_t size = 0;
//	int bpp=3;
//
//    const char* fbpath = "/dev/graphics/fb0";
//    int fb = open(fbpath, O_RDONLY);
//    if (fb >= 0) {
//        struct fb_var_screeninfo vinfo;
//        if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
//            uint32_t bytespp;
//            if (vinfoToPixelFormat(vinfo, &bytespp, &f) == NO_ERROR) {
//                size_t offset = (vinfo.xoffset + vinfo.yoffset*vinfo.xres) * bytespp;
//                w = vinfo.xres;
//                h = vinfo.yres;
//                s = vinfo.xres;
//                size = w*h*bytespp;
//                mapsize = offset + size;
//                mapbase = mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fb, 0);
//                if (mapbase != MAP_FAILED) {
//                    base = (void const *)((char const *)mapbase + offset);
//                }
//				else
//				{
//					LOGE("-----------------mmap to file faild");
//				}
//				bpp=vinfo.bits_per_pixel;
//            }
//			else
//			{
//				LOGE("-----------------vinfoToPixelFormat faild");
//			}
//        }
//		else
//		{
//			LOGE("------------------ ioctl file faild");
//		}
//        close(fb);
//    }
//	else
//	{
//		LOGE("------ open /dev/graphics/fb0 faild: %d",errno);
//	}
//
//    if (base) {
//		LOGI("---->>>>>>>---screencap success and save to image");
//		void * pData=(void*)base;
//		save_bmp(dstfn,w,h,pData,bpp);		     
//    }
//
//    close(fd);
//    if (mapbase != MAP_FAILED) {
//        munmap((void *)mapbase, mapsize);
//    }
//}

void buildDstImage(const char* dstfn,void * pImgData,int w,int h)
{
	int bpp=32;
	save_bmp(dstfn,w,h,pImgData,bpp);
}

#ifdef __cplusplus
}
#endif