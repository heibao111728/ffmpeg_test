#include "bsm_image_encoder.h"

int bsm_get_bytes_per_pixelformat(int width, int height, PIC_FORMATE_E pixel_format)
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

//核心转换器，完成将一种像素排列方式转换到另一种像素排列方式，如由yuv420到rgb24的转换。
bool bsm_pixel_format_convert(unsigned char *pdata_src, int src_width, int src_height, AVPixelFormat src_pixfmt,
    unsigned char *pdata_dst, int dst_size, AVPixelFormat dst_pixfmt)
{
    const int src_w = src_width, src_h = src_height;
    const int dst_w = src_width, dst_h = src_height;

    AVFrame* srcFrame;  //编码前数据，如yuv420，rgb24等
    AVFrame* dstFrame;  //编码前数据，如yuv420，rgb24等

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

    srcFrame = av_frame_alloc();
    if (NULL != srcFrame)
    {
        srcFrame->width = src_w;
        srcFrame->height = src_h;
        srcFrame->format = AV_PIX_FMT_YUV420P;
    }

    if (av_frame_get_buffer(srcFrame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    av_image_fill_arrays(srcFrame->data, srcFrame->linesize, pdata_src, AV_PIX_FMT_YUV420P, srcFrame->width, srcFrame->height, 1);

    dstFrame = av_frame_alloc();
    if (NULL != dstFrame)
    {
        dstFrame->width = src_w;
        dstFrame->height = src_h;
        dstFrame->format = AV_PIX_FMT_RGB24;
    }

    //获取存储媒体数据空间
    if (av_frame_get_buffer(dstFrame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    //do transform
    sws_scale(img_convert_ctx, srcFrame->data, srcFrame->linesize, 0, dstFrame->height, dstFrame->data, dstFrame->linesize);

    av_image_copy_to_buffer(pdata_dst, dst_size, dstFrame->data, dstFrame->linesize, AV_PIX_FMT_RGB24, dstFrame->width, dstFrame->height, 1);

    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);

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

bool bsm_yuv420p_to_jpeg(unsigned char *pdata_src, const char* OutputFileName, int src_width, int src_height)
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
    pCodecCtx->width = src_width;
    pCodecCtx->height = src_height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //print log info;
    //av_dump_format(pFormatCtx, 0, OutputFileName, 1);

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

    pictureFrame = av_frame_alloc();
    pictureFrame->width = pCodecCtx->width;
    pictureFrame->height = pCodecCtx->height;
    pictureFrame->format = AV_PIX_FMT_YUV420P;
    //int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    av_image_fill_arrays(pictureFrame->data, pictureFrame->linesize, pdata_src, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // start encoder
    int ret = avcodec_send_frame(pCodecCtx, pictureFrame);

    pkt = av_packet_alloc();
    av_new_packet(pkt, src_width*src_height * 3);

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

    //printf("Encode Successful.\n");

    av_packet_unref(pkt);               //av_packet_alloc()
    av_frame_free(&pictureFrame);       //av_frame_alloc()
    avformat_free_context(pFormatCtx);  //avformat_alloc_context()
    return true;
}

bool bsm_rgb24_to_jpeg(unsigned char *pdata_src, const char* OutputFileName, int src_width, int src_height)
{
    int bufsize = src_width*src_height * 3 / 2;
    unsigned char* data = (unsigned char*)malloc(bufsize);

    if (!bsm_rgb24_to_yuv420p(pdata_src, src_width, src_height, data, bufsize))
    {
        printf("covert pixel format from rgb24 to yuv420 failure\n");
        return false;
    }

    if (!bsm_yuv420p_to_jpeg(pdata_src, OutputFileName, src_width, src_height))
    {
        printf("save yuv420 to jpeg failure\n");
        return false;
    }

    return true;
}


