#include "stdio.h"
#include "Demuxer.h"

/**
DTS��Decoding Time Stamp����������ʱ��������ʱ������������ڸ��߲���������ʲôʱ�������һ֡�����ݡ�
PTS��Presentation Time Stamp��������ʾʱ��������ʱ����������߲���������ʲôʱ����ʾ��һ֡�����ݡ�
*/

int main(int argc, char* argv[])
{
    CDemuxer demuxer;
    demuxer.set_input_ps_file("E://tmp1.ps");
    demuxer.set_output_es_video_file("E://tmp1.h264");

    demuxer.demux_ps_to_es();
}