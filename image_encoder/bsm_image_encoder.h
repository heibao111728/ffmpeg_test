#ifndef BSTORM_IMAGE_ENCODER_H_
#define BSTORM_IMAGE_ENCODER_H_



#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavformat/avio.h>
#include <libavutil/file.h>
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavformat/avio.h>
#include <libavutil/file.h>
#ifdef __cplusplus
};
#endif
#endif

#include <stdio.h>
typedef enum PIC_FORMATE
{
    yuv420p = 0,
    yuv422p,
    rgb24,
    rgbp,
    bgr24,
} PIC_FORMATE_E;

/**
*   根据图片的宽度和高度以及像素的排列方式，获得最小的存储空间，目前只支持5种，分别为：
*   yuv420p
*   yuv422p
*   yuv422p
*   rgbp
*   bgr
*/
int bsm_get_bytes_per_pixelformat(int width, int height, PIC_FORMATE_E pixel_format );

//bool bsm_yuv420p_to_rgb24(unsigned char *pYUV420p_src, int src_width, int src_height, 
//                        unsigned char *pBGR24_dst, int pBGR24_dst_size);

bool bsm_yuv420p_to_rgb24(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size);
bool bsm_rgb24_to_yuv420p(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size);
bool bsm_pixel_format_convert(unsigned char *pdata_src, int src_width, int src_height, AVPixelFormat src_pixfmt, unsigned char *pdata_dst, int dst_size, AVPixelFormat dst_pixfmt);
bool bsm_yuv420p_to_jpeg(unsigned char *pdata_src, const char* OutputFileName, int src_width, int src_height);

bool bsm_rgb24_to_jpeg(unsigned char *pdata_src, const char* OutputFileName, int src_width, int src_height);

//void bsm_yuv420p_to_bgr();

// bsm_yuv422p_to_rgb
// bsm_yuv422p_to_bgr
// bsm_yuv420p_to_rgbp
// bsm_yuv422p_to_rgbp
// bsm_rgb_to_yub420p
// bsm_rgb_to_yub422p
// bsm_bgr_to_yub420p
// bsm_bgr_to_yub422p
// bsm_rgb_to_jpeg

#endif