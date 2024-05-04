#pragma once
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll
{
private:
    /* data */
    int epfd;
    struct epoll_event *events;

public:
    Epoll(/* args */);
    ~Epoll();

    void addFd(int fd, uint32_t op);
    void updateChannel(Channel *);

    std::vector<Channel *> poll(int timeout = -1); //获取活跃的Channel
};
