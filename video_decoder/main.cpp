#include "stdio.h"

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
#include <libavutil/mathematics.h>
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

#define LOG(fmt, ...) fprintf(stdout, "[DEBUG] %s:\n%s:%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

int main(int argc, char* argv[])
{
    AVIOContext *av_io_context;
    AVInputFormat * av_input_formate = NULL;
    AVFormatContext *av_formate_context_input;
    AVFormatContext *av_formate_context_out = NULL;
    AVFormatContext *av_formate_context_out_video = NULL;
    AVFormatContext *av_formate_context_out_audio = NULL;

    //AVInputFormat *av_input_formate;
    AVOutputFormat *av_output_formate_out = NULL;

    AVPacket pkt;

    AVStream *in_stream_ps = NULL;
    AVStream *out_stream_es_video = NULL;
    AVStream *out_stream_es_audio = NULL;

    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    char *in_ps_filename = "E://tmp1.ps";           // Input file URL
    char *out_es_filename_v = "E://tmp1.h264";      // Output file URL
    char *out_es_filename_a = "E://tmp1.aac";

    int i_ret;

    av_formate_context_input = avformat_alloc_context();

    //打开一个输入流，并读取头信息。
    i_ret = avformat_open_input(&av_formate_context_input, in_ps_filename, av_input_formate, NULL);
    if(i_ret < 0)
    {
        LOG("Open input file failed.");
        return -1;
    }

    //获取媒体流信息
    i_ret = avformat_find_stream_info(av_formate_context_input, NULL);
    if (i_ret < 0)
    {
        LOG("Retrieve iniput stream info failed.");
        return -1;
    }

    // 获取输出文件格式信息
    avformat_alloc_output_context2(&av_formate_context_out_video, NULL, NULL, out_es_filename_v);
    if (!av_formate_context_out_video)
    {
        LOG("Create output context failed.");
        return -1;
    }
    else
    {
        av_output_formate_out = av_formate_context_out_video->oformat;
    }

    for (int i = 0; i < av_formate_context_input->nb_streams; ++i)
    {
        in_stream_ps = av_formate_context_input->streams[i];

        if (AVMEDIA_TYPE_VIDEO == av_formate_context_input->streams[i]->codecpar->codec_type) {
            videoindex = i;
            out_stream_es_video = avformat_new_stream(av_formate_context_out_video, av_formate_context_input->video_codec);
            av_formate_context_out = av_formate_context_out_video;
        }
        else 
        {
            break;
        }
        LOG("hahahahaha.");
    }

    if (!out_stream_es_video) 
    {
        LOG("Allocating output stream failed.");
        return -1;
    }

    //// Copy the settings of AVCodecContext
    //if (avcodec_copy_context(av_formate_context_input->video_codec, av_formate_context_input->video_codec) < 0)
    //{
    //    LOG("Copy context from input to output stream codec context.");
    //    return -1;
    //}

    // Dump Format
    LOG("Input Video Start!");
    av_dump_format(av_formate_context_input, 0, in_ps_filename, 0);
    LOG("Output Video Start!");
    av_dump_format(av_formate_context_out_video, 0, out_es_filename_v, 1);
    LOG("Dump End");

    // Open output file
    if (!(av_output_formate_out->flags & AVFMT_NOFILE)) 
    {
        if (avio_open(&av_formate_context_out_video->pb, out_es_filename_v, AVIO_FLAG_WRITE) < 0)
        {
            LOG("Could not open output file '%s'", out_es_filename_v);
            return -1;
        }
    }

    // Write file header
    if (avformat_write_header(av_formate_context_out_video, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return -1;
    }

    frame_index = 0;

    while (1)
    {
        // Get an AVPacket
        if (av_read_frame(av_formate_context_input, &pkt) < 0) 
        {
            break;
        }

        in_stream_ps = av_formate_context_input->streams[pkt.stream_index];

        if (videoindex == pkt.stream_index)
        {
            out_stream_es_video = av_formate_context_out_video->streams[0];
            LOG("Write Video Packet. size: %d\t pts: %d\n", pkt.size, pkt.pts);
        }
        else 
        {
            continue;
        }

        // Copy Packet
        // Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream_ps->time_base, out_stream_es_video->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream_ps->time_base, out_stream_es_video->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream_ps->time_base, out_stream_es_video->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;

        // Write
        if (av_interleaved_write_frame(av_formate_context_out_video, &pkt) < 0)
        {
            LOG("Error muxing packet.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);
        av_packet_unref(&pkt);
        ++frame_index;
    }

    avformat_free_context(av_formate_context_out);
    avformat_free_context(av_formate_context_input);
}