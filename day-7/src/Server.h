#pragma once

class EventLoop;
class Socket;
class Acceptor;

// 接下来，介绍下「单 Reactor 单进程」这个方案：
// Reactor 对象通过 select （IO 多路复用接口） 监听事件，收到事件后通过 dispatch 进行分发，具体分发给 Acceptor 对象还是 Handler 对象，还要看收到的事件类型；
// 如果是连接建立的事件，则交由 Acceptor 对象进行处理，Acceptor 对象会通过 accept 方法 获取连接，并创建一个 Handler 对象来处理后续的响应事件；
// 如果不是连接建立事件， 则交由当前连接对应的 Handler 对象来进行响应；
// Handler 对象通过 read -> 业务处理 -> send 的流程来完成完整的业务流程。

class Server
{
private:
    EventLoop *loop;
    Acceptor *acceptor;

public:
    Server(EventLoop *);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *serv_sock);
};