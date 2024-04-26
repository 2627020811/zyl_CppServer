#include <sys/socket.h>
#include <string.h> //bzero
#include <stdio.h>
#include <arpa/inet.h> //这个头文件包含了<netinet/in.h>，不用再次包含了
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
    // setnonblocking(int fd) 函数用于将指定的文件描述符 fd 设置为非阻塞模式。在非阻塞模式下，读写操作不会阻塞进程，即使没有立即可以完成的数据，这使得程序可以更有效地处理异步 I/O 操作，特别适用于事件驱动的编程模型（如使用 epoll 等）。

    int flags = fcntl(fd, F_GETFL); // 获取当前文件描述符的状态标志
    flags |= O_NONBLOCK; // 添加 O_NONBLOCK 非阻塞模式标志
    fcntl(fd, F_SETFL, flags); // 将新的状态标志应用到文件描述符上
}
int main()
{
    // 参数解释：参数一：ip地址类型，IPV4使用AF_INET；参数二：数据传输方式，SOCK_STREAM表示流格式、面向连接，多用于TCP
    // 参数三：协议选择，0表示根据前两个参数自动推导协议类型。设置为IPPROTO_TCP和IPPTOTO_UDP，分别表示TCP和UDP。
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    // 对于客户端，服务端唯一存在的标识就是ip地址和端口，因此我们要把这个socket绑定到一个ip和端口上去，首先创建一个sockadd_in结构体
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // 是C语言的函数，能把内存块（字符串）的前n个字节置0

    // 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888); // 这一行设置了服务器监听的端口号为 8888，并使用 htons() 函数将端口号从主机字节序转换为网络字节序（大端字节序），然后将其存储到 serv_addr.sin_port 中。

    // 然后将socket地址与一个已创建的文件描述符绑定
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error"); // bind 函数用于将一个 socket 与特定的 IP 地址和端口绑定，以便监听该 IP 地址和端口的网络数据。(sockaddr*)&serv_addr 将 serv_addr 结构体类型转换为 sockaddr 类型，因为 bind 函数的第二个参数需要接受 sockaddr 类型的指针，为什么要这么做网上有说

    // 使用listen监听这个 socket窗口，listen函数的第二个参数地址是最大监听队列长度，系统建议最大值SOMAXCONN为128
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    memset(events, 0, sizeof(events));
    memset(&ev, 0, sizeof(ev));

    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;//关注可读事件，并使用边缘触发模式

    setnonblocking(sockfd);

    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); // 将文件描述符 sockfd 添加到 epoll 实例 epfd 中，监视事件 ev。

    while (true)
    {
         //epoll_wait: 等待 epoll 实例 epfd 中的事件发生，最多等待 MAX_EVENTS 个事件，阻塞直到有事件发生或超时。
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); // 有nfds个fd发生事件,events是一个指向 struct epoll_event 数组的指针，用于存储发生事件的文件描述符及其事件类型。
       
        errif(nfds == -1, "epoll wait error");
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == sockfd) // 发生连接的fd是服务器的sockfd，表示有新客户端连接
            {
                struct sockaddr_in clnt_addr;
                memset(&clnt_addr, 0, sizeof(clnt_addr));
                socklen_t clnt_addr_len=sizeof(clnt_addr);

                int clnt_sockfd=accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
                errif(clnt_sockfd==-1,"socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n",clnt_sockfd,inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
                //将新的客户端套接字添加到 epoll 实例中，监听其读事件
                memset(&ev,0,sizeof(ev));
                ev.data.fd=clnt_sockfd;
                ev.events=EPOLLIN|EPOLLET;

                // 在默认的阻塞模式下，如果使用阻塞式 I/O 函数（如 read、write、accept 等）操作 clnt_sockfd，当没有数据可读或无法立即写入数据时，这些函数会导致服务器进程阻塞，即挂起等待直到操作完成。这样会导致服务器无法处理其他任务或其他客户端的请求，降低了服务器的并发性和响应性。所以设置为非阻塞
                setnonblocking(clnt_sockfd); //将指定文件描述符设置为非阻塞模式
                epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sockfd,&ev);
            }
            else if(events[i].events&EPOLLIN) //发生事件的是客户端，并且是可读事件
            {
                char buf[READ_BUFFER];
                while(true)
                {
                    memset(buf,0,sizeof(buf));

                    ssize_t bytes_read=read(events[i].data.fd,buf,sizeof(buf)); //如果上面没有设置为非阻塞，那这里服务器就会被阻塞住了
                    if(bytes_read>0)
                    {
                        printf("message from client fd %d: %s\n",events[i].data.fd,buf);
                        write(events[i].data.fd,buf,sizeof(buf));
                    }
                    else if(bytes_read==-1&&errno==EINTR) //客户端正常中断，继续读取
                    {
                        printf("continue reading");
                        continue;
                    }
                    else if(bytes_read==-1&&((errno==EAGAIN)||(errno==EWOULDBLOCK))) //非阻塞IO，这个条件表示数据全部读取完毕
                    {
                        printf("finish reading once,errno:%d\n",errno);
                        break;
                    }
                    else if(bytes_read==0) //EOF，客户端断开连接
                    {
                        printf("EOF,client fd %d disconnected\n",events[i].data.fd);
                        close(events[i].data.fd);
                        break;
                    }
                }
            }
            else
            {
                //其他事件，其它版本实现
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    return 0;
}