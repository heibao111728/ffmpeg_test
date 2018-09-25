#include <stdio.h>
#include "bsm_image_encoder.h"


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
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

#if 0
int main(int argc, char* argv[])
{
    //Parameters	
    FILE *src_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a.yuv", "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }

    const int src_w = 640, src_h = 480;

    AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;

    //av_get_bits_per_pixel() 返回每个像素点需要几个bit位来表示，由于此处格式是yuv420p，
    //所以每个像素点需要12bit来表示。
    int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));

    FILE *dst_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a_rgb24.rgb", "wb");
    if (NULL == dst_file)
    {
        printf("open destfile failure\n");
        return -1;
    }
    const int dst_w = 640, dst_h = 480;
    AVPixelFormat dst_pixfmt = AV_PIX_FMT_RGB24;
    int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));

    //Structures
    uint8_t *src_data[4];
    int src_linesize[4];

    uint8_t *dst_data[4];
    int dst_linesize[4];

    int rescale_method = SWS_BICUBIC;


    uint8_t *temp_buffer = (uint8_t *)malloc(src_w * src_h * src_bpp / 8);

    int frame_idx = 0;
    int ret = 0;

    ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
    if (ret < 0)
    {
        printf("Could not allocate source image\n");
        return -1;
    }

    ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
    if (ret < 0)
    {
        printf("Could not allocate destination image\n");
        return -1;
    }
    //-----------------------------	
    //Init Method 1
    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_alloc_context();

    //Show AVOption
    av_opt_show2(img_convert_ctx, stdout, AV_OPT_FLAG_VIDEO_PARAM, 0);

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

    //Init Method 2
    //img_convert_ctx = sws_getContext(src_w, src_h,src_pixfmt, dst_w, dst_h, dst_pixfmt, 
    //	rescale_method, NULL, NULL, NULL); 
    //-----------------------------
    /*
    //Colorspace
    ret=sws_setColorspaceDetails(img_convert_ctx,sws_getCoefficients(SWS_CS_ITU601),0,
    sws_getCoefficients(SWS_CS_ITU709),0,
    0, 1 << 16, 1 << 16);
    if (ret==-1) {
    printf( "Colorspace not support.\n");
    return -1;
    }
    */
    while (1)
    {
        if (fread(temp_buffer, 1, src_w*src_h*src_bpp / 8, src_file) != src_w*src_h*src_bpp / 8)
        {
            printf("read src file failure\n");
            break;
        }

        switch (src_pixfmt)
        {
        case AV_PIX_FMT_GRAY8:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h);
            break;
        }
        case AV_PIX_FMT_YUV420P:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
            memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h / 4);      //U
            memcpy(src_data[2], temp_buffer + src_w*src_h * 5 / 4, src_w*src_h / 4);  //V
            break;
        }
        case AV_PIX_FMT_YUV422P:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
            memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h / 2);      //U
            memcpy(src_data[2], temp_buffer + src_w*src_h * 3 / 2, src_w*src_h / 2);  //V
            break;
        }
        case AV_PIX_FMT_YUV444P:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
            memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h);        //U
            memcpy(src_data[2], temp_buffer + src_w*src_h * 2, src_w*src_h);      //V
            break;
        }
        case AV_PIX_FMT_YUYV422:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h * 2);                  //Packed
            break;
        }
        case AV_PIX_FMT_RGB24:
        {
            memcpy(src_data[0], temp_buffer, src_w*src_h * 3);                  //Packed
            break;
        }
        default:
        {
            printf("Not Support Input Pixel Format.\n");
            break;
        }
        }

        sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
        printf("Finish process frame %5d\n", frame_idx);

        frame_idx++;

        switch (dst_pixfmt)
        {
        case AV_PIX_FMT_GRAY8:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);
            break;
        }
        case AV_PIX_FMT_YUV420P:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
            fwrite(dst_data[1], 1, dst_w*dst_h / 4, dst_file);               //U
            fwrite(dst_data[2], 1, dst_w*dst_h / 4, dst_file);               //V
            break;
        }
        case AV_PIX_FMT_YUV422P:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);					//Y
            fwrite(dst_data[1], 1, dst_w*dst_h / 2, dst_file);				//U
            fwrite(dst_data[2], 1, dst_w*dst_h / 2, dst_file);				//V
            break;
        }
        case AV_PIX_FMT_YUV444P:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
            fwrite(dst_data[1], 1, dst_w*dst_h, dst_file);                 //U
            fwrite(dst_data[2], 1, dst_w*dst_h, dst_file);                 //V
            break;
        }
        case AV_PIX_FMT_YUYV422:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h * 2, dst_file);               //Packed
            break;
        }
        case AV_PIX_FMT_RGB24:
        {
            fwrite(dst_data[0], 1, dst_w*dst_h * 3, dst_file);               //Packed
            break;
        }
        default:
        {
            printf("Not Support Output Pixel Format.\n");
            break;
        }
        }
    }

    sws_freeContext(img_convert_ctx);

    free(temp_buffer);
    fclose(dst_file);
    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);

    return 0;
}
#endif


