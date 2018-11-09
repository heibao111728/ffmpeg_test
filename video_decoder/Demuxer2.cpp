#include "Demuxer2.h"
#include <fstream>

namespace bsm {
namespace bsm_video_decoder {

callback_pull_ps_stream_demuxer2 bsm_demuxer2::m_callback_pull_ps_stream = NULL;
callback_push_es_video_stream_demuxer2 bsm_demuxer2::m_callback_push_es_video_stream = NULL;
callback_push_es_audio_stream_demuxer2 bsm_demuxer2::m_callback_push_es_audio_stream = NULL;

int bsm_demuxer2::find_next_hx_str(unsigned char* source, int source_length, unsigned char* seed, int seed_length)
{
    if (source && seed)
    {
    }
    else
    {
        //failure
        return 0;
    }

    unsigned char* pHeader = source + seed_length;

    int total_length = source_length;
    int processed_length = 0;

    int src_offset = 0;
    while (total_length - processed_length >= seed_length)
    {
        for (int i = 0; i < seed_length && (pHeader[i] == seed[i]); i++)
        {
            if (seed_length - 1 == i)
            {
                //find ok
                return seed_length + processed_length;
            }
        }

        processed_length++;
        pHeader = source + seed_length + processed_length;
    }

    return 0;
}


int bsm_demuxer2::deal_ps_packet(unsigned char * packet, int length)
{
    int ps_packet_header_stuffed_size;
    int pes_system_header_header_length;
    int pes_program_stream_map_length;
    int pes_video_h264_packet_size;
    int pes_video_h264_packet_stuffed_size;

    int packet_total_length = length;
    int packet_processed_length = 0;

    ps_packet_header* ps_head = NULL;
    pes_system_header_packet_header_t* pes_system_header;
    pes_program_stream_map_packet_header_t* pes_psm_header;
    pes_media_packet_header_t* pes_video_h264_packet_header;
    pes_media_packet_header_t* pes_audio_packet_header;

    unsigned char* next_pes_packet = NULL;

    littel_endian_size_u tmp_size;

    // deal with ps packet header.
    if (packet[0] == 0x00
        && packet[1] == 0x00
        && packet[2] == 0x01
        && packet[3] == 0xba)
    {
        //��ps��ͷ��14���ֽڵ����3λ��ȡͷ��������ݵĳ���
        ps_packet_header_stuffed_size = packet[13] & 0x07;

        //+14��ԭ���Ǳ�ʾ������ݵĳ���λ��PS��ͷ���ĵ�14���ֽڵĺ�3λ˵����
        //��ʹ��sizeof����PS��ͷ�����ȵ�ԭ���ǽṹ���ڲ��ᷢ���Զ����룬���¸ý����׼ȷ��
        packet_processed_length += 14 + ps_packet_header_stuffed_size;

        next_pes_packet = packet + packet_processed_length;

        while (next_pes_packet && (packet_total_length - packet_processed_length >= 0))
        {
            if (next_pes_packet
                && next_pes_packet[0] == 0x00
                && next_pes_packet[1] == 0x00
                && next_pes_packet[2] == 0x01
                && next_pes_packet[3] == 0xbb)
            {
                //this pes packet is system_header or psm, which data is not usefull for es_h264
                pes_system_header = (pes_system_header_packet_header_t*)next_pes_packet;

                littel_endian_size_u tmp_size;
                tmp_size.byte[0] = pes_system_header->header_length.byte[1];
                tmp_size.byte[1] = pes_system_header->header_length.byte[0];

                pes_system_header_header_length = tmp_size.length;

                // +6��ԭ����pes_packet_header_t�г����ֽ�֮ǰ����6���ֽ�
                packet_processed_length += (6 + pes_system_header_header_length);

                next_pes_packet = packet + packet_processed_length;
            }
            else if (next_pes_packet
                && next_pes_packet[0] == 0x00
                && next_pes_packet[1] == 0x00
                && next_pes_packet[2] == 0x01
                && next_pes_packet[3] == 0xbc)
            {
                //this pes packet is program stream map, which data is not usefull for es_h264
                pes_psm_header = (pes_program_stream_map_packet_header_t*)next_pes_packet;


                tmp_size.byte[0] = pes_psm_header->program_stream_map_length.byte[1];
                tmp_size.byte[1] = pes_psm_header->program_stream_map_length.byte[0];

                pes_program_stream_map_length = tmp_size.length;

                // +6��ԭ����pes_packet_header_t���Զ��������֮ǰ��6���ֽ�
                packet_processed_length += 6 + pes_program_stream_map_length;

                next_pes_packet = packet + packet_processed_length;
            }

            else if (next_pes_packet
                && next_pes_packet[0] == 0x00
                && next_pes_packet[1] == 0x00
                && next_pes_packet[2] == 0x01
                && next_pes_packet[3] == 0xe0)
            {
                //contain video es stream
                pes_video_h264_packet_header = (pes_media_packet_header_t*)next_pes_packet;

                //littel_endian_size_u tmp_size;
                tmp_size.byte[0] = pes_video_h264_packet_header->packet_size.byte[1];
                tmp_size.byte[1] = pes_video_h264_packet_header->packet_size.byte[0];

                pes_video_h264_packet_size = tmp_size.length;
                pes_video_h264_packet_stuffed_size = pes_video_h264_packet_header->PES_header_data_length;

                // +9 ��ԭ����pes_video_h264_packet_stuffed_size֮ǰ����9���ֽڵ�ͷ������
                // +6 ��ԭ����pes�����ܳ�������ͷ��֮���6���ֽڴ��õ��ġ�
                //write_media_data_to_file(m_output_es_video_file_name,
                    //next_pes_packet + 9 + pes_video_h264_packet_stuffed_size,
                    //pes_video_h264_packet_size + 6 - 9 - pes_video_h264_packet_stuffed_size);
                if (m_callback_push_es_video_stream)
                {
                    m_callback_push_es_video_stream(NULL, 
                        next_pes_packet + 9 + pes_video_h264_packet_stuffed_size,
                        pes_video_h264_packet_size + 6 - 9 - pes_video_h264_packet_stuffed_size);
                }

                packet_processed_length += 6 + pes_video_h264_packet_size;
                next_pes_packet = packet + packet_processed_length;
            }

            else if (next_pes_packet
                && next_pes_packet[0] == 0x00
                && next_pes_packet[1] == 0x00
                && next_pes_packet[2] == 0x01
                && next_pes_packet[3] == 0xc0)
            {
                //contain audio es stream
                pes_audio_packet_header = (pes_media_packet_header_t*)next_pes_packet;
            }
            else
            {
                // bad data
                break;
            }
            if (packet_total_length - packet_processed_length < 0)
            {
                LOG("error, please check if src ps file formate is right\n");
                return packet_total_length;
            }
        }

    }
    else
    {
        return 0;
    }

    return packet_processed_length;
}

void bsm_demuxer2::write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
    FILE* pf_media_file = NULL;
    if (pLog != NULL && nLen > 0)
    {
        if (NULL == pf_media_file && strlen(file_name) > 0)
        {
            //һ��Ҫ�Զ����Ʒ�ʽ���ļ�����Ȼ��Windowsƽ̨����ļ�ʱ�Ὣ�����С�ת���ɡ��س�+���С��������ļ�����
            ::fopen_s(&pf_media_file, file_name, "ab+");
        }

        if (pf_media_file != NULL)
        {
            ::fwrite(pLog, nLen, 1, pf_media_file);
            ::fflush(pf_media_file);
            ::fclose(pf_media_file);
            pf_media_file = NULL;
        }
    }
}

int bsm_demuxer2::demux_ps_to_es_file(char* ps_file_name)
{
    int buffer_capacity = MAX_BUFFER_SIZE;      //buffer total size
    int buffer_size = 0;                        //buffer current size
    int processed_size = 0;                     //�Ѿ�������Ļ������ݴ�С
    int buffer_left_size = MAX_BUFFER_SIZE;     //������ʣ���С
    int read_size = 0;
    int next_ps_packet_offset = 0;              //��������һ��ps����λ��

    int ps_packet_length = 0;                   //ps������

    bool is_end_of_file = false;

    unsigned char* stream_data_buf = NULL;
    unsigned char* tmp_data_buf = NULL;

    stream_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);

