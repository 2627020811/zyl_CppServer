#pragma once
#include <sys/epoll.h>

//Channel 将文件描述符与相应的事件处理逻辑（如读取、写入）绑定在一起，形成一个独立的事件处理单元。
//这样做的好处是可以将事件处理逻辑与业务逻辑分离，使代码更清晰、易读和易维护

class Epoll;

class Channel
{
    private:
        Epoll *ep;
        int fd;
        uint32_t events;
        uint32_t revents;
        bool inEpoll;
    public:
        Channel(Epoll *_ep,int _fd);
        ~Channel();

        void enableReading();
        
        int getFd();
        uint32_t getEvents();
        uint32_t getRevents();

        bool getInEpoll();
        void setInEpoll();

        //void setEvents(uint32_t)
        void setRevents(uint32_t);
};