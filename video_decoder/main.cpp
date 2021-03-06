#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"
#include "RtpReceiver\RtpReceiver.h"
#include "StreamManager\StreamManager.h"
#include "utils\logger.h"

/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

using namespace bsm;
using namespace bsm_video_decoder;

stream_manager g_stream_manager;

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

    data_length = g_stream_manager.pull_data(NULL, buffer, 100 * 1024);
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
    int recv_date_length = 0;
    while (recv_date_length != buf_size)
    {
        recv_date_length = g_stream_manager.pull_data(NULL, buf, buf_size);
        //LOG("src stream_manager don't have enought data, callback will waite a moment.\n");
    }
    LOG("receive data success, receive size = %d.\n", recv_date_length);

    return recv_date_length;
}

/**
*    callback function, used for bsm_demuxer, which use ffmpeg.
*/
int callback_pull_ps_stream_dexuxer(void *opaque, uint8_t *buf, int buf_size)
{
    int recv_date_length = 0;
    while (recv_date_length != buf_size)
    {
        recv_date_length = g_stream_manager.pull_data(NULL, buf, buf_size);
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
    write_data_length = g_stream_manager.push_data(buf, data_length);
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
#if 0
    bsm_demuxer demuxer;
    demuxer.set_output_es_video_file("E://demuxer_callback_stream_demuxer_file.h264");

    demuxer.demux_ps_to_es_file("E://rtpreciver_tmp1.ps");
#endif

    /**
    *   test bsm_demuxer, demux stream from network
    */
#if 0
    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);

    g_stream_manager.set_capacity_size(4 * 1024 * 1024);

    CRtpReceiver rtp_recviver;
    rtp_recviver.setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    //Sleep(5000);

    bsm_demuxer demuxer;
    demuxer.setup_callback_function(callback_pull_ps_stream_dexuxer, callback_push_es_video_stream, NULL);

    demuxer.demux_ps_to_es_network();

    while (1)
    {
        //callback_read_data_to_file(NULL, NULL, 0);
    }
#endif



    /**
    *   test bsm_demuxer2, demux stream from RTP.
    */
#if 1

    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);

    g_stream_manager.set_capacity_size(4*1024*1024);

    CRtpReceiver rtp_recviver;
    rtp_recviver.setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    bsm_demuxer2 demuxer2;
    demuxer2.setup_callback_function(callback_pull_ps_stream, callback_push_es_video_stream, NULL);

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

    bsm_demuxer2 demuxer2;
    demuxer2.setup_callback_function(NULL, callback_push_es_video_stream_file, NULL);

    demuxer2.demux_ps_to_es_file("E://rtpreciver_tmp1.ps");

    while (1)
    {
        Sleep(10);
    }

    return 0;

#endif 

    bsm_logger::get_instance()->uninit_logger();
}