    tmp_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);

    unsigned char ps_packet_start_code[4];
    ps_packet_start_code[0] = 0x00;
    ps_packet_start_code[1] = 0x00;
    ps_packet_start_code[2] = 0x01;
    ps_packet_start_code[3] = 0xba;

    errno_t err;
    FILE* pf_ps_file;

    err = ::fopen_s(&pf_ps_file, ps_file_name, "rb");
    if (err == 0)
    {
        printf("The file '%s' was opened\n", ps_file_name);
    }
    else
    {
        printf("The file '%s' was not opened\n", ps_file_name);
        return -1;
    }

    do {
        ps_packet_length = find_next_hx_str(stream_data_buf + next_ps_packet_offset,
            MAX_BUFFER_SIZE - next_ps_packet_offset,
            ps_packet_start_code, 4);

        if (0 != ps_packet_length)
        {
            //����PS���ɹ�, ��ʼ����
            processed_size += deal_ps_packet(stream_data_buf + next_ps_packet_offset, ps_packet_length);

            next_ps_packet_offset += ps_packet_length;
            //buffer_left_size = processed_size;
        }
        else
        {
            //����ʧ��
            if (0 == processed_size && (buffer_size == buffer_capacity))
            {
                //������̫С
                LOG("buffer is too small.\n");
                return -1;
            }
            else 
            {
                //�������㹻����������ʣ������ݲ���һ����PS packet�� ��Ҫ���¶�ȡ�ļ�
                if (is_end_of_file)
                {
                    //������󻺴��е�����, �������������ʧ���һ��PS������
                    deal_ps_packet(stream_data_buf + processed_size, MAX_BUFFER_SIZE - processed_size);
                    break;
                }

                //����ʧ�ܣ����ļ�δ���꣬��������ļ�
                //��һ������������ʣ�������ƶ���������ǰ�ˣ�
                if(0<processed_size)
                { 
                    memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);
                    memcpy(tmp_data_buf, stream_data_buf + processed_size, MAX_BUFFER_SIZE - processed_size);

                    memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);
                    memcpy(stream_data_buf, tmp_data_buf, MAX_BUFFER_SIZE - processed_size);

                    next_ps_packet_offset = 0;
                    buffer_left_size += processed_size;
                    processed_size = 0;
                }

                //�ڶ�������ȡ�ļ����ݽ�������������
                read_size = ::fread_s(stream_data_buf + (MAX_BUFFER_SIZE - buffer_left_size), buffer_left_size, 1, buffer_left_size, pf_ps_file);
                buffer_size = read_size;

                buffer_left_size -= read_size;
                if (buffer_left_size > 0)
                {
                    LOG("end of file.\n");
                    is_end_of_file = true;
                    continue;
                }
            }
        }
    } while (true);

    //release memory
    if (NULL != stream_data_buf)
    {
        free(stream_data_buf);
    }

    if (NULL != tmp_data_buf)
    {
        free(tmp_data_buf);
    }

    if (pf_ps_file)
    {
        err = fclose(pf_ps_file);
        if (err == 0)
        {
            printf("The file '%s' was closed\n", ps_file_name);
        }
        else
        {
            printf("The file '%s' was not closed\n", ps_file_name);
        }
    }

    return 0;
}

