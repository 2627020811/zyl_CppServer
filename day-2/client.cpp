#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include"util.h"
int main()
{
    //客户端代码和服务器代码几乎一样：创建一个socket文件描述符，与一个IP地址和端口绑定，最后并不是监听这个端口，而是使用connect函数尝试连接这个服务器。
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    errif(sockfd==-1,"socket create error");
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port=htons(8888);
    errif(connect(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))==-1,"socket connect error");

    //以下逻辑和服务端一致
    while(true)
    {
        char buf[1024];
        memset(buf,0,sizeof(buf));

        scanf("%s",buf);

        ssize_t write_bytes=write(sockfd,buf,sizeof(buf)); //ssize_t 是一个用于表示有符号大小的数据类型
        if(write_bytes==-1) //表示发生错误
        {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }

        memset(buf,0,sizeof(buf)); //清空缓冲区
        ssize_t read_bytes=read(sockfd,buf,sizeof(buf));//从服务器socket读到缓冲区，返回已读数据大小
        if(read_bytes>0)
        {
            printf("message from server: %s\n",buf);
        }
        else if(read_bytes==0)
        {
            printf("server socket disconnected!\n");
            break;
        }
        else if (read_bytes==-1)
        {
            close(sockfd);
            errif(true,"socket read error");
        }
        
    }
    return 0;
}