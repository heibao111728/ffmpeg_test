#include "stdio.h"
#include "Demuxer.h"
#include "Demuxer2.h"

/**
DTS��Decoding Time Stamp����������ʱ��������ʱ������������ڸ��߲���������ʲôʱ�������һ֡�����ݡ�
PTS��Presentation Time Stamp��������ʾʱ��������ʱ����������߲���������ʲôʱ����ʾ��һ֡�����ݡ�
*/

int main(int argc, char* argv[])
{
#if 0
    CDemuxer demuxer;
    demuxer.set_input_ps_file("E://tmp1.ps");
    demuxer.set_output_es_video_file("E://tmp1.h264");

    demuxer.demux_ps_to_es();
#endif

#if 1
    CDemuxer2 ps_demuxer;
    ps_demuxer.setup_src_ps_file("E://tmp1.ps");
    ps_demuxer.setup_dst_es_video_file("E://tmp1_bsm.h264");

    if (ps_demuxer.open_src_ps_file())
    {
        ps_demuxer.do_demux();
    }

    ps_demuxer.close_src_ps_file();
    return 0;
#endif
}