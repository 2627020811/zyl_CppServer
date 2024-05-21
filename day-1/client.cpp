#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<iostream>
int main()
{
    //客户端代码和服务器代码几乎一样：创建一个socket文件描述符，与一个IP地址和端口绑定，最后并不是监听这个端口，而是使用connect函数尝试连接这个服务器。
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    // std::cout<<"client.cpp sockfd first"<<sockfd<<std::endl;
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port=htons(8888);
    connect(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr));
    // std::cout<<"client.cpp sockfd"<<sockfd<<std::endl;
    return 0;
}