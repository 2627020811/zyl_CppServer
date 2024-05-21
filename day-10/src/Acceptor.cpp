#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <stdio.h>

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop), sock(nullptr), acceptChannel(nullptr)
{
    //服务器接受连接模块
    sock = new Socket(); //服务器的socket
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);

    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();
    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb); //acceptConnection

        // 即InetAddress *clnt_addr = new InetAddress();
        // Socket *clnt_sock = new Socket(sock->accept(clnt_addr));
        // printf("new client fd %d! IP: %s, Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->getAddr().sin_addr), ntohs(clnt_addr->getAddr().sin_port));
        // clnt_sock->setnonblocking();
        // newConncectionCallback(clnt_sock); 

    acceptChannel->enableReading();
    delete addr;
}

Acceptor::~Acceptor()
{
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection()
{
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s, Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->getAddr().sin_addr), ntohs(clnt_addr->getAddr().sin_port));
    clnt_sock->setnonblocking();
    newConncectionCallback(clnt_sock); //Server::newConnection(Socket *sock)

        // 即Connection *conn = new Connection(loop, sock);
        // Connection():
            // channel=new Channel(loop,sock->getFd());
            // std::function<void()> cb=std::bind(&Connection::echo,this,sock->getFd());
            // channel->setCallback(cb);
            // channel->enableReading();
            // readBuffer=new Buffer();
        
        // std::function<void(Socket *)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        // conn->setDeleteConnectionCallback(cb);
        // connections[sock->getFd()] = conn;

    delete clnt_addr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb)
{
    newConncectionCallback=_cb;
}
