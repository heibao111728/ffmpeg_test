#include "Demuxer.h"

void CDemuxer::set_input_ps_file(char* file_name)
{
    memset(m_input_ps_file_name, 0x00, MAX_FILE_NAME_LENGTH);
    if (strlen(file_name) > 0)
    {
        sprintf_s(m_input_ps_file_name, MAX_FILE_NAME_LENGTH, "%s", file_name);
    }
}
void CDemuxer::set_output_es_video_file(char* file_name)
{
    memset(m_output_es_video_file_name, 0x00, MAX_FILE_NAME_LENGTH);
    if (strlen(file_name) > 0)
    {
        sprintf_s(m_output_es_video_file_name, MAX_FILE_NAME_LENGTH, "%s", file_name);
    }
}

void CDemuxer::set_output_es_audio_file(char* file_name)
{
    memset(m_output_es_audio_file_name, 0x00, MAX_FILE_NAME_LENGTH);
    if (strlen(file_name) > 0)
    {
        sprintf_s(m_output_es_audio_file_name, MAX_FILE_NAME_LENGTH, "%s", file_name);
    }
}

bool CDemuxer::demux_ps_to_es()
{
    AVInputFormat * av_input_formate = NULL;
    AVOutputFormat *av_output_formate = NULL;

    AVFormatContext *av_formate_context_input;
    AVFormatContext *av_formate_context_out_video = NULL;
    AVFormatContext *av_formate_context_out_audio = NULL;

    AVPacket av_packet;

    AVStream *in_stream_ps = NULL;
    AVStream *out_stream_es_video = NULL;
    AVStream *out_stream_es_audio = NULL;

    int videoindex = -1;
    int audioindex = -1;
    int frame_index = 0;

    int i_ret;

    av_formate_context_input = avformat_alloc_context();

    //打开一个输入流，并读取头信息。
    i_ret = avformat_open_input(&av_formate_context_input, m_input_ps_file_name, 0, NULL);
    if (i_ret < 0)
    {
        LOG("Open input file failed.");
        return false;
    }

    //获取媒体流信息
    i_ret = avformat_find_stream_info(av_formate_context_input, NULL);
    if (i_ret < 0)
    {
        LOG("Retrieve iniput stream info failed.");
        return false;
    }

    // 获取输出文件格式信息
    avformat_alloc_output_context2(&av_formate_context_out_video, NULL, NULL, m_output_es_video_file_name);
    if (!av_formate_context_out_video)
    {
        LOG("Create output context failed.");
        return false;
    }
    else
    {
        av_output_formate = av_formate_context_out_video->oformat;
    }

    // Open output file
    if (!(av_output_formate->flags & AVFMT_NOFILE))
    {
        if (avio_open(&av_formate_context_out_video->pb, m_output_es_video_file_name, AVIO_FLAG_WRITE) < 0)
        {
            LOG("Could not open output file '%s'", m_output_es_video_file_name);
            return false;
        }
    }

    //打开输出流
    out_stream_es_video = avformat_new_stream(av_formate_context_out_video, av_formate_context_out_video->video_codec);

    if (!out_stream_es_video)
    {
        LOG("Allocating output stream failed.");
        return false;
    }

    // Write file header
    if (avformat_write_header(av_formate_context_out_video, NULL) < 0)
    {
        LOG("Error occurred when opening video output file.");
        return false;
    }

    //获取视频流索引
    for (int i = 0; i < av_formate_context_input->nb_streams; ++i)
    {
        in_stream_ps = av_formate_context_input->streams[i];

        if (AVMEDIA_TYPE_VIDEO == in_stream_ps->codecpar->codec_type)
        {
            videoindex = i;
        }
        else
        {
            break;
        }
    }

    frame_index = 0;

    while (1)
    {
        // Get an AVPacket
        if (av_read_frame(av_formate_context_input, &av_packet) < 0)
        {
            LOG("end of file!\n");
            break;
        }

        if (videoindex == av_packet.stream_index)
        {
            LOG("Write Video Packet. size: %d\t pts: %d\n", av_packet.size, av_packet.pts);
        }
        else
        {
            continue;
        }

        // Write
        if (av_interleaved_write_frame(av_formate_context_out_video, &av_packet) < 0)
        {
            LOG("Error when write_frame.");
            break;
        }

        LOG("Write %8d frames to output file.", frame_index);

        av_packet_unref(&av_packet);

        ++frame_index;
    }

    // Write file trailer
    if (av_write_trailer(av_formate_context_out_video) != 0)
    {
        LOG("Error occurred when writing file trailer.");
        return false;
    }

    avformat_free_context(av_formate_context_out_video);
    avformat_free_context(av_formate_context_input);

    return true;
}

