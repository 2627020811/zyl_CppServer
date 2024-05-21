#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr)
{
    // 新的 TCP 连接应该由 Server 类来创建和管理生命周期，而 Acceptor 类则负责接受连接请求并创建新的连接套接字。这样的组织结构既符合逻辑关系，又保持了服务器的通用性。
    acceptor = new Acceptor(loop); //新建连接，把服务端这边处理好，建立新建连接的监听事件，但延迟给这个监听事件的回调函数赋值，实际赋值为下两句代码的newConnection
    // 这样一来，新建连接的逻辑就在Acceptor类中。但逻辑上新socket建立后就和之前监听的服务器socket没有任何关系了，TCP连接和Acceptor一样，拥有以上提到的三个特点，这两个类之间应该是平行关系。所以新的TCP连接应该由Server类来创建并管理生命周期，而不是Acceptor。并且将这一部分代码放在Server类里也并没有打破服务器的通用性，因为对于所有的服务，都要使用Acceptor来建立连接。
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);  //接受连接，返回客户端socket并添加到epoll监听
    // std::placeholders::_1是占位符 是1不是l 
    //在 std::bind 的调用中，第一个参数是要绑定的可调用对象，可以是函数、函数指针、成员函数指针、lambda 表达式等。对于成员函数来说，我们需要提供一个对象的指针或引用，以便在调用时能够访问到对象的成员。
    acceptor->setNewConnectionCallback(cb); //建立连接后才赋值的真正回调函数
}

Server::~Server()
{
    delete acceptor;
}

void Server::handleReadEvent(int clnt_sockfd)
{
    char buf[READ_BUFFER];
    while (true) // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
    {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(clnt_sockfd, buf, sizeof(buf));
        if (bytes_read > 0)
        {
            printf("message from client fd %d: %s\n", clnt_sockfd, buf);
            write(clnt_sockfd, buf, sizeof(buf));
        }
        else if (bytes_read == -1 && errno == EINTR)
        { // 客户端正常中断、继续读取
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
        else if (bytes_read == 0)
        { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", clnt_sockfd);
            close(clnt_sockfd); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

void Server::newConnection(Socket *serv_sock)
{
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->setnonblocking();
    Channel *clntChannel=new Channel(loop,clnt_sock->getFd());
    std::function<void()> cb=std::bind(&Server::handleReadEvent,this,clnt_sock->getFd()); //针对客户端可读事件的回调函数，不是属于客户端的回调函数
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}
