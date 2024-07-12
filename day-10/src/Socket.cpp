#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>

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

void Socket::bind(InetAddress *_Inetaddr)
{
    struct sockaddr_in addr = _Inetaddr->getAddr();
    socklen_t addr_len = _Inetaddr->getAddr_len();

    errif(::bind(fd, (sockaddr *)&addr, addr_len) == -1, "socket bind error");
    _Inetaddr->setInetAddr(addr, addr_len);
}

void Socket::listen()
{
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress *_Inetaddr)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    int clnt_sockfd = ::accept(fd, (sockaddr *)&addr, &addr_len);
    errif(clnt_sockfd==-1,"socket accept error");
    _Inetaddr->setInetAddr(addr,addr_len);
    return clnt_sockfd;
}

void Socket::connect(InetAddress *_addr)
{
    struct sockaddr_in addr=_addr->getAddr();
    socklen_t addr_len=_addr->getAddr_len();
    errif(::connect(fd,(sockaddr*)&addr,addr_len)==-1,"socket connect error");
}

int Socket::getFd()
{
    return fd;
}