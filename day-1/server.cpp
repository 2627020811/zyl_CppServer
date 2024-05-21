#include <sys/socket.h>
#include <string.h> //bzero
#include<stdio.h>
#include <arpa/inet.h> //这个头文件包含了<netinet/in.h>，不用再次包含了
#include<iostream>
int main()
{
    // 参数解释：参数一：ip地址类型，IPV4使用AF_INET；参数二：数据传输方式，SOCK_STREAM表示流格式、面向连接，多用于TCP
    // 参数三：协议选择，0表示根据前两个参数自动推导协议类型。设置为IPPROTO_TCP和IPPTOTO_UDP，分别表示TCP和UDP。
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 对于客户端，服务端唯一存在的标识就是ip地址和端口，因此我们要把这个socket绑定到一个ip和端口上去，首先创建一个sockadd_in结构体
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr)); // 是C语言的函数，能把内存块（字符串）的前n个字节置0

    //设置地址族、IP地址和端口
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port=htons(8888); //这一行设置了服务器监听的端口号为 8888，并使用 htons() 函数将端口号从主机字节序转换为网络字节序（大端字节序），然后将其存储到 serv_addr.sin_port 中。

    // 然后将socket地址与一个已创建的文件描述符绑定
    bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr)); //bind 函数用于将一个 socket 与特定的 IP 地址和端口绑定，以便监听该 IP 地址和端口的网络数据。(sockaddr*)&serv_addr 将 serv_addr 结构体类型转换为 sockaddr 类型，因为 bind 函数的第二个参数需要接受 sockaddr 类型的指针，为什么要这么做网上有说

    // 使用listen监听这个 socket窗口，listen函数的第二个参数地址是最大监听队列长度，系统建议最大值SOMAXCONN为128
    listen(sockfd,SOMAXCONN);

    // 要接受一个客户端连接，需要使用accept函数。对于每一个客户端，我们在接受连接时也需要保存客户端的socket地址信息
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len=sizeof(clnt_addr);
    memset(&clnt_addr, 0, sizeof(clnt_addr));

    int clnt_sockfd=accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
    // std::cout<<"server.cpp sockfd"<<sockfd<< std::endl;
    printf("new client fd %d! IP:%s Port:%d\n",clnt_sockfd,inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
    //inet_ntoa(clnt_addr.sin_addr) 用于将 clnt_addr 结构体中的客户端 IP 地址转换为点分十进制的字符串格式。
    //ntohs(clnt_addr.sin_port) 将 clnt_addr 结构体中的客户端端口号从网络字节序（大端字节序）转换为主机字节序（小端字节序），以便打印出端口号。
    
    // 要注意和accept和bind的第三个参数有一点区别，对于bind只需要传入serv_addr的大小即可，而accept需要写入客户端socket长度，所以需要定义一个类型为socklen_t的变量，并传入这个变量的地址。
    
    // 另外，accept函数会阻塞当前程序，直到有一个客户端socket被接受后程序才会往下运行。

    return 0;
}