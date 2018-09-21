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
        if (fread( temp_buffer, 1, src_w*src_h*src_bpp / 8, src_file ) != src_w*src_h*src_bpp / 8)
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

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
    FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) 
    {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) 
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if (ret < 0) 
        {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3 (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}


int EncodeYUVToJPEG(const char* InputFileName, const char* OutputFileName, int in_w, int in_h)
{
    FILE *in_file = NULL;
    fopen_s(&in_file, InputFileName, "rb");      //Input File

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, OutputFileName);
    AVOutputFormat *fmt = pFormatCtx->oformat;

    AVStream *video_st = avformat_new_stream(pFormatCtx, 0);
    if (video_st == NULL)
    {
        return -1;
    }
    // 获取编解码器上下文信息
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
    //pCodecCtx = video_st->codec;//已弃用
    if (avcodec_parameters_to_context(pCodecCtx, video_st->codecpar) < 0)
    {
        printf( "Copy stream failed!" );
        return -1;
    }
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //dump some information
    av_dump_format(pFormatCtx, 0, OutputFileName, 1);

    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
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

    AVFrame* pictureFrame = av_frame_alloc();
    pictureFrame->width = pCodecCtx->width;
    pictureFrame->height = pCodecCtx->height;
    pictureFrame->format = AV_PIX_FMT_YUV420P;
    int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    uint8_t *picture_buf = (uint8_t *)av_malloc(size);
    av_image_fill_arrays(pictureFrame->data, pictureFrame->linesize, picture_buf, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    //Write Header
    //avformat_write_header(pFormatCtx, NULL);

    int y_size = pCodecCtx->width * pCodecCtx->height;
    AVPacket *pkt = av_packet_alloc();
    av_new_packet(pkt, y_size * 3);
    //Read YUV
    if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0)
    {
        printf("Could not read input file.");
        return -1;
    }
    pictureFrame->data[0] = picture_buf;                    // Y
    pictureFrame->data[1] = picture_buf + y_size;           // U 
    pictureFrame->data[2] = picture_buf + y_size * 5 / 4;   // V

    avformat_write_header(pFormatCtx, NULL);

    int ret = avcodec_send_frame(pCodecCtx, pictureFrame);
    while (ret >= 0)
    {
        pkt->stream_index = video_st->index;
        ret = avcodec_receive_packet(pCodecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return -1;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        av_write_frame(pFormatCtx, pkt);
    }
    av_packet_unref(pkt);
    /////// Encode //////////////// 
    //Write Trailer
    av_write_trailer(pFormatCtx);

    printf("Encode Successful.\n");

    if (video_st)
    {
        //avcodec_close(video_st->codec);
        av_free(pictureFrame);
        av_free(picture_buf);
    }
    //avio_close(pFormatCtx->pb);//Method 1
    avformat_free_context(pFormatCtx);
    fclose(in_file);
    return 0;
    }

int main(int argc, char* argv[])
{
#if 0 
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

    if (!bsm_yuv420p_to_rgb24_v1(src_buffer, src_w, src_h, dst_buffer, dst_size))
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
#endif
    const char* OutputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a.jpg";
    const char* InputFileName = "D:\\VSProject\\image_encoder\\x64\\Debug\\a.yuv";
    int in_w = 640, in_h = 480;
    EncodeYUVToJPEG(InputFileName, OutputFileName, in_w, in_h);
    return getchar();
}