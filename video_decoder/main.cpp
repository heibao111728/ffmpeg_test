#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"
#include "RtpReceiver\RtpReceiver.h"
#include "StreamManager\StreamManager.h"
#include "utils\logger.h"

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
    //while (recv_time < 50)
    //{
        data_length = stream_manager::get_instance()->pull_data(NULL, buf, buf_size);
        if (0 < data_length)
        {
            //LOG("hava receive data, data_length=%d\n", data_length);
        }
        recv_time++;
    //}

    return data_length;
}

/**
*    callback function, used for bsm_demuxer, which use ffmpeg.
*/
int callback_pull_ps_stream_dexuxer(void *opaque, uint8_t *buf, int buf_size)
{
    int recv_date_length = 0;
    while (recv_date_length != buf_size)
    {
        recv_date_length = stream_manager::get_instance()->pull_data(NULL, buf, buf_size);
        //LOG("src stream_manager don't have enought data, callback will waite a moment.\n");
    }
    LOG("receive data success, receive size = %d.\n", recv_date_length);

    return recv_date_length;
}

int callback_push_es_video_stream(void *opaque, uint8_t *data, int data_length)
{
    char* file_name = "E://demuxer_callback_stream_demuxer_network.h264";
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

int callback_push_es_video_stream_file(void *opaque, uint8_t *data, int data_length)
{
    char* file_name = "E://demuxer_callback_stream_demuxer2_file.h264";
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
        //LOG("write data, data_length=%d\n", write_data_length);
        //read_data_length = stream_manager::get_instance()->pull_data(NULL, buffer, write_data_length);
        //write_media_data_to_file("E://callback_tmp1.ps", buffer, read_data_length);
    }
    return write_data_length;
}

#define __MAX_BUFFER_SIZE (2 * 1024 * 1024)


int main(int argc, char* argv[])
{
    bsm_logger::set_log_type(log_type_file);
    if (!bsm_logger::get_instance()->init_logger("E://bsm_video_decoder.log"))
    {
        return -1;
    }

    /**
    *   test bsm_demuxer, demux stream from file
    */
#if 1
    bsm_demuxer::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    bsm_demuxer demuxer;
    demuxer.set_output_es_video_file("E://demuxer_callback_stream_demuxer_file.h264");

    demuxer.demux_ps_to_es_file("E://rtpreciver_tmp1.ps");
    //demuxer.demux_ps_to_es_network();
#endif

    /**
    *   test bsm_demuxer, demux stream from network
    */
#if 0
    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);

    stream_manager::set_capacity_size(4 * 1024 * 1024);

    CRtpReceiver::setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    CRtpReceiver rtp_recviver;
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    //Sleep(5000);

    bsm_demuxer::setup_callback_function(callback_pull_ps_stream_dexuxer, callback_push_es_video_stream, NULL);
    bsm_demuxer demuxer;

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

    bsm_demuxer2::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);
    bsm_demuxer2 demuxer2;

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

    bsm_demuxer2::setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream_file, NULL);
    bsm_demuxer2 demuxer2;

    demuxer2.demux_ps_to_es_file("E://tmp1.ps");

    //unsigned char ps_packet[20];
    //ps_packet[0] = 0x00;
    //ps_packet[1] = 0x00;
    //ps_packet[2] = 0x00;
    //ps_packet[3] = 0x00;
    //ps_packet[4] = 0x01;
    //ps_packet[5] = 0xba;
    //ps_packet[6] = 0xaa;
    //ps_packet[7] = 0xbb;
    //ps_packet[8] = 0xcc;
    //ps_packet[9] = 0xdd;
    //ps_packet[10] = 0xee;
    //ps_packet[11] = 0xff;
    //ps_packet[12] = 0xaa;
    //ps_packet[13] = 0xbb;
    //ps_packet[14] = 0xcc;
    //ps_packet[15] = 0xdd;
    //ps_packet[16] = 0x00;
    //ps_packet[17] = 0x00;
    //ps_packet[18] = 0x00;
    //ps_packet[19] = 0xba;


    //unsigned char ps_packet_start_code[4];
    //ps_packet_start_code[0] = 0x00;
    //ps_packet_start_code[1] = 0x00;
    //ps_packet_start_code[2] = 0x01;
    //ps_packet_start_code[3] = 0xba;

    //int ps_packet_length = 0;
    //int ps_packet_start_position = 0;

    //if (demuxer2.find_next_ps_packet(ps_packet, 20, &ps_packet_start_position, &ps_packet_length))
    //{
    //    LOG("find success");
    //}
    //else
    //{
    //    LOG("not find.");
    //}

    while (1)
    {
        //callback_read_data(NULL, NULL, 0);
        Sleep(10);
    }

    return 0;

#endif 

    //avio_read();
    bsm_logger::get_instance()->uninit_logger();
}
