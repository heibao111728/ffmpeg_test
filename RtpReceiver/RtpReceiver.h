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

//MAX_FRAME_SIZE, 100M�������ȡ����������Ƶ֡����������ظ�������Ƶ֡�����Խ��ÿռ�����
#define MAX_FRAME_SIZE 10 * 1024 
#define SDP_SIZE 4 * 1024

typedef struct rtp_header
{
    //��Ϊ�ֽ���ͬ�����Զ����ֶε�˳���ǰ���rfc3550�ж����˳���������巽���ȡ��
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
*   ���� GB28181 ��¼C
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

    char* getFrame();
    char* getSdpInfo();
    int generateSdpInfo();
    uint16_t getMediaPort();

    /**
    *   ���ܣ�
    *       ���� RTP ��������ѡ����ȷ�ĸ��ش�����
    *   function��
    *       choice, depend on RTP payload, right payload process function. 
    */
    int handlePacket(RTPPacket* packet);

    /**
    *   ���ܣ�
    *       �����յ��İ�ƴװ��������һ֡����
    *   function��
    *       assemle packet data to an full Frame
    */
    int handlePsPacket(RTPPacket* packet);
    int handleMPEG4Packet(RTPPacket* packet);
    int handleH264Packet(RTPPacket* packet);

    static void ThreadProc(void* pParam);   //�̺߳���
    int StartProc();
    void StopProc();
    HANDLE m_threadHandle;  //�߳̾��
    bool m_bThreadRuning;   //�߳�����״̬

    void write_media_data_to_file(char* file_name, void* pLog, int nLen);

    static void setup_callback_function(
        callback_get_ps_stream_fp get_ps_stream,
        callback_get_h264_stream_fp get_h264_stream,
        callback_get_mpeg4_stream_fp get_mpeg4_stream,
        callback_get_svac_stream_fp get_svac_stream);

private:
    RTPUDPv4TransmissionParams m_Transparams;
    RTPSessionParams m_Sessparams;
    RTPSession m_RtpSession;
    char m_SdpInfo[SDP_SIZE] = { 0 };
    uint16_t m_mediaPort;
    uint8_t m_pFrame[MAX_FRAME_SIZE];       //���PSý��֡����
    uint8_t m_pEsFrame[MAX_FRAME_SIZE];       //���ESý��֡����
    uint8_t* m_pTmpFrame;                   //��װ������֡�ѵ�ַ�����ڲ���ֿ⡣
    int m_offset;                           //λ��
    int m_frameSize;                        //����֡��С
    bool m_isMarkerPacket;                  //����֡rtp��ͷ���

    FILE* m_pLogFile;
    char m_ClientId[20 + 1];
    char m_ClientIp[20 + 1];

    //CDemuxer2 m_ps_demuxer;

    static callback_get_ps_stream_fp callback_get_ps_stream;
    static callback_get_h264_stream_fp callback_get_h264_stream;
    static callback_get_mpeg4_stream_fp callback_get_mpeg4_stream;
    static callback_get_svac_stream_fp callback_get_svac_stream;
};

#endif
