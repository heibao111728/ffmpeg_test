#include "stdio.h"
#include "Demuxer.h"

/**
DTS（Decoding Time Stamp）：即解码时间戳，这个时间戳的意义在于告诉播放器该在什么时候解码这一帧的数据。
PTS（Presentation Time Stamp）：即显示时间戳，这个时间戳用来告诉播放器该在什么时候显示这一帧的数据。
*/

int main(int argc, char* argv[])
{
    CDemuxer demuxer;
    demuxer.set_input_ps_file("E://tmp1.ps");
    demuxer.set_output_es_video_file("E://tmp1.h264");

    demuxer.demux_ps_to_es();
}