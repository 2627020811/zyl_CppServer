#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket() : fd(-1)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}
Socket::Socket(int _fd) : fd(_fd)
{
    errif(fd == -1, "socket create error");
}

Socket::~Socket()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress *addr)
{
    errif(::bind(fd, (sockaddr *)&addr->addr, addr->addr_len) == -1, "socket bind error");
}

void Socket::listen()
{
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking()
{
    // setnonblocking(int fd) 函数用于将指定的文件描述符 fd 设置为非阻塞模式。在非阻塞模式下，读写操作不会阻塞进程，即使没有立即可以完成的数据，这使得程序可以更有效地处理异步 I/O 操作，特别适用于事件驱动的编程模型（如使用 epoll 等）。

    int flags = fcntl(fd, F_GETFL); // 获取当前文件描述符的状态标志
    flags |= O_NONBLOCK;            // 添加 O_NONBLOCK 非阻塞模式标志
    fcntl(fd, F_SETFL, flags);      // 将新的状态标志应用到文件描述符上
}

int Socket::accept(InetAddress *addr)
{
    int clnt_sockfd = ::accept(fd, (sockaddr *)&addr->addr, &addr->addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    return clnt_sockfd;
}

int Socket::getFd()
{
    return fd;
}