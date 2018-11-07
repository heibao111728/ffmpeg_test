#ifndef __RTPRECEIVER_H__
#define __RTPRECEIVER_H__

//jrtplib headers 
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpsourcedata.h"

using namespace jrtplib;

#define SDP_SIZE 4 * 1024

typedef struct rtp_header
{
    //因为字节序不同，所以定义字段的顺序不是按照rfc3550中定义的顺序，这样定义方便存取；
    //because of bit sequence, we define this struct not as order define in rfc3550.
    //LITTLE_ENDIAN
    unsigned short   cc : 4;    // CSRC count
    unsigned short   x  : 1;     // header extension flag
    unsigned short   p  : 1;     // padding flag
    unsigned short   v  : 2;     // packet type
    unsigned short   pt : 7;    // payload type
    unsigned short   m  : 1;     // marker bit
    unsigned short   seq;       // sequence number
    unsigned long    ts;        // timestamp
    unsigned long    ssrc;      // synchronization source
}rtp_header_t;

/**
*   依据 GB28181 附录C
*/
enum PAYLOADTYPE
{
    PS      = 96,
    MPEG4   = 97,
    H264    = 98,
    SVAC    = 99
};

typedef int(*callback_get_ps_stream_fp)(void *opaque, uint8_t *buf, int data_length);
typedef int(*callback_get_h264_stream_fp)(void *opaque, uint8_t *buf, int data_length);
typedef int(*callback_get_mpeg4_stream_fp)(void *opaque, uint8_t *buf, int data_length);
typedef int(*callback_get_svac_stream_fp)(void *opaque, uint8_t *buf, int data_length);

class CRtpReceiver
{
public:
    CRtpReceiver(unsigned short rtpPort = 9000);
    ~CRtpReceiver();

    char* get_sdp_info();
    int generate_sdp_info();
    uint16_t get_media_port();

    /**
    *   功能：
    *       根据 RTP 负载类型选择正确的负载处理函数
    *   function：
    *       choice, depend on RTP payload, right payload process function. 
    */
    int handle_packet(RTPPacket* packet);

    /**
    *   功能：
    *       将接收到的包拼装成完整的一帧数据
    *   function：
    *       assemle packet data to an full Frame
    */
    int handle_ps_packet(RTPPacket* packet);
    int handle_mpeg4_packet(RTPPacket* packet);
    int handle_h264_packet(RTPPacket* packet);

    static void thread_proc(void* pParam);   //线程函数
    int start_proc();
    void stop_proc();
    HANDLE m_thread_handle;  //线程句柄
    bool m_b_thread_runing;   //线程运行状态
    
    bool set_cleint_ip(char* ip);
    bool set_media_port(unsigned short port);
    bool set_client_id(char* id);

    void write_media_data_to_file(char* file_name, void* pLog, int nLen);

    static void setup_callback_function(
        callback_get_ps_stream_fp get_ps_stream,
        callback_get_h264_stream_fp get_h264_stream,
        callback_get_mpeg4_stream_fp get_mpeg4_stream,
        callback_get_svac_stream_fp get_svac_stream);

private:
    //RTPSession m_rtp_session;
    char m_sdp_info[SDP_SIZE] = { 0 };

    bool m_is_marker_packet;                  //完整帧rtp包头标记

    char m_client_id[20 + 1];
    char m_client_ip[20 + 1];
    uint16_t m_media_port;

    static callback_get_ps_stream_fp m_callback_get_ps_stream;
    static callback_get_h264_stream_fp m_callback_get_h264_stream;
    static callback_get_mpeg4_stream_fp m_callback_get_mpeg4_stream;
    static callback_get_svac_stream_fp m_callback_get_svac_stream;
};

#endif
