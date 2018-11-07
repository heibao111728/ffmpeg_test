#include "RtpReceiver.h"
#include <process.h>

callback_get_ps_stream_fp CRtpReceiver::m_callback_get_ps_stream = NULL;
callback_get_h264_stream_fp CRtpReceiver::m_callback_get_h264_stream = NULL;
callback_get_mpeg4_stream_fp CRtpReceiver::m_callback_get_mpeg4_stream = NULL;
callback_get_svac_stream_fp CRtpReceiver::m_callback_get_svac_stream = NULL;

CRtpReceiver::CRtpReceiver(unsigned short rtpPort)
    :m_media_port(rtpPort)
{

}

CRtpReceiver::~CRtpReceiver()
{
}

char* CRtpReceiver::get_sdp_info()
{
    return m_sdp_info;
}

int CRtpReceiver::generate_sdp_info()
{
    char pMediaPort[10] = { 0 };
    _itoa_s(m_media_port, pMediaPort, 10);

    char pSsrc[50] = { 0 };
    sprintf_s(pSsrc, "%s", "999999");

    sprintf_s(m_sdp_info, 4 * 1024,
        "v=0\r\n"
        "o=%s 0 0 IN IP4 %s\r\n"
        "s=Play\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=video %s RTP/AVP 96 98 97\r\n"
        "a=recvonly\r\n"
        "a=rtpmap:96 PS/90000\r\n"
        "a=rtpmap:98 H264/90000\r\n"
        "a=rtpmap:97 MPEG-4/90000\r\n"
        "y=%s\r\n"
        "f=\r\n",
        m_client_id,
        m_client_ip,
        m_client_ip,
        pMediaPort,
        pSsrc);

    return 0;
}

uint16_t CRtpReceiver::get_media_port()
{
    return m_media_port;
}

int CRtpReceiver::start_proc()
{
    m_thread_handle = (HANDLE)_beginthread(thread_proc, 0, (void*)this);
    if (0 == m_thread_handle)
    {
        //线程启动失败
        return -1;
    }
    m_b_thread_runing = true;

    //生成sdp信息
    generate_sdp_info();

    return 0;
}

void CRtpReceiver::stop_proc()
{
    m_b_thread_runing = false;
}

void CRtpReceiver::thread_proc(void* pParam)
{
    CRtpReceiver* pThis = (CRtpReceiver*)pParam;

    RTPUDPv4TransmissionParams Transparams;
    RTPSessionParams Sessparams;
    RTPSession rtp_session;

    Sessparams.SetOwnTimestampUnit(1.0 / 8000.0);
    Sessparams.SetAcceptOwnPackets(true);

    Transparams.SetPortbase(pThis->m_media_port);

    int status, i, num;

    status = rtp_session.Create(Sessparams, &Transparams);
    if (0 != status)
    {
        printf("create rtp session failure. check if you init windows network content.\n");
        exit(0);
    }

    while (pThis->m_b_thread_runing)
    {
        rtp_session.BeginDataAccess();

        // check incoming packets
        if (rtp_session.GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *pack;

                while ((pack = rtp_session.GetNextPacket()) != NULL)
                {
                    //pThis->assemleFrame(pack);
                    pThis->handle_packet(pack);
                    // we don't longer need the packet, so
                    // we'll delete it
                    rtp_session.DeletePacket(pack);
                }
            } while (rtp_session.GotoNextSourceWithData());
        }
        rtp_session.EndDataAccess();
        RTPTime::Wait(RTPTime(1, 0));
    }

    rtp_session.BYEDestroy(RTPTime(10, 0), 0, 0);
}

int CRtpReceiver::handle_packet(RTPPacket* packet)
{
    int packetSize = 0;
    uint8_t packetPayloadType;

    if (NULL == packet)
    {
        return 0;
    }

    packetSize = packet->GetPacketLength();
    packetPayloadType = packet->GetPayloadType();

    //按负载类型处理负载数据
    switch (packetPayloadType)
    {
    case PS: //96
    {
        handle_ps_packet( packet );
        break;
    }
    case MPEG4: //97
    {
        break;
    }
    case H264: //98
    {
        handle_h264_packet(packet);
        break;
    }
    case SVAC: //99
    {
        break;
    }
    default:
    {
        break;
    }
    }
}

int CRtpReceiver::handle_ps_packet(RTPPacket* packet)
{
    if (packet && m_callback_get_ps_stream)
    {
        write_media_data_to_file("E://rtpreciver_tmp1.ps", packet->GetPayloadData(), packet->GetPayloadLength());
        m_callback_get_ps_stream(NULL, packet->GetPayloadData(), packet->GetPayloadLength());
        return packet->GetPayloadLength();
    }
    else
    {
        return 0;
    }
}

int CRtpReceiver::handle_mpeg4_packet(RTPPacket* packet)
{
    return 0;
}

int CRtpReceiver::handle_h264_packet(RTPPacket* packet)
{
    if (packet && m_callback_get_h264_stream)
    {
        //write_media_data_to_file("E://rtpreciver_tmp1.ps", packet->GetPayloadData(), packet->GetPayloadLength());
        m_callback_get_h264_stream(NULL, packet->GetPayloadData(), packet->GetPayloadLength());
        return packet->GetPayloadLength();
    }
    else
    {
        return 0;
    }
}

void CRtpReceiver::write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
    FILE* pLogFile = NULL;
    if (pLog != NULL && nLen > 0)
    {
        if (NULL == pLogFile && strlen(file_name) > 0)
        {
            ::fopen_s(&pLogFile, file_name, "ab+");
        }

        if (pLogFile != NULL)
        {
            ::fwrite(pLog, nLen, 1, pLogFile);
            ::fclose(pLogFile);
            pLogFile = NULL;
        }
    }
}

bool CRtpReceiver::set_cleint_ip(char* ip)
{
    if (strlen(ip) < 7)
    {
        return false;
    }
    else
    {
        sprintf(m_client_ip, "%s", ip);
        return true;
    }
}

bool CRtpReceiver::set_media_port(unsigned short port)
{
    if (0 > port)
    {
        return false;
    }
    else
    {
        m_media_port = port;
    }
}

bool CRtpReceiver::set_client_id(char* id)
{
    if (strlen(id) != 20)
    {
        return false;
    }
    else
    {
        sprintf(m_client_id, "%s", id);
        return true;
    }
}

void CRtpReceiver::setup_callback_function(
    callback_get_ps_stream_fp get_ps_stream,
    callback_get_h264_stream_fp get_h264_stream,
    callback_get_mpeg4_stream_fp get_mpeg4_stream,
    callback_get_svac_stream_fp get_svac_stream)
{
    m_callback_get_ps_stream = get_ps_stream;
    m_callback_get_h264_stream = get_h264_stream;
    m_callback_get_mpeg4_stream = get_mpeg4_stream;
    m_callback_get_svac_stream = get_svac_stream;
}