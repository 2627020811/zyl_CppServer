#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include "util.h"

#define BUFFER_SIZE 1024

int main()
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    errif(sockfd==-1,"socket create error");

    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port=htons(8888);

    errif(connect(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))==-1,"socket connect error");

    while(true)
    {
        char buf[BUFFER_SIZE]; //在这个版本，buf大小必须大于或等于服务器端buf大小，不然会出错，想想为什么？
        memset(buf,0,sizeof(buf));

        scanf("%s",buf);
        //对于套接字，write和read默认是阻塞的！！！但可以通过设置文件描述符的属性为非阻塞模式来改变其行为，所以要根据需要设置阻塞还是非阻塞

        //当调用 write 函数时，如果写入的数据量超过了内核缓冲区的剩余空间大小，或者目标文件描述符对应的管道/套接字缓冲区已满，则 write 函数会阻塞，直到有足够的空间写入或其他条件满足。
        // 当调用 read 函数时，如果文件描述符对应的缓冲区没有数据可读（对于套接字/管道），或者文件描述符对应的文件已经读取到末尾，则 read 函数会阻塞，直到有数据可读或其他条件满足。
        ssize_t write_bytes=write(sockfd,buf,sizeof(buf)); //向服务器发送数据

        if(write_bytes==-1)
        {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }

        memset(buf,0,sizeof(buf));
        ssize_t read_bytes=read(sockfd,buf,sizeof(buf)); //// 从服务器接收数据
        if(read_bytes>0)
        {
            printf("message from server: %s\n",buf);
        }
        else if (read_bytes==0)
        {
            printf("server socket disconnected!\n");
            break;
        }
        else if(read_bytes==-1)
        {
            close(sockfd);
            errif(true,"socket read error");
        }
    }
    close(sockfd);
    return 0;
}

