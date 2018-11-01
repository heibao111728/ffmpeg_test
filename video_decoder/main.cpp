#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"

/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

int callback_read_data(void *opaque, uint8_t *buf, int buf_size)
{
    int size = buf_size;
    int ret = 1;
    // printf("read data %d\n", buf_size);
    do
    {
        //ret = get_queue(&recvqueue, buf, buf_size);
    } while (ret);
    // printf("read data Ok %d\n", buf_size);
    return size;
}

int main(int argc, char* argv[])
{
#if 1
    CDemuxer demuxer;

    CDemuxer::setup_callback_function(callback_read_data);

    demuxer.set_input_ps_file("E://success_data//tmp1.ps");
    demuxer.set_output_es_video_file("E://success_data//tmp1.h264");

    demuxer.demux_ps_to_es();
#endif

#if 0
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
}