int bsm_demuxer2::demux_ps_to_es_network()
{
    int buffer_capacity = MAX_BUFFER_SIZE;      //buffer ������
    //int buffer_size = 0;                        //buffer �е�ǰ���ݴ�С
    int processed_size = 0;                     //�Ѿ�������Ļ������ݴ�С
    int buffer_left_size = MAX_BUFFER_SIZE;     //������ʣ���С
    int read_size = 0;
    int next_ps_packet_offset = 0;              //��������һ��ps����λ��

    int ps_packet_length = 0;                   //ps������

    bool is_end_of_file = false;

    unsigned char* stream_data_buf = NULL;
    unsigned char* tmp_data_buf = NULL;

    stream_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);

    tmp_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);

    unsigned char ps_packet_start_code[4];
    ps_packet_start_code[0] = 0x00;
    ps_packet_start_code[1] = 0x00;
    ps_packet_start_code[2] = 0x01;
    ps_packet_start_code[3] = 0xba;

    //buffer_left_size = MAX_BUFFER_SIZE;

    do {
        ps_packet_length = find_next_hx_str(stream_data_buf + next_ps_packet_offset,
            MAX_BUFFER_SIZE - next_ps_packet_offset,
            ps_packet_start_code, 4);

        if (0 != ps_packet_length)
        {
            //����PS���ɹ�, ��ʼ����
            processed_size += deal_ps_packet(stream_data_buf + next_ps_packet_offset, ps_packet_length);

            next_ps_packet_offset += ps_packet_length;
        }
        else
        {
            //����ʧ��
            if (0 == processed_size && 0 == buffer_left_size)
            {
                LOG("error: Two situations can cause this problem to occur:\n\
                            1. demuxer2 buffer is too small, need resize it bigger.\n\
                            2. data in buffer is error, now, we will abandon this data to ensure demuxer run gracefully.(default solution) ");
                return -1;
                //memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);
                //memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);

                //next_ps_packet_offset = 0;
                //buffer_left_size = MAX_BUFFER_SIZE;
                //processed_size = 0;
                //continue;
            }
            else
            {
                //����ʧ��
                //��һ������������ʣ�������ƶ���������ǰ�ˣ�
                //step 1:move left data to buffer header.
                if(0 < processed_size)
                { 
                    memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);
                    memcpy(tmp_data_buf, stream_data_buf + processed_size, MAX_BUFFER_SIZE - processed_size);

                    memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);
                    memcpy(stream_data_buf, tmp_data_buf, MAX_BUFFER_SIZE - processed_size);

                    next_ps_packet_offset = 0;
                    buffer_left_size = processed_size;
                    processed_size = 0;
                }

                //�ڶ��������ûص�������demux2������������
                //step2: call callback function to fill full buffer.
                read_size = m_callback_pull_ps_stream(NULL, stream_data_buf + (MAX_BUFFER_SIZE - buffer_left_size), buffer_left_size);

                buffer_left_size -= read_size;
                if (buffer_left_size > 0)
                {
                    //LOG("outer buffer do'nt have enought data, need wait a moment.\n");
                    continue;
                }
            }
        }
    } while (true);

    //release memory
    if (NULL != stream_data_buf)
    {
        free(stream_data_buf);
    }

    if (NULL != tmp_data_buf)
    {
        free(tmp_data_buf);
    }

    return 0;
}

void bsm_demuxer2::setup_callback_function(callback_pull_ps_stream_demuxer2 pull_ps_stream,
    callback_push_es_video_stream_demuxer2 push_es_video_stream,
    callback_push_es_audio_stream_demuxer2 push_es_audio_stream)
{
    m_callback_pull_ps_stream = pull_ps_stream;
    m_callback_push_es_video_stream = push_es_video_stream;
    m_callback_push_es_audio_stream = push_es_audio_stream;
}

}//namespace bsm_video_decoder
}//namespace bsm