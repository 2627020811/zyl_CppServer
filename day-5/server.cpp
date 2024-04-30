#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include "util.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd)
{
    // setnonblocking(int fd) 函数用于将指定的文件描述符 fd 设置为非阻塞模式。在非阻塞模式下，读写操作不会阻塞进程，即使没有立即可以完成的数据，这使得程序可以更有效地处理异步 I/O 操作，特别适用于事件驱动的编程模型（如使用 epoll 等）。

    int flags = fcntl(fd, F_GETFL); // 获取当前文件描述符的状态标志
    flags |= O_NONBLOCK;            // 添加 O_NONBLOCK 非阻塞模式标志
    fcntl(fd, F_SETFL, flags);      // 将新的状态标志应用到文件描述符上
}

void handleReadEvent(int fd);

int main()
{
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();

    Epoll *ep = new Epoll();
    serv_sock->setnonblocking();
    Channel *servChannel = new Channel(ep, serv_sock->getFd());
    servChannel->enableReading(); //监视可读事件

    while (true)
    {
        std::vector<Channel *> activeChannels = ep->poll();
        int nfds = activeChannels.size();

        for (int i = 0; i < nfds; i++)
        {
            int chfd = activeChannels[i]->getFd();
            if (chfd == serv_sock->getFd()) // 新客户端连接
            {
                InetAddress *clnt_addr = new InetAddress();                   // 没有delete,会发生内存泄露
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // 没有delete,会发生内存泄露
                printf("new client fd %d! IP: %s, Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->setnonblocking();
                Channel *clntChannel = new Channel(ep, clnt_sock->getFd());
                clntChannel->enableReading();
            }
            else if (activeChannels[i]->getEvents() & EPOLLIN)
            {
                // 通过按位与运算符 & 来判断 events[i].events 是否包含了 EPOLLIN 标志位，以确定是否发生了可读事件。这是因为在 epoll 的事件结构 epoll_event 中，每个事件都是一个位掩码，其中包含了多个不同的事件标志位，如可读事件 EPOLLIN、可写事件 EPOLLOUT、错误事件 EPOLLERR 等。
                handleReadEvent(activeChannels[i]->getFd());
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }
    delete serv_sock;
    delete serv_addr;
    return 0;
}

void handleReadEvent(int sockfd)
{
    char buf[READ_BUFFER];
    while (true) // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
    {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0)
        {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }
        else if (bytes_read == -1 && errno == EINTR) // 客户端正常中断、继续读取
        {
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 非阻塞IO，这个条件表示数据已经全部读取完毕，可以退出循环了
        {
            printf("finish reading once,errno:%d\n", errno);
            break;
        }
        else if (bytes_read == 0)
        {
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}