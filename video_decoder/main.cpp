#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"
#include "StreamManager\StreamManager.h"
#include "RtpReceiver\RtpReceiver.h"

/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

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

    data_length = CStreamManager::get_instance()->read_data(NULL, buffer, 100 * 1024);
    if (0 < data_length)
    {
        LOG("hava receive data, data_length=%d\n", data_length);
        write_media_data_to_file("E://callback_tmp1.ps", buffer, data_length);
    }
    return data_length;
}

int callback_read_data(void *opaque, uint8_t *buf, int buf_size)
{   
    int data_length = 0;

    
    while (buf_size != data_length)
    {
        data_length = CStreamManager::get_instance()->read_data(NULL, buf, buf_size);
        Sleep(1);
    }

    return data_length;
}

int callback_get_ps_stream(void *opaque, uint8_t *buf, int data_length)
{
    int write_data_length = 0;
    int read_data_length = 0;
    unsigned char buffer[100 * 1024] = { 0 };
    write_data_length = CStreamManager::get_instance()->write_data(buf, data_length);
    if (0 < write_data_length)
    {
        LOG("write data, data_length=%d\n", write_data_length);
        //read_data_length = CStreamManager::get_instance()->read_data(NULL, buffer, write_data_length);
        //write_media_data_to_file("E://callback_tmp1.ps", buffer, write_data_length);
    }
    return write_data_length;
}


#define __MAX_BUFFER_SIZE (2 * 1024 * 1024)
int main(int argc, char* argv[])
{
    WSADATA dat;
    WSAStartup(MAKEWORD(2, 2), &dat);
#if 0
    /**
    *   test CDemuxer, demux stream from file
    */
    CDemuxer::setup_callback_function(callback_read_data);
    CDemuxer demuxer;

    demuxer.set_input_ps_file("E://success_data//tmp1.ps");
    demuxer.set_output_es_video_file("E://success_data//tmp1.h264");

    demuxer.demux_ps_to_es();
#endif

#if 0
    /**
    *   test CDemuxer2, demux stream from file
    */
    CDemuxer2 ps_demuxer;

    int length_of_ps_header;
    length_of_ps_header = sizeof(ps_packet_header_t);

    ps_demuxer.setup_src_ps_file("E://success_data//tmp1.ps");
    ps_demuxer.setup_dst_es_video_file("E://success_data//tmp1.h264");

    if (ps_demuxer.open_src_ps_file())
    {
        ps_demuxer.do_demux();
    }

    ps_demuxer.close_src_ps_file();
    return 0;
#endif

#if 1
    /**
    *   test CDemuxer, demux stream from RTP.
    */

    CRtpReceiver::setup_callback_function(callback_get_ps_stream, NULL, NULL, NULL);
    CRtpReceiver rtp_recviver;
    rtp_recviver.set_cleint_ip("192.168.2.102");
    rtp_recviver.start_proc();

    CDemuxer::setup_callback_function(callback_read_data);
    CDemuxer demuxer;
    demuxer.set_output_es_video_file("E://demuxer_netstream.h264");
    demuxer.demux_ps_to_es_network();

    while (1)
    {
        //callback_read_data(NULL, NULL, 0);
        Sleep(10);
    }

    //while (1)
    //{
    //    if (0 < CStreamManager::get_instance()->read_data(NULL, buffer, __MAX_BUFFER_SIZE))
    //    {
    //        write_media_data_to_file("E://rtp_tmp1.ps", buffer, __MAX_BUFFER_SIZE);
    //    } 
    //}

    //if (buffer)
    //{
    //    free(buffer);
    //}

    return 0;

#endif 
}