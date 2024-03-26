#include "CurrentThread.h"
#include <unistd.h>

__thread int t_cacheTid = 0;

void CurrentThread::tid()
{
    if(t_cacheTid == 0) {
        t_cacheTid = ::syscall(SYS_gettid);
    }
}