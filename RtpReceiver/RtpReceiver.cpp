#include "RtpReceiver.h"
#include <process.h>
//#include "DataRepository\include\DataRepository.h"

extern char g_ClientIp[20]; //sip UA IP
extern char g_ClientId[20]; //sip UA Id

////���PS�������ݲֿ⣬ÿһ������洢һ֡������PS��
//extern CDataRepository<unsigned char*> g_PsPacketRepo;
//
////���ES�������ݲֿ⣬ÿһ������洢һ֡������ES��
//extern CDataRepository<unsigned char*> g_EsPacketRepo;

CRtpReceiver::CRtpReceiver(unsigned short rtpPort)
    :m_mediaPort(rtpPort)
{
    m_offset = 0;
    //m_ps_demuxer.setup_dst_es_video_file("E:\\buf_es.h264");
}

CRtpReceiver::~CRtpReceiver()
{
}

char* CRtpReceiver::getFrame()
{
    return nullptr;
}

char* CRtpReceiver::getSdpInfo()
{
    return m_SdpInfo;
}

int CRtpReceiver::generateSdpInfo()
{
    char pMediaPort[10] = { 0 };
    _itoa_s(m_mediaPort, pMediaPort, 10);

    char pSsrc[50] = { 0 };
    sprintf_s(pSsrc, "%s", "999999");

    sprintf_s(m_SdpInfo, 4 * 1024,
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
        g_ClientId,
        g_ClientIp,
        g_ClientIp,
        pMediaPort,
        pSsrc);

    return 0;
}

uint16_t CRtpReceiver::getMediaPort()
{
    return m_mediaPort;
}

int CRtpReceiver::StartProc()
{
    m_threadHandle = (HANDLE)_beginthread(ThreadProc, 0, (void*)this);
    if (0 == m_threadHandle)
    {
        //�߳�����ʧ��
        return -1;
    }
    m_bThreadRuning = true;

    //����sdp��Ϣ
    generateSdpInfo();

    return 0;
}

void CRtpReceiver::StopProc()
{
    m_bThreadRuning = false;

    m_RtpSession.BYEDestroy(RTPTime(10, 0), 0, 0);
}

void CRtpReceiver::ThreadProc(void* pParam)
{
    CRtpReceiver* pThis = (CRtpReceiver*)pParam;

    (pThis->m_Sessparams).SetOwnTimestampUnit(1.0 / 8000.0);
    (pThis->m_Sessparams).SetAcceptOwnPackets(true);
    (pThis->m_Transparams).SetPortbase(pThis->m_mediaPort);

    int status, i, num;

    status = (pThis->m_RtpSession).Create((pThis->m_Sessparams), &(pThis->m_Transparams));

    while (pThis->m_bThreadRuning)
    {
        (pThis->m_RtpSession).BeginDataAccess();

        // check incoming packets
        if ((pThis->m_RtpSession).GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *pack;

                while ((pack = (pThis->m_RtpSession).GetNextPacket()) != NULL)
                {
                    //pThis->assemleFrame(pack);
                    pThis->handlePacket(pack);
                    // we don't longer need the packet, so
                    // we'll delete it
                    (pThis->m_RtpSession).DeletePacket(pack);
                }
            } while ((pThis->m_RtpSession).GotoNextSourceWithData());
        }
        (pThis->m_RtpSession).EndDataAccess();
        RTPTime::Wait(RTPTime(1, 0));
    }
}

int CRtpReceiver::handlePacket(RTPPacket* packet)
{
    int packetSize = 0;
    uint8_t packetPayloadType;

    if (NULL == packet)
    {
        return 0;
    }

    packetSize = packet->GetPacketLength();
    packetPayloadType = packet->GetPayloadType();

    //���������ʹ���������
    switch (packetPayloadType)
    {
    case PS: //96
    {
        handlePsPacket( packet );
        break;
    }
    case MPEG4: //97
    {
        break;
    }
    case H264: //98
    {
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


int CRtpReceiver::handlePsPacket(RTPPacket* packet)
{
    

    if (NULL == packet)
    {
        return 0;
    }

    if (packet->HasMarker())   //�����ݰ�, asMarker() Returns true is the marker bit was set
    {
        //���յ�������һ֡��������Ƶ֡���У��������������������ռ��ͷţ�����һ֡���ݴ�š�
        memcpy(m_pFrame + m_offset, packet->GetPayloadData(), packet->GetPayloadLength());
        m_frameSize = m_offset + packet->GetPayloadLength();
        m_pTmpFrame = new uint8_t(m_frameSize);

        //g_PsPacketRepo.putData(m_pTmpFrame);    //��PS���ֿ�
        //memcpy(m_pTmpFrame, m_pFrame, m_frameSize);

        write_media_data_to_file("E://buf_mediaplay.ps", m_pFrame, m_frameSize);
        //write_media_data_to_file("E://src_mediaplay.ps", packet->GetPayloadData(), packet->GetPayloadLength());

        //deal_ps_packet(m_pFrame, m_frameSize);
        //m_ps_demuxer.setup_dst_es_video_file("E://buf_mediaplay.es");
        //m_ps_demuxer.deal_ps_packet(m_pFrame, m_frameSize);

        m_frameSize = 0;
        m_offset = 0;
    }
    else
    {
        memcpy(m_pFrame + m_offset, packet->GetPayloadData(), packet->GetPayloadLength());
        m_offset += packet->GetPayloadLength();
        //write_media_data_to_file("E://src_mediaplay.ps", packet->GetPayloadData(), packet->GetPayloadLength());
    }
    return packet->GetPayloadLength();
}

int CRtpReceiver::handleMPEG4Packet(RTPPacket* packet)
{
    return 0;
}

int CRtpReceiver::handleH264Packet(RTPPacket* packet)
{
    return 0;
}

void CRtpReceiver::write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
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