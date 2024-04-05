#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cacheTid;

    void cacheTid();    // 获取linux下进程管理的当前线程id

    inline int tid() {
        if(__builtin_expect(t_cacheTid == 0, 0)) {  // 默认优化
            cacheTid();
        }
        return t_cacheTid;
    }      
}