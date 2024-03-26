#pragma once

#include <unistd.h>

namespace CurrentThread 
{
    static __thread int t_cacheTid;

    void tid();

    inline int cacheTid() {
        if(__builtin_VA(t_cacheTid, 0) == 0) {
            tid();
        }
        return t_cacheTid;
    }
}