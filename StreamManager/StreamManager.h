#ifndef __STREAMMANAGER_H__
#define __STREAMMANAGER_H__

#include <stdio.h>
#include <stdlib.h>

#include "utils\bsm_lock.h"
#include "utils\logger.h"

using namespace bsm;
using namespace bsm_utils;

class stream_manager
{
public:
    stream_manager(int capacity = 1*1024*1024);
    ~stream_manager();

    
    void set_capacity_size(int capacity_size);

    /**
    *   description:
    *       read length of data to buffer.
    */
    int pull_data(void* op, unsigned char* buffer, int length);

    /**
    *   description:
    *       write data.
    */
    int push_data(unsigned char* data, int length);

    int get_data_length();

private:
    unsigned char* m_stream_data_buffer;
    int m_stream_start_point;
    int m_stream_end_point;
    int m_data_length;
    int m_capacity;

    bsm_lock m_lock;
};

#endif // !__STREAMMANAGER_H__

