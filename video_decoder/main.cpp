#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"
#include "RtpReceiver\RtpReceiver.h"
#include "StreamManager\StreamManager.h"

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


/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

using namespace bsm;
using namespace bsm_video_decoder;

void write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
    FILE* m_pLogFile = NULL ;
    if (pLog != NULL && nLen > 0)
    {
        if (NULL == m_pLogFile && strlen(file_name) > 0)
        {
            ::fopen_s(&m_pLogFile, file_name, "ab+");
        }

        if (m_pLogFile != NULL)
        {
            ::fwrite(pLog, nLen, 1, m_pLogFile);
            ::fclose(m_pLogFile);
            m_pLogFile = NULL;
        }
    }
}

int callback_read_data_to_file(void *opaque, uint8_t *buf, int buf_size)
{
    int data_length = 0;

    unsigned char buffer[100 * 1024] = { 0 };

    data_length = stream_manager::get_instance()->pull_data(NULL, buffer, 100 * 1024);
    if (0 < data_length)
    {
        LOG("hava receive data, data_length=%d\n", data_length);
        write_media_data_to_file("E://callback_tmp1.ps", buffer, data_length);
    }
    return data_length;
}

/**
*    callback function, used for bsm_demuxer2
*/

int callback_pull_ps_stream(void *opaque, uint8_t *buf, int buf_size)
{
    int data_length = 0;

    int recv_time = 0;
    while (recv_time < 50)
    {
        data_length = stream_manager::get_instance()->pull_data(NULL, buf, buf_size);
        if (0 < data_length)
        {
            LOG("hava receive data, data_length=%d\n", data_length);
        }
        recv_time++;
    }

    return data_length;
}

int callback_push_es_video_stream(void *opaque, uint8_t *data, int data_length)
{
    char* file_name = "E://demuxer_callback_stream.h264";
    FILE* p_file = NULL;
    int write_data_size = 0;
    if (data != NULL && data_length > 0)
    {
        if (NULL == p_file && strlen(file_name) > 0)
        {
            ::fopen_s(&p_file, file_name, "ab+");
        }

        if (p_file != NULL)
        {
            write_data_size = ::fwrite(data, data_length, 1, p_file);
            ::fclose(p_file);
            p_file = NULL;
        }
    }
    return write_data_size;
}




/**
*   callback function, used for rtpreceiver.
*/

int callback_push_ps_stream(void *opaque, uint8_t *buf, int data_length)
{
    int write_data_length = 0;
    int read_data_length = 0;
    unsigned char buffer[100 * 1024] = { 0 };
    write_data_length = stream_manager::get_instance()->push_data(buf, data_length);
    if (0 < write_data_length)
    {
        LOG("write data, data_length=%d\n", write_data_length);
        //read_data_length = stream_manager::get_instance()->pull_data(NULL, buffer, write_data_length);
        //write_media_data_to_file("E://callback_tmp1.ps", buffer, read_data_length);
    }
    return write_data_length;
}

int avio_read();

#define __MAX_BUFFER_SIZE (2 * 1024 * 1024)


int main(int argc, char* argv[])
{
    /**
    *   test bsm_demuxer, demux stream from file
    */
#if 0
    bsm_demuxer::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    bsm_demuxer demuxer;
    demuxer.set_output_es_video_file("E://tmp1.h264");

    demuxer.demux_ps_to_es_file("E://tmp1.ps");
    //demuxer.demux_ps_to_es_network();
#endif

    /**
    *   test bsm_demuxer, demux stream from network
    */
#if 1
    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);

    stream_manager::set_capacity_size(4 * 1024 * 1024);

    CRtpReceiver::setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    CRtpReceiver rtp_recviver;
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    bsm_demuxer::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    bsm_demuxer demuxer;
    demuxer.set_output_es_video_file("E://tmp1.h264");

    


    //demuxer.demux_ps_to_es_file("E://tmp1.ps");
    demuxer.demux_ps_to_es_network();

    while (1)
    {
        //callback_read_data_to_file(NULL, NULL, 0);
    }
#endif



    /**
    *   test bsm_demuxer2, demux stream from RTP.
    */
#if 0

    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);

    stream_manager::set_capacity_size(4*1024*1024);

    CRtpReceiver::setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    CRtpReceiver rtp_recviver;
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    demuxer2::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    demuxer2 demuxer2;

    demuxer2.demux_ps_to_es_network();

    while (1)
    {
        //callback_read_data(NULL, NULL, 0);
        Sleep(10);
    }

    return 0;

#endif 

    /**
    *   test bsm_demuxer2, demux stream from file.
    */
#if 0

    bsm_demuxer2::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    bsm_demuxer2 demuxer2;

    demuxer2.demux_ps_to_es_file("E://tmp1.ps");

    while (1)
    {
        //callback_read_data(NULL, NULL, 0);
        Sleep(10);
    }

    return 0;

#endif 

    //avio_read();
}


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

static int write_packet(void *opaque, uint8_t *buf, int buf_size)
{
    char* file_name = "E://ffmpeg_avio_read_callback_write_data.h264";
    FILE* p_file = NULL;
    int write_data_size = 0;
    if (buf != NULL && buf_size > 0)
    {
        if (NULL == p_file && strlen(file_name) > 0)
        {
            ::fopen_s(&p_file, file_name, "ab+");
        }

        if (p_file != NULL)
        {
            write_data_size = ::fwrite(buf, buf_size, 1, p_file);
            ::fclose(p_file);
            p_file = NULL;
        }
    }
    return write_data_size;

    return buf_size;
}

int avio_read()
{
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;

    AVStream *out_stream_es_video = NULL;
    AVStream *in_stream_ps = NULL;
    int frame_index = 0;
    int videoindex = -1;
    AVPacket av_packet;


    uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    char *input_filename = NULL;
    int ret = 0;
    struct buffer_data bd = { 0 };

    input_filename = "E://tmp1.ps";

    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
        goto end;

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(fmt_ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
        0, &bd, &read_packet, &write_packet, NULL);
    if (!avio_ctx) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    fmt_ctx->pb = avio_ctx;

    ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }

    av_dump_format(fmt_ctx, 0, input_filename, 0);

    //打开输出流
    out_stream_es_video = avformat_new_stream(fmt_ctx, fmt_ctx->video_codec);

    if (!out_stream_es_video)
    {
        LOG("Allocating output stream failed.");
        return false;
    }

    // Write file header
    if (avformat_write_header(fmt_ctx, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return false;
    }

    //获取视频流索引
    for (int i = 0; i < fmt_ctx->nb_streams; ++i)
    {
        in_stream_ps = fmt_ctx->streams[i];

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
        if (av_read_frame(fmt_ctx, &av_packet) < 0)
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
        if (av_interleaved_write_frame(fmt_ctx, &av_packet) < 0)
        {
            LOG("Error when write_frame.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);

        av_packet_unref(&av_packet);

        ++frame_index;
    }

    // Write file trailer
    if (av_write_trailer(fmt_ctx) != 0)
    {
        LOG("Error occurred when writing file trailer.");
        return false;
    }

end:
    avformat_close_input(&fmt_ctx);
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    av_file_unmap(buffer, buffer_size);

    if (ret < 0) {
        //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