int callback_read_data(void *opaque, uint8_t *buf, int buf_size)
{
    int size = buf_size;
    int ret;
    // printf("read data %d\n", buf_size);
    do
    {
        //ret = get_queue(&recvqueue, buf, buf_size);
    } while (ret);
    // printf("read data Ok %d\n", buf_size);
    return size;
}

#define BUF_SIZE (8*1024*1024)
#define AVCODEC_MAX_AUDIO_FRAME_SIZE (8*1024)

bool CDemuxer::demux_ps_to_es_network()
{

    uint8_t *media_buffer = (uint8_t *)av_malloc(sizeof(uint8_t) * BUF_SIZE);

    AVCodec *pVideoCodec, *pAudioCodec;
    AVCodecContext *pVideoCodecCtx = NULL;
    AVCodecContext *pAudioCodecCtx = NULL;
    AVIOContext *av_io_context = NULL;
    AVInputFormat *av_input_format = NULL;
    AVFormatContext *av_format_context = NULL;

    //step1:申请一个AVIOContext
    av_io_context = avio_alloc_context(media_buffer, BUF_SIZE, 0, NULL, callback_read_data, NULL, NULL);
    if (!av_io_context)
    {
        fprintf(stderr, "avio alloc failed!\n");
        return -1;
    }

    //step2:探测流格式
    if (av_probe_input_buffer(av_io_context, &av_input_format, "", NULL, 0, 0) < 0)
    {
        fprintf(stderr, "probe failed!\n");
        return -1;
    }
    else
    {
        fprintf(stdout, "probe success!\n");
        fprintf(stdout, "format: %s[%s]\n", av_input_format->name, av_input_format->long_name);
    }

    //step3:这一步很关键
    av_format_context = avformat_alloc_context();
    av_format_context->pb = av_io_context; 
    
    //step4:打开流
    if (avformat_open_input(&av_format_context, "", av_input_format, NULL) < 0)
    {
        fprintf(stderr, "avformat open failed.\n");
        return -1;
    }
    else
    {
        fprintf(stdout, "open stream success!\n");
    }

    //以下就和文件处理一致了
    if (avformat_find_stream_info(av_format_context, NULL) < 0)
    {
        fprintf(stderr, "could not fine stream.\n");
        return -1;
    }
    av_dump_format(av_format_context, 0, "", 0);
    int videoindex = -1;
    int audioindex = -1;
    for (int i = 0; i < av_format_context->nb_streams; i++)
    {
        if ((av_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) &&
            (videoindex < 0))
        {
            videoindex = i;
        }
        if ((av_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) &&
            (audioindex < 0))
        {
            audioindex = i;
        }
    }
    if (videoindex < 0 || audioindex < 0)
    {
        fprintf(stderr, "videoindex=%d, audioindex=%d\n", videoindex, audioindex);
        return -1;
    }
    AVStream *pVst, *pAst;
    pVst = av_format_context->streams[videoindex];
    pAst = av_format_context->streams[audioindex];
    pVideoCodecCtx = pVst->codec;
    pAudioCodecCtx = pAst->codec;
    pVideoCodec = avcodec_find_decoder(pVideoCodecCtx->codec_id);
    if (!pVideoCodec)
    {
        fprintf(stderr, "could not find video decoder!\n");
        return -1;
    }

    pAudioCodec = avcodec_find_decoder(pAudioCodecCtx->codec_id);
    if (!pAudioCodec)
    {
        fprintf(stderr, "could not find audio decoder!\n");
        return -1;
    }

    int got_picture;
    uint8_t samples[AVCODEC_MAX_AUDIO_FRAME_SIZE * 3 / 2];
    AVFrame *pframe = av_frame_alloc();
    AVPacket pkt;
    av_init_packet(&pkt);
    while (1)
    {
        if (av_read_frame(av_format_context, &pkt) >= 0)
        {
            if (pkt.stream_index == videoindex)
            {
                fprintf(stdout, "pkt.size=%d,pkt.pts=%lld, pkt.data=0x%x.", pkt.size, pkt.pts, (unsigned int)pkt.data);
            }
            else if (pkt.stream_index == audioindex)
            {

            }
            av_free_packet(&pkt);
        }
    }

    av_free(media_buffer);
    av_frame_free(&pframe);

    return 0;
}