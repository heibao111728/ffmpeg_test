#include "Demuxer2.h"
#include <fstream>

int CDemuxer2::find_next_hx_str(unsigned char* source, int source_length, unsigned char* seed, int seed_length)
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


int CDemuxer2::deal_ps_packet(unsigned char * packet, int length)
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
        ps_head = (ps_packet_header_t*)packet;

        ps_packet_header_stuffed_size = ps_head->pack_stuffing_length & 0x07;

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
                write_media_data_to_file(dst_es_video_filename,
                    next_pes_packet + 9 + pes_video_h264_packet_stuffed_size,
                    pes_video_h264_packet_size + 6 - 9 - pes_video_h264_packet_stuffed_size);

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
        }

    }
    else
    {
        return 0;
    }

    return packet_processed_length;
}

void CDemuxer2::write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
    FILE* m_pLogFile = NULL;
    if (pLog != NULL && nLen > 0)
    {
        if (NULL == m_pLogFile && strlen(file_name) > 0)
        {
            //һ��Ҫ�Զ����Ʒ�ʽ���ļ�����Ȼ��Windowsƽ̨����ļ�ʱ�Ὣ�����С�ת���ɡ��س�+���С��������ļ�����
            ::fopen_s(&m_pLogFile, file_name, "ab+");
        }

        if (m_pLogFile != NULL)
        {
            ::fwrite(pLog, nLen, 1, m_pLogFile);
            ::fflush(m_pLogFile);
            ::fclose(m_pLogFile);
            m_pLogFile = NULL;
        }
    }
}

int CDemuxer2::do_demux()
{
    int buffer_size = MAX_BUFFER_SIZE;
    int processed_size = 0;             //�Ѿ�������Ļ������ݴ�С
    int read_buffer_left_size = 0;          //������ʣ���С

    unsigned char* stream_data_buf = NULL;
    unsigned char* tmp_data_buf = NULL;

    stream_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);

    tmp_data_buf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);

    int read_size = 0;

    unsigned char ps_packet_start_code[4];
    ps_packet_start_code[0] = 0x00;
    ps_packet_start_code[1] = 0x00;
    ps_packet_start_code[2] = 0x01;
    ps_packet_start_code[3] = 0xba;

    int ps_packet_length = 0;
    int return_value = -1;

    int next_ps_packet_offset = 0;

    read_buffer_left_size = MAX_BUFFER_SIZE;

    bool is_end_of_file = false;

    do {
        ps_packet_length = find_next_hx_str(stream_data_buf + next_ps_packet_offset,
            MAX_BUFFER_SIZE - next_ps_packet_offset,
            ps_packet_start_code, 4);

        if (0 != ps_packet_length)
        {
            //����PS���ɹ�, ��ʼ����
            processed_size += deal_ps_packet(stream_data_buf + next_ps_packet_offset, ps_packet_length);

            next_ps_packet_offset += ps_packet_length;
            read_buffer_left_size = processed_size;
        }
        else
        {
            //����ʧ�ܣ��ļ��Ѿ�����
            if (is_end_of_file)
            {
                //������󻺴��е�����, �������������ʧ���һ��PS������
                deal_ps_packet(stream_data_buf + processed_size, MAX_BUFFER_SIZE - processed_size);
                break;
            }

            //����ʧ�ܣ����ļ�δ���꣬��������ļ�
            //��һ������������ʣ�������ƶ���������ǰ�ˣ�
            memset(tmp_data_buf, 0x00, MAX_BUFFER_SIZE);
            memcpy(tmp_data_buf, stream_data_buf + processed_size, MAX_BUFFER_SIZE - processed_size);

            memset(stream_data_buf, 0x00, MAX_BUFFER_SIZE);
            memcpy(stream_data_buf, tmp_data_buf, MAX_BUFFER_SIZE - processed_size);

            processed_size = 0;
            next_ps_packet_offset = 0;

            //�ڶ�������ȡ�ļ����ݽ�������������
            read_size = ::fread_s(stream_data_buf + (MAX_BUFFER_SIZE - read_buffer_left_size), read_buffer_left_size, 1, read_buffer_left_size, m_pf_ps_file);
            if (read_buffer_left_size > read_size)
            {
                is_end_of_file = true;
                continue;
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

void CDemuxer2::setup_src_ps_file(char* filename)
{
    memset(src_ps_filename, 0x00, MAX_FILENAME_LENGTH);
    if (strlen(filename) > 0)
    {
        sprintf_s(src_ps_filename, MAX_FILENAME_LENGTH, "%s", filename);
    }
}

void CDemuxer2::setup_dst_es_video_file(char* filename)
{
    memset(dst_es_video_filename, 0x00, MAX_FILENAME_LENGTH);
    if (strlen(filename) > 0)
    {
        sprintf_s(dst_es_video_filename, MAX_FILENAME_LENGTH, "%s", filename);
    }
}

void CDemuxer2::setup_dst_es_audio_file(char* filename)
{
    memset(dst_es_audio_filename, 0x00, MAX_FILENAME_LENGTH);
    if (strlen(filename) > 0)
    {
        sprintf_s(dst_es_audio_filename, MAX_FILENAME_LENGTH, "%s", filename);
    }
}

bool CDemuxer2::open_src_ps_file()
{
    //open ps file
    errno_t err;
    err = ::fopen_s(&m_pf_ps_file, src_ps_filename, "rb");
    if (err == 0)
    {
        printf("The file '%s' was opened\n", src_ps_filename);
    }
    else
    {
        printf("The file '%s' was not opened\n", src_ps_filename);
    }
    return (0 == err);
}
bool CDemuxer2::close_src_ps_file()
{
    // close ps file 
    errno_t err = 0;
    if (m_pf_ps_file)
    {
        err = fclose(m_pf_ps_file);
        if (err == 0)
        {
            printf("The file '%s' was closed\n", src_ps_filename);
        }
        else
        {
            printf("The file '%s' was not closed\n", src_ps_filename);
        }
    }
    return (0 == err);
}