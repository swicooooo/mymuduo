#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int32_t Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name) :
    started_(false),joined_(false),func_(std::move(func)),name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        thread_->detach();
    }
}

// 开启单独线程
void Thread::start()
{
    sem_t sem;
    sem_init(&sem,false,0);

    started_ = true;
    thread_ = std::make_shared<std::thread>([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    });

    sem_wait(&sem); // 等待tid_初始化成功
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread %d ", num);
        name_ = buf;
    }
}
