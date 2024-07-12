#include <unistd.h>
#include <cstring>

#include <functional>
#include <iostream>

#include "Buffer.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "util.h"

void OneClient(int msgs, int wait)
{
    Socket *sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);

    sock->Connect(addr);

    int sockfd = sock->GetFd();

    Buffer *send_buffer = new Buffer();
    Buffer *read_buffer = new Buffer();

    sleep(wait);
    int count = 0;

    while (count < msgs)
    {
        send_buffer->SetBuf("I'm client!");
        ssize_t write_bytes = write(sockfd, send_buffer->ToStr(), send_buffer->Size());
        if (write_bytes == -1)
        {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }

        int already_read = 0;
        char buf[1024]; // 这个buf大小无所谓
        while (true)
        {
            memset(&buf, 0, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if (read_bytes > 0)
            {
                read_buffer->Append(buf, read_bytes);
                already_read += read_bytes;
            }
            else if (read_bytes == 0)
            { // EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if (already_read >= send_buffer->Size())
            {
                printf("count: %d, message from server: %s\n", count++, read_buffer->ToStr());
                break;
            }
        }
        read_buffer->Clear();
    }

    delete addr;
    delete sock;
    delete send_buffer;
    delete read_buffer;
}

int main(int argc, char *argv[])
{
    int threads = 100;
    int msgs = 100;
    int wait = 0;
    int o = -1;

    const char *optstring = "t:m:w:";
    while ((o = getopt(argc, argv, optstring)) != -1) // getopt() 是一个 C 函数，用于解析命令行选项和参数。它允许程序接收和处理命令行输入的选项（通常是标志）和相应的参数
    {
        switch (o)
        {
        case 't':
            threads = std::stoi(optarg);
            break;
        case 'm':
            msgs = std::stoi(optarg);
            break;
        case 'w':
            wait = std::stoi(optarg);
            break;
        case '?':
            printf("error optopt: %c\n", optopt);
            printf("error opterr: %d\n", opterr);
            break;
        default:
            break;
        }
    }

    ThreadPool *poll = new ThreadPool(threads);
    std::function<void()> func = std::bind(OneClient, msgs, wait);
    for (int i = 0; i < threads; ++i)
    {
        poll->Add(func);
    }
    delete poll;
    return 0;
}
