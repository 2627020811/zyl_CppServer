#pragma once
#include "Macros.h"

#include <functional>

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
public:
    // 在C++中，explicit关键字用来修饰类的构造函数，被修饰的构造函数的类，不能发生相应的隐式类型转换，只能以显式的方式进行类型转换。
    explicit Acceptor(EventLoop *loop);
    ~Acceptor();
    DISALLOW_COPY_AND_MOVE(Acceptor);
    void AcceptConnection();
    void SetNewConnectionCallback(std::function<void(Socket *)> const &callback);

private:
    EventLoop *loop_;
    Socket *sock_;
    Channel *channel_;
    std::function<void(Socket *)> new_connection_callback_;
};