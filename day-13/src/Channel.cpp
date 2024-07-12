#include "Channel.h"

#include <sys/epoll.h>
#include <unistd.h>

#include <utility>

#include "EventLoop.h"
#include "Socket.h"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), listen_events_(0), ready_events_(0), in_epoll_(false) {}

Channel::~Channel()
{
    if (fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

void Channel::HandleEvent()
{
    // ready_events_ 是一个变量，表示已准备好的事件（通常是通过 epoll 或类似的事件驱动机制获得）。
    // EPOLLIN 和 EPOLLPRI 是事件类型的标志位，表示可读事件和高优先级可读事件。
    // & 是按位与操作符，用于检查 ready_events_ 是否包含指定的事件标志位。
    if (ready_events_ & (EPOLLIN | EPOLLPRI))
    {
        read_callback_();
    }
    // EPOLLOUT 是事件类型的标志位，表示可写事件。
    if (ready_events_ & (EPOLLOUT))
    {
        write_callback_();
    }
}

void Channel::EnableRead()
{
    // listen_events_：表示关注的事件类型的标志位。
    // loop_：表示事件循环对象的指针。
    // EPOLLIN 和 EPOLLPRI 是事件类型的标志位，分别表示可读事件和高优先级可读事件。
    // |= 是按位或赋值操作符，用于将指定的事件标志位添加到 listen_events_ 中。
    listen_events_ |= EPOLLIN | EPOLLPRI;
    loop_->UpdateChannel(this);
}

void Channel::UseET()
{
    listen_events_ |= EPOLLET;
    loop_->UpdateChannel(this);
}

int Channel::GetFd() { return fd_; }

uint32_t Channel::GetListenEvents() { return listen_events_; }
uint32_t Channel::GetReadyEvents() { return ready_events_; }

bool Channel::GetInEpoll() { return in_epoll_; }

void Channel::SetInEpoll(bool in) { in_epoll_ = in; }

void Channel::SetReadyEvents(uint32_t ev) { ready_events_ = ev; }

// std::function 是 C++ 标准库中的一个通用函数封装类模板。它可以包装和存储各种可调用对象，包括函数指针、函数对象、Lambda 表达式等。
// <void()> 是 std::function 模板参数，表示封装的函数对象的签名。在这种情况下，void() 表示一个没有参数且返回类型为 void 的函数对象。
// const & 表示该 std::function 类型的引用是一个常量引用，即不可修改的引用。
void Channel::SetReadCallback(std::function<void()> const &callback) { read_callback_ = callback; }