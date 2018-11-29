/*****************************************************************************
bsm - bstorm distributed streaming analysing system
Copyright (c) 2018 boostiv , Inc.
File discription:This file can only be used under the boostiv authorilization.
Author:	yangdong
Datetime:2018/9/26
*****************************************************************************/

#ifndef BSTORM_IMAGE_ENCODER_H_
#define BSTORM_IMAGE_ENCODER_H_

namespace bsm{
namespace bsm_image_encoder{
    
typedef enum bsm_pixel_format
{
    yuv420p = 0,
    yuv422p,
    rgb24,
    rgbp,
    bgr24,
} bsm_pixel_format_e;

class bsm_image_decoder
{
public:
    bsm_image_decoder() {}
    ~bsm_image_decoder() {}

    /**
    *  description:
    *       get picture width and height depended on 'bsm_pixel_format_e', which, now, suport 5 format, as is:
    *           yuv420p
    *           yuv422p
    *           rgb24
    *           rgbp
    *           bgr24
    */
    int bsm_get_bytes_per_pixelformat(int width, int height, bsm_pixel_format_e pixel_format);

    bool bsm_yuv420p_to_rgb24(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size);
    bool bsm_rgb24_to_yuv420p(unsigned char *pdata_src, int src_width, int src_height, unsigned char *pdata_dst, int data_dst_size);

    bool bsm_yuv420p_to_jpeg(unsigned char *pdata_src, const char *output_file_name, int src_width, int src_height);
    bool bsm_rgb24_to_jpeg(unsigned char *pdata_src, const char *output_file_name, int src_width, int src_height);


private:
    bool bsm_pixel_format_convert(unsigned char *pdata_src, int src_width, int src_height, bsm_pixel_format_e src_pixfmt,
        unsigned char *pdata_dst, int dst_size, bsm_pixel_format_e dst_pixfmt);

};
} // namespace bsm_image_encoder
} // namespace bsm

#endif