#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "Demuxer2.h"
#include "utils/logger.h"

#ifndef WIN32

#endif

namespace bsm {
namespace bsm_video_decoder {

callback_pull_ps_stream_demuxer2 bsm_demuxer2::m_callback_pull_ps_stream = NULL;
callback_push_es_video_stream_demuxer2 bsm_demuxer2::m_callback_push_es_video_stream = NULL;
callback_push_es_audio_stream_demuxer2 bsm_demuxer2::m_callback_push_es_audio_stream = NULL;

bool bsm_demuxer2::find_next_hx_str(unsigned char* source, int source_length, unsigned char* seed, int seed_length, int* position)
{
    if (!source || !seed)
    {
        return false;
    }

    unsigned char* pHeader = source;
    int total_length = source_length;
    int processed_length = 0;

    while (total_length - processed_length >= seed_length)
    {
        for (int i = 0; i < seed_length && (pHeader[i] == seed[i]); i++)
        {
            if (seed_length - 1 == i)
            {
                *position = processed_length;
                return true;
            }
        }

        processed_length++;
        pHeader = source + processed_length;
    }

    return false;
}

bool bsm_demuxer2::find_next_ps_packet(unsigned char* source, int source_length, int* ps_packet_start_point, int* ps_packet_length)
{
    if (!source)
    {
        return false;
    }

    int _ps_packet_start_point = 0;
    int _ps_packet_end_point = 0;

    unsigned char ps_packet_start_code[4];
    ps_packet_start_code[0] = 0x00;
    ps_packet_start_code[1] = 0x00;
    ps_packet_start_code[2] = 0x01;
    ps_packet_start_code[3] = 0xba;

    if (find_next_hx_str(source, source_length, ps_packet_start_code, 4, &_ps_packet_start_point))
    {
        if (find_next_hx_str(source + 4, source_length - 4, ps_packet_start_code, 4, &_ps_packet_end_point))
        {
            *ps_packet_start_point = _ps_packet_start_point;
            *ps_packet_length = (_ps_packet_end_point - _ps_packet_start_point)+4;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
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
        //we can get padding data length , from the last 3 bit in fourteenth(14) byte in header.
        ps_packet_header_stuffed_size = packet[13] & 0x07;

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

                // the reason of +6, is there 6 byte data before padding data in header.
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

                // the reason of +6, is there 6 byte data before padding data in header.
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

                packet_processed_length += 6 + pes_video_h264_packet_size;
                if (packet_total_length < packet_processed_length)
                {
                    LOG("error: packet_total_length < packet_processed_length.\n");
                    break;
                }

                // the reason of +9 is , before pes_video_h264_packet_stuffed_size filed, there are 9 bit in header.
                // the reason of +6 is , all pes packet length is get from the sixth bit in header.
                if (m_callback_push_es_video_stream)
                {
                    m_callback_push_es_video_stream(NULL, 
                        next_pes_packet + 9 + pes_video_h264_packet_stuffed_size,
                        pes_video_h264_packet_size + 6 - 9 - pes_video_h264_packet_stuffed_size);
                }

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
                LOG("error, please check if src ps packet formate is right\n");
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
            // must open file as binary model, otherwise, on windows it will replease '\n' to '\r\n'.
            pf_media_file = fopen(file_name, "ab+");
        }

        if (pf_media_file != NULL)
        {
            fwrite(pLog, nLen, 1, pf_media_file);
            fflush(pf_media_file);
            fclose(pf_media_file);
            pf_media_file = NULL;
        }
    }
}

int bsm_demuxer2::demux_ps_to_es_file(char* ps_file_name)
{
    int buffer_capacity = MAX_BUFFER_SIZE;      //buffer total size
    int buffer_size = 0;                        //buffer current size
    int buffer_left_size = MAX_BUFFER_SIZE;     
    int processed_size = 0;                     

    int _ps_packet_start_position = 0;
    int _ps_packet_length = 0;

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

    //errno_t err;
    FILE* pf_ps_file;

    pf_ps_file = fopen(ps_file_name, "rb");
    if (pf_ps_file)
    {
        printf("The file '%s' was opened\n", ps_file_name);
    }
    else
    {
        printf("failure, when open The file '%s'.\n", ps_file_name);
        return -1;
    }

    do {
        if (find_next_ps_packet(stream_data_buf + processed_size, buffer_capacity - processed_size,
            &_ps_packet_start_position, &_ps_packet_length))
        {
            if (_ps_packet_length != deal_ps_packet(stream_data_buf + _ps_packet_start_position + processed_size, _ps_packet_length))
            {
                LOG("please check if ps packe is right.\n");
            }

            processed_size += _ps_packet_length;
            buffer_size = buffer_capacity - processed_size;
        }
        else
        {
            if (is_end_of_file)
            {
                //deal last PS packet in buffer.
                deal_ps_packet(stream_data_buf + processed_size, buffer_capacity - processed_size);
                break;
            }
            
            if (buffer_size < buffer_capacity)
            {
                if(0 < processed_size)
                { 
                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memcpy(tmp_data_buf, stream_data_buf + processed_size, buffer_capacity - processed_size);

                    memset(stream_data_buf, 0x00, buffer_capacity);
                    memcpy(stream_data_buf, tmp_data_buf, buffer_capacity - processed_size);

                    buffer_left_size = processed_size;
                    processed_size = 0;
                }

                //read data from file to fill buffer.
                if(buffer_left_size > fread(stream_data_buf + (buffer_capacity - buffer_left_size), 1, buffer_left_size, pf_ps_file))
                {
                    LOG("end of file.\n");
                    is_end_of_file = true;
                }
                else
                {
                    buffer_size = buffer_capacity;
                }
                continue;
            }
            else if (buffer_size == buffer_capacity)
            {
                if (0 < _ps_packet_start_position)
                {
                    LOG("have find first PS packet.");
                    if (0 < processed_size)
                    {
                        LOG("error : 0 < processed_size && 0 < _ps_packet_start_position");
                    }
                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memcpy(tmp_data_buf, stream_data_buf + _ps_packet_start_position, buffer_capacity - _ps_packet_start_position);

                    memset(stream_data_buf, 0x00, buffer_capacity);
                    memcpy(stream_data_buf, tmp_data_buf, buffer_capacity - _ps_packet_start_position);

                    buffer_size = buffer_capacity - _ps_packet_start_position;
                    buffer_left_size = _ps_packet_start_position;
                    processed_size = 0;
                }
                else
                { 
                    LOG("buffer is too small.\n");

                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memset(stream_data_buf, 0x00, buffer_capacity);
                    buffer_size = 0;
                    buffer_left_size = buffer_capacity;
                    processed_size = 0;
                }
            }
            else
            {
                LOG("error : buffer_size > buffer_capacity");
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
        if (0 == fclose(pf_ps_file))
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
    int buffer_capacity = MAX_BUFFER_SIZE;      
    int buffer_size = 0;                        
    int processed_size = 0;                     
    int buffer_left_size = MAX_BUFFER_SIZE;     

    int _ps_packet_start_position = 0;
    int _ps_packet_length = 0;

    int real_process_ps_packet_size;

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

    do {
        if (find_next_ps_packet(stream_data_buf + processed_size, buffer_capacity - processed_size,
            &_ps_packet_start_position, &_ps_packet_length))
        {
            if (_ps_packet_length != deal_ps_packet(stream_data_buf + _ps_packet_start_position + processed_size, _ps_packet_length))
            {
                LOG("please check if ps packe is right.\n");
            }

            processed_size += _ps_packet_length;
            buffer_size = buffer_capacity - processed_size;
        }
        else
        {
            if (buffer_size < buffer_capacity)
            {
                if (0 < processed_size)
                {
                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memcpy(tmp_data_buf, stream_data_buf + processed_size, buffer_capacity - processed_size);

                    memset(stream_data_buf, 0x00, buffer_capacity);
                    memcpy(stream_data_buf, tmp_data_buf, buffer_capacity - processed_size);

                    buffer_left_size = processed_size;
                    processed_size = 0;
                }

                //read data from network, to fill buffer.
                if (buffer_left_size == m_callback_pull_ps_stream(NULL, stream_data_buf + (MAX_BUFFER_SIZE - buffer_left_size), buffer_left_size))
                {
                    buffer_size = buffer_capacity;
                }

                continue;
            }
            else if (buffer_size == buffer_capacity)
            {
                if (0 < _ps_packet_start_position)
                {
                    LOG("have find first PS packet.");

                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memcpy(tmp_data_buf, stream_data_buf + _ps_packet_start_position, buffer_capacity - _ps_packet_start_position);

                    memset(stream_data_buf, 0x00, buffer_capacity);
                    memcpy(stream_data_buf, tmp_data_buf, buffer_capacity - _ps_packet_start_position);

                    buffer_size = buffer_capacity - _ps_packet_start_position;
                    buffer_left_size = _ps_packet_start_position;
                    processed_size = 0;
                }
                else
                {
                    LOG("buffer is too small.\n");

                    memset(tmp_data_buf, 0x00, buffer_capacity);
                    memset(stream_data_buf, 0x00, buffer_capacity);
                    buffer_size = 0;
                    buffer_left_size = buffer_capacity;
                    processed_size = 0;
                }
            }
            else
            {
                LOG("error : buffer_size > buffer_capacity");
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
