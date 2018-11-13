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

#include "Demuxer.h"
#include "stdio.h"
#include "StreamManager\StreamManager.h"

namespace bsm {
namespace bsm_video_decoder {

#define BUF_SIZE (1*1024*1024)
#define AVCODEC_MAX_AUDIO_FRAME_SIZE (8*1024)

callback_pull_ps_stream_demuxer bsm_demuxer::m_callback_pull_ps_stream = NULL;
callback_push_es_video_stream_demuxer bsm_demuxer::m_callback_push_es_video_stream = NULL;
callback_push_es_audio_stream_demuxer bsm_demuxer::m_callback_push_es_audio_stream = NULL;

void bsm_demuxer::set_output_es_video_file(char* file_name)
{
    memset(m_output_es_video_file_name, 0x00, MAX_FILE_NAME_LENGTH___BSM_DEMUXER);
    if (strlen(file_name) > 0)
    {
        sprintf_s(m_output_es_video_file_name, MAX_FILE_NAME_LENGTH___BSM_DEMUXER, "%s", file_name);
    }
}

void bsm_demuxer::set_output_es_audio_file(char* file_name)
{
    memset(m_output_es_audio_file_name, 0x00, MAX_FILE_NAME_LENGTH___BSM_DEMUXER);
    if (strlen(file_name) > 0)
    {
        sprintf_s(m_output_es_audio_file_name, MAX_FILE_NAME_LENGTH___BSM_DEMUXER, "%s", file_name);
    }
}

bool bsm_demuxer::demux_ps_to_es_file(char* ps_file_name)
{
    uint8_t *media_buffer = (uint8_t *)av_malloc(sizeof(uint8_t) * BUF_SIZE);
    //AVInputFormat * av_input_formate = NULL;
    AVOutputFormat *av_output_formate = NULL;

    AVFormatContext *av_formate_context_input;
    AVFormatContext *av_formate_context_out_video = NULL;
    AVFormatContext *av_formate_context_out_audio = NULL;

    AVPacket av_packet;

    AVStream *in_stream_ps = NULL;
    AVStream *out_stream_es_video = NULL;
    AVStream *out_stream_es_audio = NULL;

    AVIOContext *av_io_context_output = NULL;

    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    int i_ret;

    av_formate_context_input = avformat_alloc_context();

    //打开一个输入流，并读取头信息。
    i_ret = avformat_open_input(&av_formate_context_input, ps_file_name, 0, NULL);
    if (i_ret < 0)
    {
        LOG("Open input file failed.");
        return false;
    }

    //获取媒体流信息
    i_ret = avformat_find_stream_info(av_formate_context_input, NULL);
    if (i_ret < 0)
    {
        LOG("Retrieve iniput stream info failed.");
        return false;
    }

    //获取输出文件格式信息
    avformat_alloc_output_context2(&av_formate_context_out_video, NULL, NULL, m_output_es_video_file_name);
    if (!av_formate_context_out_video)
    {
        LOG("Create output context failed.");
        return false;
    }
    else
    {
        av_output_formate = av_formate_context_out_video->oformat;
    }

    //指定输出格式
    //av_output_format = av_guess_format("h264", NULL, NULL);
    //av_formate_context_out_video = avformat_alloc_context();
    //av_formate_context_out_video->oformat = av_output_format;
    

    // Open output file
    if (!(av_output_formate->flags & AVFMT_NOFILE))
    {
        if (avio_open(&av_formate_context_out_video->pb, m_output_es_video_file_name, AVIO_FLAG_WRITE) < 0)
        {
            LOG("Could not open output file '%s'", m_output_es_video_file_name);
            return false;
        }
    }

    //打开输出流
    out_stream_es_video = avformat_new_stream(av_formate_context_out_video, av_formate_context_out_video->video_codec);

    if (!out_stream_es_video)
    {
        LOG("Allocating output stream failed.");
        return false;
    }

    // Write file header
    if (avformat_write_header(av_formate_context_out_video, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return false;
    }

    //获取视频流索引
    for (int i = 0; i < av_formate_context_input->nb_streams; ++i)
    {
        in_stream_ps = av_formate_context_input->streams[i];

        if (AVMEDIA_TYPE_VIDEO == in_stream_ps->codecpar->codec_type)
        {
            videoindex = i;
        }
        else
        {
            break;
        }
    }

    frame_index = 0;

    while (1)
    {
        // Get an AVPacket
        if (av_read_frame(av_formate_context_input, &av_packet) < 0)
        {
            LOG("end of file!\n");
            break;
        }

        if (videoindex == av_packet.stream_index)
        {
            LOG("Write Video Packet. size: %d\t pts: %d\n", av_packet.size, av_packet.pts);
        }
        else
        {
            continue;
        }

        // Write
        if (av_interleaved_write_frame(av_formate_context_out_video, &av_packet) < 0)
        {
            LOG("Error when write_frame.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);

        av_packet_unref(&av_packet);

        ++frame_index;
    }

    // Write file trailer
    if (av_write_trailer(av_formate_context_out_video) != 0)
    {
        LOG("Error occurred when writing file trailer.");
        return false;
    }

    avformat_free_context(av_formate_context_out_video);
    avformat_free_context(av_formate_context_input);

    return true;
}

void bsm_demuxer::setup_callback_function(callback_pull_ps_stream_demuxer pull_ps_stream,
    callback_push_es_video_stream_demuxer push_es_video_stream,
    callback_push_es_audio_stream_demuxer push_es_audio_stream)
{
    m_callback_pull_ps_stream = pull_ps_stream;
    m_callback_push_es_video_stream = push_es_video_stream;
    m_callback_push_es_audio_stream = push_es_audio_stream;
}

bool bsm_demuxer::demux_ps_to_es_network()
{
    AVIOContext *av_io_context_input = NULL;
    AVIOContext *av_io_context_output = NULL;

    AVInputFormat * input_formate = NULL;
    AVOutputFormat *av_output_format = NULL;

    AVFormatContext *av_format_context_input = NULL;
    AVFormatContext *av_formate_context_out_video = NULL;
    
    AVStream *in_stream_ps = NULL;
    AVStream *out_stream_es;
    AVPacket av_packet;

    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    int ret;

    unsigned char* input_buffer = (unsigned char*)av_malloc(RTP_V4_RECEIVE_BUFFER);
    unsigned char* output_buffer = (unsigned char*)av_malloc(RTP_V4_RECEIVE_BUFFER);

    // input
    av_format_context_input = avformat_alloc_context();
    av_io_context_input = avio_alloc_context(input_buffer, RTP_V4_RECEIVE_BUFFER, 0, NULL, m_callback_pull_ps_stream, NULL, NULL);
    if (av_io_context_input == NULL)
    {
        return -1;
    }
    av_format_context_input->pb = av_io_context_input;
    av_format_context_input->flags = AVFMT_FLAG_CUSTOM_IO;


    if ((ret = avformat_open_input(&av_format_context_input, NULL, NULL, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    input_formate = av_find_input_format("mpeg");
    av_format_context_input->iformat = input_formate;

    if ((ret = avformat_find_stream_info(av_format_context_input, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    //output
    av_io_context_output = avio_alloc_context(output_buffer, RTP_V4_RECEIVE_BUFFER, 1, NULL, NULL, m_callback_push_es_video_stream, NULL);
    if (av_io_context_output == NULL)
    {
        return -1;
    }

    av_formate_context_out_video = avformat_alloc_context();
    av_formate_context_out_video->pb = av_io_context_output;
    av_formate_context_out_video->flags = AVFMT_FLAG_CUSTOM_IO;

    av_output_format = av_guess_format("h264", NULL, NULL);
    av_formate_context_out_video->oformat = av_output_format;

    out_stream_es = avformat_new_stream(av_formate_context_out_video, NULL);
    if (!out_stream_es)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        return AVERROR_UNKNOWN;
    }

    // Write file header
    if (avformat_write_header(av_formate_context_out_video, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return false;
    }

    //获取视频流索引
    for (int i = 0; i < av_format_context_input->nb_streams; ++i)
    {
        in_stream_ps = av_format_context_input->streams[i];

        if (AVMEDIA_TYPE_VIDEO == in_stream_ps->codecpar->codec_type)
        {
            videoindex = i;
        }
        else
        {
            break;
        }
    }

    frame_index = 0;

    while (1)
    {
        // Get an AVPacket
        if (av_read_frame(av_format_context_input, &av_packet) < 0)
        {
            LOG("have done!\n");
            break;
        }

        if (videoindex == av_packet.stream_index)
        {
            LOG("Write Video Packet. size: %d\t pts: %d\n", av_packet.size, av_packet.pts);
        }
        else
        {
            continue;
        }

        // Write
        if (av_interleaved_write_frame(av_formate_context_out_video, &av_packet) < 0)
        {
            LOG("Error when write_frame.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);

        av_packet_unref(&av_packet);

        ++frame_index;
    }

    // Write file trailer
    if (av_write_trailer(av_formate_context_out_video) != 0)
    {
        LOG("Error occurred when writing file trailer.");
        return false;
    }

    if (input_buffer)
    {
        av_free(input_buffer);
    }
    
    if (output_buffer)
    {
        av_free(output_buffer);
    }
    
    avio_context_free(&av_io_context_input);
    avio_context_free(&av_io_context_output);
    avformat_free_context(av_format_context_input);
    avformat_free_context(av_formate_context_out_video);
}


bool bsm_demuxer::demux_ps_file_to_es_stream(char* ps_file_name)
{
    AVOutputFormat *av_output_formate = NULL;
    AVFormatContext *av_formate_context_input;
    AVFormatContext *av_formate_context_out_video = NULL;

    AVStream *in_stream_ps = NULL;

    AVPacket av_packet;

    AVIOContext *av_io_context = NULL;

    int i_ret;
    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    uint8_t *media_buffer = (uint8_t *)av_malloc(sizeof(uint8_t) * BUF_SIZE);

    av_io_context = avio_alloc_context(media_buffer, BUF_SIZE, 0, NULL, NULL, m_callback_push_es_video_stream, NULL);
    if (!av_io_context)
    {
        fprintf(stderr, "avio alloc failed!\n");
        return -1;
    }

    av_formate_context_out_video = avformat_alloc_context();
    av_formate_context_out_video->pb = av_io_context;

    av_formate_context_input = avformat_alloc_context();

    //打开一个输入流，并读取头信息。
    i_ret = avformat_open_input(&av_formate_context_input, ps_file_name, 0, NULL);
    if (i_ret < 0)
    {
        LOG("Open input file failed.");
        return false;
    }

    //获取媒体流信息
    i_ret = avformat_find_stream_info(av_formate_context_input, NULL);
    if (i_ret < 0)
    {
        LOG("Retrieve iniput stream info failed.");
        return false;
    }

    // Write file header
    if (avformat_write_header(av_formate_context_out_video, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return false;
    }

    //获取视频流索引
    for (int i = 0; i < av_formate_context_input->nb_streams; ++i)
    {
        in_stream_ps = av_formate_context_input->streams[i];

        if (AVMEDIA_TYPE_VIDEO == in_stream_ps->codecpar->codec_type)
        {
            videoindex = i;
        }
        else
        {
            break;
        }
    }

    frame_index = 0;

    while (1)
    {
        // Get an AVPacket
        if (av_read_frame(av_formate_context_input, &av_packet) < 0)
        {
            LOG("end of file!\n");
            break;
        }

        if (videoindex == av_packet.stream_index)
        {
            LOG("Write Video Packet. size: %d\t pts: %d\n", av_packet.size, av_packet.pts);
        }
        else
        {
            continue;
        }

        // Write
        if (av_interleaved_write_frame(av_formate_context_out_video, &av_packet) < 0)
        {
            LOG("Error when write_frame.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);

        av_packet_unref(&av_packet);

        ++frame_index;
    }

    // Write file trailer
    if (av_write_trailer(av_formate_context_out_video) != 0)
    {
        LOG("Error occurred when writing file trailer.");
        return false;
    }

    avio_context_free(&av_io_context);
    avformat_free_context(av_formate_context_input);
    avformat_free_context(av_formate_context_out_video);
}

} // namespace bsm_video_decoder
} // namespace bsm