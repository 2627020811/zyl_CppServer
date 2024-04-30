#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include "util.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd)
{
    // setnonblocking(int fd) 函数用于将指定的文件描述符 fd 设置为非阻塞模式。在非阻塞模式下，读写操作不会阻塞进程，即使没有立即可以完成的数据，这使得程序可以更有效地处理异步 I/O 操作，特别适用于事件驱动的编程模型（如使用 epoll 等）。

    int flags = fcntl(fd, F_GETFL); // 获取当前文件描述符的状态标志
    flags |= O_NONBLOCK;            // 添加 O_NONBLOCK 非阻塞模式标志
    fcntl(fd, F_SETFL, flags);      // 将新的状态标志应用到文件描述符上
}

void handleReadEvents(int);

int main()
{
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    Epoll *ep = new Epoll();
    serv_sock->setnonblocking();
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);

    while (true)
    {
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();

        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == serv_sock->getFd()) // 新客户端连接，把客户端的socket存下来
            {
                InetAddress *clnt_addr = new InetAddress();                   // 会发生内存泄漏！没有delete
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // 会发生内存泄漏！没有delete
                printf("new client fd %d! IP:%s Port:%d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->setnonblocking();
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET);
            }
            else if (events[i].events & EPOLLIN) // 可读事件 EPOLLIN 是 epoll 中用于表示可读事件的标志位
            {
                handleReadEvents(events[i].data.fd);
            }
            else
            {
                // 其他事件，其他版本实现
                printf("something else happened!\n");
            }
        }
    }
    delete serv_sock;
    delete serv_addr;
    return 0;
}

void handleReadEvents(int sockfd)
{
    char buf[READ_BUFFER];
    while (true)
    {
        memset(buf, 0, sizeof(buf));

        ssize_t bytes_read = read(sockfd, buf, sizeof(buf)); // 如果上面没有设置为非阻塞，那这里服务器就会被阻塞住了
        if (bytes_read > 0)
        {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }
        else if (bytes_read == -1 && errno == EINTR) // 客户端正常中断，继续读取
        {
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 非阻塞IO，这个条件表示数据全部读取完毕
        {
            printf("finish reading once,errno:%d\n", errno);
            break;
        }
        else if (bytes_read == 0) // EOF，客户端断开连接
        {
            printf("EOF,client fd %d disconnected\n", sockfd);
            close(sockfd);
            break;
        }
    }
}