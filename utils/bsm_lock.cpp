#include "bsm_lock.h"

namespace bsm {
namespace bsm_utils {

bsm_lock::bsm_lock()
{
#ifndef _WIN32
    pthread_mutex_init(&m_mutex, NULL);
#else
    m_mutex = ::CreateMutex(NULL, false, NULL);
#endif
}

bsm_lock::~bsm_lock()
{
#ifndef _WIN32
    pthread_mutex_destroy(&m_mutex);
#else
    ::CloseHandle(m_mutex);
#endif
}

void bsm_lock::lock() const
{
#ifndef _WIN32
    pthread_mutex_lock(&m_mutex);
#else
    DWORD dw = WaitForSingleObject(m_mutex, INFINITE);
#endif
}

void bsm_lock::unlock() const
{
#ifndef _WIN32
    pthread_mutex_unlock(&m_mutex);
#else
    ::ReleaseMutex(m_mutex);
#endif
}

} //namespace bsm_utils
} //namespace bsm