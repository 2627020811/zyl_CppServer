#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop)
{
    sock = new Socket();
    addr = new InetAddress("127.0.0.1", 8888);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();

    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading(); // 添加监听事件
}

Acceptor::~Acceptor()
{
    delete sock;
    delete addr;
    delete acceptChannel;
}

void Acceptor::acceptConnection()
{
    newConnectionCallback(sock); // 执行这个可调用对象
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket *)> _cb)
{
    newConnectionCallback = _cb; // 设置这个可调用对象
}