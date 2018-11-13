/*****************************************************************************
bsm - bstorm distributed streaming analysing system
Copyright (c) 2018 boostiv , Inc.
File discription:This file can only be used under the boostiv authorilization.
Author:	    yangdong
Datetime:   2018/11/13
*****************************************************************************/

#ifndef __DEMUXER_H__
#define __DEMUXER_H__

namespace bsm{
namespace bsm_video_decoder{

#define MAX_FILE_NAME_LENGTH___BSM_DEMUXER 60
#define RTP_V4_RECEIVE_BUFFER	32768

typedef int(*callback_pull_ps_stream_demuxer)(void *opaque, unsigned char *buf, int buf_size);         //input ps stream
typedef int(*callback_push_es_video_stream_demuxer)(void *opaque, unsigned char *buf, int buf_size);   //output video es stream
typedef int(*callback_push_es_audio_stream_demuxer)(void *opaque, unsigned char *buf, int buf_size);   //output audio es stream

/**
*   demux ps stream to es stream, using ffmpeg.
*/

class bsm_demuxer
{
public:
    bsm_demuxer() {}
    ~bsm_demuxer() {}

    void set_output_es_video_file(char* file_name);
    void set_output_es_audio_file(char* file_name);

    bool demux_ps_to_es_file(char* ps_file_name);
    bool demux_ps_to_es_network();

    bool demux_ps_file_to_es_stream(char* ps_file_name);

public:
    static void setup_callback_function(callback_pull_ps_stream_demuxer pull_ps_stream,
        callback_push_es_video_stream_demuxer push_es_video_stream,
        callback_push_es_audio_stream_demuxer push_es_audio_stream);

    static callback_pull_ps_stream_demuxer m_callback_pull_ps_stream;
    static callback_push_es_video_stream_demuxer m_callback_push_es_video_stream;
    static callback_push_es_audio_stream_demuxer m_callback_push_es_audio_stream;

private:
    char m_output_es_video_file_name[MAX_FILE_NAME_LENGTH___BSM_DEMUXER];
    char m_output_es_audio_file_name[MAX_FILE_NAME_LENGTH___BSM_DEMUXER];
};

} // namespace bsm_video_decoder
} // namespace bsm
#endif // !__DEMUXER_H__

