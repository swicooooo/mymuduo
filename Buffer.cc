#include "Buffer.h"

#include <cerrno>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// readv能使用多段非连续的空间存储数据
int Buffer::readFd(int fd, int *saveErrno)
{
    char extraBuf[65536] = {0}; // 64k,最大存储空间

    int writeable = writeableBytes();
    struct iovec iov[2]; // 第一块用buffer堆存储，第二块用extraBuf栈存储
    iov[0].iov_base = beginWrite();
    iov[0].iov_len = writeable;

    iov[1].iov_base = extraBuf;
    iov[1].iov_len = sizeof extraBuf;

    ssize_t n = ::readv(fd, iov, writeable<sizeof extraBuf?2:1); 
    if(n < 0){
        *saveErrno = errno;
    }
    else if(n <= writeable) {   // buffer够写
        writerIndex_ += n;
    }
    else {  // 多余的存储在extraBuf
        writerIndex_ = buffer_.size();
        append(extraBuf, n-writeable);
    }
    return n;
}

int Buffer::writeFd(int fd, int *saveErrno)
{
    int n = ::write(fd, peek(), readableBytes());   
    if(n < 0) {
        *saveErrno = errno;
    }
    return n;
}