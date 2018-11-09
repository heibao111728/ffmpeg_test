#include "logger.h"

#include <stdio.h>

bsm_log_type_e bsm_logger::m_log_type = log_type_stdout;
bsm_logger* bsm_logger::m_instance = NULL;

bsm_logger::bsm_logger()
{
    m_fp_logfile = NULL;
}

bool bsm_logger::init_logger(char* file_name)
{
    //FILE* p_logfile;
    if (0 == fopen_s(&m_fp_logfile, file_name, "w+"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bsm_log_type_e bsm_logger::get_log_type()
{
    return m_log_type;
}

void bsm_logger::set_log_type(bsm_log_type_e log_type)
{
    m_log_type = log_type;
}

void bsm_logger::uninit_logger()
{
    if (NULL != m_fp_logfile)
    {
        fclose(m_fp_logfile);
    }
}


