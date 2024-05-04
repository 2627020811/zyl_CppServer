#include"src/EventLoop.h"
#include"src/Server.h"

int main()
{
    EventLoop *loop=new EventLoop(); //事件驱动
    Server *server=new Server(loop);
    loop->loop();
    return 0;

}