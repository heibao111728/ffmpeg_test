#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "stdio.h"
#include "stdint.h"


typedef enum bsm_log_type
{
    log_type_stdout = 1,
    log_type_file = 2
}bsm_log_type_e;

class bsm_logger
{
public:
    static bsm_log_type m_log_type;
    static bsm_logger* m_instance;
    static bsm_logger* get_instance()
    {
        if (NULL != m_instance)
        {
            return m_instance;
        }
        else
        {
            m_instance = new bsm_logger();
            return m_instance;
        }
    }
    static bsm_log_type_e get_log_type();
    static void set_log_type(bsm_log_type_e log_type);

    bool init_logger(char* file_name);
    void uninit_logger();

    ~bsm_logger() {}
public:
    FILE* m_fp_logfile;
private:
    bsm_logger();

private:  
};

#define LOG(fmt, ...) do{\
    if(log_type_stdout == bsm_logger::get_log_type() )\
    {\
        fprintf(stdout, "[DEBUG] %s\n%s:%d:" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
    }\
    else if(log_type_file == bsm_logger::get_log_type() )\
    {\
        if(NULL != bsm_logger::get_instance()->m_fp_logfile)\
        {\
            fprintf(bsm_logger::get_instance()->m_fp_logfile, "[DEBUG] %s\n%s:%d:" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
        }\
        else\
        {\
            printf("log_file not init.\n");\
        }\
    }}while(0);

#endif // !__LOGGER_H__

