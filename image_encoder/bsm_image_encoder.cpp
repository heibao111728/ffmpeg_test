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
#include "bsm_image_encoder.h"

namespace bsm{
namespace bsm_image_encoder{

int bsm_get_bytes_per_pixelformat(int width, int height, bsm_pixel_format_e pixel_format)
{
    int bpp = 0;

    switch (pixel_format)
    {
    case yuv420p:
    {
        bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(AV_PIX_FMT_YUV420P));
        break;
    }
    case yuv422p:
    {
        bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(AV_PIX_FMT_YUV422P));
        break;
    }
    case rgb24:
    {
        bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(AV_PIX_FMT_RGB24));
        break;
    }
    case rgbp:
    {
        bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(AV_PIX_FMT_GBRP));
        break;
    }
    case bgr24:
    {
        bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(AV_PIX_FMT_BGR24));
        break;
    }
    default:
    {
        printf("Not Support Input Pixel Format.\n");
        break;
    }
    }
    
    return width * height * bpp / 8;
}

// core converter, convert one pixel format to another. e.g convert yuv420 to rgb24.
bool bsm_pixel_format_convert(unsigned char *pdata_src, int src_width, int src_height, AVPixelFormat src_pixfmt,
    unsigned char *pdata_dst, int dst_size, AVPixelFormat dst_pixfmt)
{
    const int src_w = src_width, src_h = src_height;
    const int dst_w = src_width, dst_h = src_height;

    AVFrame *src_frame;  //data before encode, e.g yuv420, rgb24
    AVFrame *dst_frame;  //data after encode, e.g jpeg

    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_alloc_context();

    //Set Value
    av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
    av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
    av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
    av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
    //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
    av_opt_set_int(img_convert_ctx, "src_range", 1, 0);
    av_opt_set_int(img_convert_ctx, "dstw", dst_w, 0);
    av_opt_set_int(img_convert_ctx, "dsth", dst_h, 0);
    av_opt_set_int(img_convert_ctx, "dst_format", dst_pixfmt, 0);
    av_opt_set_int(img_convert_ctx, "dst_range", 1, 0);
    sws_init_context(img_convert_ctx, NULL, NULL);

    src_frame = av_frame_alloc();
    if (NULL != src_frame)
    {
        src_frame->width = src_w;
        src_frame->height = src_h;
        src_frame->format = src_pixfmt;
    }

    if (av_frame_get_buffer(src_frame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    av_image_fill_arrays(src_frame->data, src_frame->linesize, pdata_src, src_pixfmt, src_frame->width, src_frame->height, 1);

    dst_frame = av_frame_alloc();
    if (NULL != dst_frame)
    {
        dst_frame->width = src_w;
        dst_frame->height = src_h;
        dst_frame->format = dst_pixfmt;
    }

    //get frame buffer
    if (av_frame_get_buffer(dst_frame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    //do transform
    sws_scale(img_convert_ctx, src_frame->data, src_frame->linesize, 0, dst_frame->height, dst_frame->data, dst_frame->linesize);

    av_image_copy_to_buffer(pdata_dst, dst_size, dst_frame->data, dst_frame->linesize, dst_pixfmt, dst_frame->width, dst_frame->height, 1);

    av_frame_free(&src_frame);
    av_frame_free(&dst_frame);

    sws_freeContext(img_convert_ctx);

    return true;
}

bool bsm_yuv420p_to_rgb24(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size)
{
    AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;
    AVPixelFormat dst_pixfmt = AV_PIX_FMT_RGB24;

    return bsm_pixel_format_convert(pdata_src, src_width, src_height, src_pixfmt, pdata_dst, data_dst_size, dst_pixfmt);
}

bool bsm_rgb24_to_yuv420p(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size)
{
    AVPixelFormat src_pixfmt = AV_PIX_FMT_RGB24;
    AVPixelFormat dst_pixfmt = AV_PIX_FMT_YUV420P;

    return bsm_pixel_format_convert(pdata_src, src_width, src_height, src_pixfmt, pdata_dst, data_dst_size, dst_pixfmt);
}

bool bsm_yuv420p_to_jpeg(unsigned char *pdata_src, const char *output_file_name, int src_width, int src_height)
{
    AVFormatContext *p_format_context;
    AVStream *video_st;
    AVCodecContext* p_codec_context;
    AVCodec *p_codec;
    AVFrame* picture_frame;  //data before encode, e.g yuv420, rgb24
    AVPacket *pkt;          //data after encode, e.g jpeg

    p_format_context = avformat_alloc_context();
    avformat_alloc_output_context2(&p_format_context, NULL, NULL, output_file_name);

    // get codec context
    p_codec_context = avcodec_alloc_context3(NULL);

    p_codec_context->codec_id = p_format_context->oformat->video_codec;
    p_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    p_codec_context->pix_fmt = AV_PIX_FMT_YUVJ420P;
    p_codec_context->width = src_width;
    p_codec_context->height = src_height;
    p_codec_context->time_base.num = 1;
    p_codec_context->time_base.den = 25;

    //print log info;
    //av_dump_format(p_format_context, 0, output_file_name, 1);

    p_codec = avcodec_find_encoder(p_codec_context->codec_id);
    if (!p_codec)
    {
        printf("Codec not found.");
        return -1;
    }

    if (avcodec_open2(p_codec_context, p_codec, NULL) < 0)
    {
        printf("Could not open codec.");
        return -1;
    }

    picture_frame = av_frame_alloc();
    picture_frame->width = p_codec_context->width;
    picture_frame->height = p_codec_context->height;
    picture_frame->format = AV_PIX_FMT_YUV420P;
    //int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, p_codec_context->width, p_codec_context->height, 1);

    av_image_fill_arrays(picture_frame->data, picture_frame->linesize, pdata_src, AV_PIX_FMT_YUV420P, p_codec_context->width, p_codec_context->height, 1);

    // start encoder
    int ret = avcodec_send_frame(p_codec_context, picture_frame);

    pkt = av_packet_alloc();
    av_new_packet(pkt, src_width*src_height * 3);

    //Read encoded data from the encoder.
    ret = avcodec_receive_packet(p_codec_context, pkt);

    video_st = avformat_new_stream(p_format_context, 0);
    if (video_st == NULL)
    {
        return -1;
    }

    //Write Header
    avformat_write_header(p_format_context, NULL);

    //Write body
    av_write_frame(p_format_context, pkt);

    //Write Trailer
    av_write_trailer(p_format_context);

    //printf("Encode Successful.\n");

    av_packet_unref(pkt);               //av_packet_alloc()
    av_frame_free(&picture_frame);       //av_frame_alloc()
    avformat_free_context(p_format_context);  //avformat_alloc_context()
    return true;
}

bool bsm_rgb24_to_jpeg(unsigned char *pdata_src, const char *output_file_name, int src_width, int src_height)
{
    int bufsize = src_width*src_height * 3 / 2;
    unsigned char* data = (unsigned char*)malloc(bufsize);

    if (!bsm_rgb24_to_yuv420p(pdata_src, src_width, src_height, data, bufsize))
    {
        printf("covert pixel format from rgb24 to yuv420 failure\n");
        return false;
    }

    if (!bsm_yuv420p_to_jpeg(data, output_file_name, src_width, src_height))
    {
        printf("save yuv420 to jpeg failure\n");
        return false;
    }

    return true;
}

}// namespace bsm_image_encoder
}// namespace bsm


