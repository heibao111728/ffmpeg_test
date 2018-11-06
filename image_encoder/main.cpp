#include <stdio.h>
#include <stdlib.h>
#include "bsm_image_encoder.h"

using namespace bsm;
using namespace bsm_image_encoder;

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

    FILE *src_file = fopen("D:\\VSProject\\ffmpeg_test\\Debug\\a.yuv", "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }

    FILE *dst_file = fopen("D:\\VSProject\\ffmpeg_test\\Debug\\a_rgb24.rgb", "wb");
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
	/**
	*	this is in branch only
	*/
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

    FILE *src_file = fopen("D:\\VSProject\\ffmpeg_test\\Debug\\a_rgb24.rgb", "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }

    FILE *dst_file = fopen("D:\\VSProject\\ffmpeg_test\\Debug\\a_yuv420.yuv", "wb");
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
    char* input_file_name = "D:\\VSProject\\ffmpeg_test\\Debug\\a.yuv";
    char* output_file_name = "D:\\VSProject\\ffmpeg_test\\Debug\\a.jpg";

    int src_w = 640;
    int src_h = 480;
    
    //读取像素数据方法一：
    int src_size = bsm_get_bytes_per_pixelformat(src_w, src_h, yuv420p);

    unsigned char *src_buffer = (unsigned char *)malloc(src_size);

    FILE *src_file = fopen(input_file_name, "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }
    if (fread(src_buffer, 1, src_size, src_file) != src_size)
    {
        printf("read src file failure\n");
        return -1;
    }

    bsm_yuv420p_to_jpeg(src_buffer, output_file_name, src_w, src_h);
    fclose(src_file);

    //读取像素数据方法二，该方法读取的数据，不能用于sws_scale()进行色彩模式转换，原因目前未知。
    //uint8_t *buffer = NULL;
    //size_t buffer_size;

    ///* slurp file content into buffer */
    //int ret1 = av_file_map(InputFileName, &buffer, &buffer_size, 0, NULL);
    //if (ret1 < 0)
    //{
    //    printf("read input file failure\n");
    //}

    //if (!bsm_yuv420p_to_jpeg(buffer, OutputFileName, in_w, in_h))
    //{
    //    printf("convert to jpeg failure\n");
    //}

    return 1;
#endif

#if 1
    /**
    *   save rgb420 to jpg
    */
    char* input_file_name = "D:\\VSProject\\ffmpeg_test\\Debug\\a_rgb24.rgb";
    char* output_file_name = "D:\\VSProject\\ffmpeg_test\\Debug\\a.jpg";

    int src_w = 640;
    int src_h = 480;

    //读取像素数据方法一：
    int src_size = bsm_get_bytes_per_pixelformat(src_w, src_h, rgb24);

    unsigned char *src_buffer = (unsigned char *)malloc(src_size);

    FILE *src_file = fopen(input_file_name, "rb");
    if (NULL == src_file)
    {
        printf("open srcfile failure\n");
        return -1;
    }
    if (fread(src_buffer, 1, src_size, src_file) != src_size)
    {
        printf("read src file failure\n");
        return -1;
    }

    bsm_rgb24_to_jpeg(src_buffer, output_file_name, src_w, src_h);
    fclose(src_file);

    //读取像素数据方法二，该方法读取的数据，不能用于sws_scale()进行色彩模式转换，原因目前未知。
    //uint8_t *buffer = NULL;
    //size_t buffer_size;
    ///* slurp file content into buffer */
    //int ret1 = av_file_map(input_file_name, &buffer, &buffer_size, 0, NULL);
    //if (ret1 < 0)
    //{
    //    printf("read input file failure\n");
    //}
    //
    //bsm_rgb24_to_jpeg(buffer, output_file_name, src_w, src_h);

    //av_file_unmap(buffer, buffer_size);

    return 0;

#endif

}