int EncodeYUVToJPEG(const char* InputFileName, const char* OutputFileName, int in_w, int in_h)
{
    AVFormatContext *pFormatCtx;
    AVStream *video_st;
    AVCodecContext* pCodecCtx;
    AVCodec *pCodec;
    AVFrame* pictureFrame;  //编码前数据，如yuv420，rgb24等
    AVPacket *pkt;          //编码后数据，如jpeg等

    pFormatCtx = avformat_alloc_context();
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, OutputFileName);

    // 获取编解码上下文信息
    pCodecCtx = avcodec_alloc_context3(NULL);

    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //print log info;
    av_dump_format(pFormatCtx, 0, OutputFileName, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec)
    {
        printf("Codec not found.");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec.");
        return -1;
    }
    uint8_t *buffer = NULL;
    size_t buffer_size;

    pictureFrame = av_frame_alloc();
    pictureFrame->width = pCodecCtx->width;
    pictureFrame->height = pCodecCtx->height;
    pictureFrame->format = AV_PIX_FMT_YUV420P;
    //int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    /* slurp file content into buffer */
    int ret1 = av_file_map(InputFileName, &buffer, &buffer_size, 0, NULL);
    if (ret1 < 0)
    {
        printf("read input file failure\n");
    }

    av_image_fill_arrays(pictureFrame->data, pictureFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // start encoder
    int ret = avcodec_send_frame(pCodecCtx, pictureFrame);

    pkt = av_packet_alloc();
    av_new_packet(pkt, in_w*in_h*3);

    //Read encoded data from the encoder.
    ret = avcodec_receive_packet(pCodecCtx, pkt);

    video_st = avformat_new_stream(pFormatCtx, 0);
    if (video_st == NULL)
    {
        return -1;
    }

    //Write Header
    avformat_write_header(pFormatCtx, NULL);

    //Write body
    av_write_frame(pFormatCtx, pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);

    printf("Encode Successful.\n");

    av_packet_unref(pkt);               //av_packet_alloc()
    av_frame_free(&pictureFrame);       //av_frame_alloc()
    avformat_free_context(pFormatCtx);  //avformat_alloc_context()

    return 0;
}

int main(int argc, char* argv[])
{
#if 1 
    /**
    * 将yuv420p格式数据转换成rgb24格式
    */
    int src_w = 640;
    int src_h = 480;


    int src_size = bsm_get_bytes_per_pixelformat(src_w, src_h, yuv420p);
    int dst_size = bsm_get_bytes_per_pixelformat(src_w, src_h, rgb24);


    unsigned char *src_buffer = (unsigned char *)malloc(src_size);
    unsigned char *dst_buffer = (unsigned char *)malloc(dst_size);

    FILE *src_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a.yuv", "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }

    FILE *dst_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a_rgb24.rgb", "wb");
    if (NULL == dst_file)
    {
        printf("open destfile failure\n");
        return -1;
    }


    if (fread(src_buffer, 1, src_size, src_file) != src_size)
    {
        printf("read src file failure\n");
        return -1;
    }

    if (!bsm_yuv420p_to_rgb24(src_buffer, src_w, src_h, dst_buffer, dst_size))
    {
        printf("formate convert failure\n");
        return -1;
    }
    else
    {
        fwrite(dst_buffer, 1, dst_size, dst_file);               //Packed
    }

    fclose(src_file);
    fclose(dst_file);
#endif
#if 0
    /**
    * 将rgb24格式数据转换成yuv420p格式
    */
    int src_w = 640;
    int src_h = 480;


    int src_size = bsm_get_bytes_per_pixelformat(src_w, src_h, rgb24);
    int dst_size = bsm_get_bytes_per_pixelformat(src_w, src_h, yuv420p);


    unsigned char *src_buffer = (unsigned char *)malloc(src_size);
    unsigned char *dst_buffer = (unsigned char *)malloc(dst_size);

    FILE *src_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a_rgb24.rgb", "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }

    FILE *dst_file = fopen("D:\\VSProject\\image_encoder\\x64\\Debug\\a_yuv420.yuv", "wb");
    if (NULL == dst_file)
    {
        printf("open destfile failure\n");
        return -1;
    }


    if (fread(src_buffer, 1, src_size, src_file) != src_size)
    {
        printf("read src file failure\n");
        return -1;
    }

    if (!bsm_rgb24_to_yuv420p(src_buffer, src_w, src_h, dst_buffer, dst_size))
    {
        printf("formate convert failure\n");
        return -1;
    }
    else
    {
        fwrite(dst_buffer, 1, dst_size, dst_file);               //Packed
    }

    fclose(src_file);
    fclose(dst_file);
#endif

#if 0
    /**
    * save YUV420 to jpeg
    */
    const char* OutputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a.jpg";
    const char* InputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a.yuv";
    int in_w = 640, in_h = 480;
    //EncodeYUVToJPEG(InputFileName, OutputFileName, in_w, in_h);

    uint8_t *buffer = NULL;
    size_t buffer_size;

    /* slurp file content into buffer */
    int ret1 = av_file_map(InputFileName, &buffer, &buffer_size, 0, NULL);
    if (ret1 < 0)
    {
        printf("read input file failure\n");
    }

    if (!bsm_yuv420p_to_jpeg(buffer, OutputFileName, in_w, in_h))
    {
        printf("convert to jpeg failure\n");
    }

    return 1;
#endif

#if 0
    const char* OutputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a.jpg";
    const char* InputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a_rgb24.rgb";
    int in_w = 640, in_h = 480;
    //EncodeYUVToJPEG(InputFileName, OutputFileName, in_w, in_h);

    uint8_t *buffer = NULL;
    size_t buffer_size;

    /* slurp file content into buffer */
    int ret1 = av_file_map(InputFileName, &buffer, &buffer_size, 0, NULL);
    if (ret1 < 0)
    {
        printf("read input file failure\n");
    }

    if (!bsm_rgb24_to_jpeg(buffer, OutputFileName, in_w, in_h))
    {
        printf("convert to jpeg failure\n");
    }

    return 1;
#endif

}