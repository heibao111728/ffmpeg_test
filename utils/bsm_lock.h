#ifndef __BSM_LOCK_H__
#define __BSM_LOCK_H__

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace bsm {
namespace bsm_utils {

    class bsm_i_lock
    {
    public:
        virtual ~bsm_i_lock() {};
        virtual void lock() const = 0;
        virtual void unlock() const = 0;
    };

    class bsm_lock : public bsm_i_lock
    {
    public:
        bsm_lock();
        ~bsm_lock();

        virtual void lock() const;
        virtual void unlock() const;

    private:
#ifndef _WIN32
        mutable pthread_mutex_t m_mutex;
#else
        HANDLE m_mutex;
#endif
    };
} //namespace bsm_utils
} //namespace bsm

#endif // !__BSM_LOCK_H__

