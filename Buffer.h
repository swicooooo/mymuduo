#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <algorithm>

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer
/// @endcode
{
public:
    static const std::size_t KCheapPrepend = 8;
    static const std::size_t KInitialSize = 1024;

    explicit Buffer(std::size_t initialSize = KInitialSize)
        : buffer_(KCheapPrepend+KInitialSize), readerIndex_(KCheapPrepend), writerIndex_(KCheapPrepend){}

    /**
     * 获取三个区间大小 | prepend ---- read ---- write |
    */
    std::size_t readableBytes() const{ return writerIndex_ - readerIndex_; }
    std::size_t writeableBytes() const{ return buffer_.size() - writerIndex_; }
    std::size_t prependableBytes() const{ return readerIndex_; }

    /**
     * 消费可读数据
    */
    void retrieve(std::size_t len){
        if(len < readableBytes()) {
            readerIndex_ += len;
        }else{
            retrieveAll();
        }
    }
    void retrieveAll(){ 
        readerIndex_ = KCheapPrepend;
        writerIndex_ = KCheapPrepend;
    }
    std::string retrieveAsString(std::size_t len){
        std::string ret(peek(),len);
        retrieve(len);
        return ret;
    }
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes()); 
    }

    /**
     * 获取buffer首地址
    */
    const char* peek() const{ return begin() + readerIndex_; }  

    /**
     * 获取数据  
    */
    int readFd(int fd, int *saveErrno);
    int writeFd(int fd, int *saveErrno);

    void append(const char *data, std::size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }
    void ensureWriteableBytes(std::size_t len) {
        if(writeableBytes() < len) {
            makeSpace(len);
        }
    }
    void makeSpace(std::size_t len) {
        // 总可写区间小于要写入数据长度,则直接扩容
        if(readerIndex_+writeableBytes() < len+KCheapPrepend) {
            buffer_.resize(writerIndex_+len);
        }else{
            // 移动可读区域到KCheapPrepend
            std::copy(begin()+readerIndex_,begin()+writerIndex_,begin()+KCheapPrepend);
            readerIndex_ = KCheapPrepend;
            writerIndex_ = readerIndex_ + readableBytes();
        }
    }

private:
    char* begin(){ return &*buffer_.begin(); }
    const char* begin() const{ return &*buffer_.begin(); }
    char* beginWrite() { return begin()+writerIndex_; }
    const char* beginWrite() const { return begin()+writerIndex_; }

    std::vector<char> buffer_;
    std::size_t readerIndex_;
    std::size_t writerIndex_;
};