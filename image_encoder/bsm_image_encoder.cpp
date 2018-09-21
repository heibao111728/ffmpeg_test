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

    //av_get_bits_per_pixel() 返回每个像素点需要几个bit位来表示，由于此处格式是yuv420p，
    //所以每个像素点需要12bit来表示。
    int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));
    int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));

    //Structures, used by ffmpeg.
    uint8_t *src_data[4];
    int src_linesize[4];

    uint8_t *dst_data[4];
    int dst_linesize[4];

    int ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
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

    switch (src_pixfmt)
    {
    case AV_PIX_FMT_GRAY8:
    {
        memcpy(src_data[0], pdata_src, src_w*src_h);
        break;
    }
    case AV_PIX_FMT_YUV420P:
    {
        memcpy(src_data[0], pdata_src, src_w*src_h);                    //Y
        memcpy(src_data[1], pdata_src + src_w*src_h, src_w*src_h / 4);      //U
        memcpy(src_data[2], pdata_src + src_w*src_h * 5 / 4, src_w*src_h / 4);  //V
        break;
    }
    case AV_PIX_FMT_YUV422P:
    {
        memcpy(src_data[0], pdata_src, src_w*src_h);                    //Y
        memcpy(src_data[1], pdata_src + src_w*src_h, src_w*src_h / 2);      //U
        memcpy(src_data[2], pdata_src + src_w*src_h * 3 / 2, src_w*src_h / 2);  //V
        break;
    }
    case AV_PIX_FMT_YUV444P:
    {
        memcpy(src_data[0], pdata_src,  src_w*src_h);                    //Y
        memcpy(src_data[1], pdata_src + src_w*src_h, src_w*src_h);        //U
        memcpy(src_data[2], pdata_src + src_w*src_h * 2, src_w*src_h);      //V
        break;
    }
    case AV_PIX_FMT_YUYV422:
    {
        memcpy(src_data[0], pdata_src, src_w*src_h * 2);                  //Packed
        break;
    }
    case AV_PIX_FMT_RGB24:
    {
        memcpy(src_data[0], pdata_src, src_w*src_h * 3);                  //Packed
        break;
    }
    default:
    {
        printf("Not Support Input Pixel Format.\n");
        break;
    }
    }

    sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);

    switch (dst_pixfmt)
    {
    case AV_PIX_FMT_GRAY8:
    {
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h);
        break;
    }
    case AV_PIX_FMT_YUV420P:
    {
        //Y
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h);

        //U
        memcpy_s(pdata_dst + dst_w*dst_h,
            dst_size - dst_w*dst_h, 
            dst_data[1], 
            dst_w*dst_h / 4);
        //V
        memcpy_s(pdata_dst + dst_w*dst_h + dst_w*dst_h / 4,
            dst_size - dst_w*dst_h - dst_w*dst_h / 4, 
            dst_data[2], 
            dst_w*dst_h / 4);
        break;
    }
    case AV_PIX_FMT_YUV422P:
    {
        //fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);					//Y
        //fwrite(dst_data[1], 1, dst_w*dst_h / 2, dst_file);				//U
        //fwrite(dst_data[2], 1, dst_w*dst_h / 2, dst_file);				//V

        //Y
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h);
        //U
        memcpy_s(pdata_dst + dst_w*dst_h, 
            dst_size - dst_w*dst_h, 
            dst_data[1], 
            dst_w*dst_h / 2);
        //V
        memcpy_s(pdata_dst + dst_w*dst_h + dst_w*dst_h / 2, 
            dst_size - dst_w*dst_h - dst_w*dst_h / 2, 
            dst_data[2], dst_w*dst_h / 2);
        break;
    }
    case AV_PIX_FMT_YUV444P:
    {
        //fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
        //fwrite(dst_data[1], 1, dst_w*dst_h, dst_file);                 //U
        //fwrite(dst_data[2], 1, dst_w*dst_h, dst_file);                 //V
        //Y
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h);
        //U
        memcpy_s(pdata_dst + dst_w*dst_h, dst_size - dst_w*dst_h, dst_data[1], dst_w*dst_h);
        //V
        memcpy_s(pdata_dst + dst_w*dst_h + dst_w*dst_h, dst_size - dst_w*dst_h - dst_w*dst_h, dst_data[2], dst_w*dst_h);
        break;
    }
    case AV_PIX_FMT_YUYV422:
    {
        //fwrite(dst_data[0], 1, dst_w*dst_h * 2, dst_file);               //Packed
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h * 2);
        break;
    }
    case AV_PIX_FMT_RGB24:
    {
        memcpy_s(pdata_dst, dst_size, dst_data[0], dst_w*dst_h * 3);
        break;
    }
    default:
    {
        printf("Not Support Output Pixel Format.\n");
        break;
    }
    }

    sws_freeContext(img_convert_ctx);

    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);

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
