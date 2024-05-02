#pragma once
// 这两行代码是前向声明（forward declaration），用于在 EventLoop 类中声明 Epoll 类和 Channel 类，而不是包含它们的完整定义。这样做的目的是告诉编译器这两个类是存在的，但是在当前文件中并不提供它们的具体实现细节

// 因此，这两行代码的作用是为了在 EventLoop 类中声明 Epoll 类和 Channel 类的存在，而不是包含它们的完整定义，以避免头文件的循环依赖，并提高编译速度。
class Epoll;
class Channel;

class EventLoop
{
private:
    Epoll *ep;
    bool quit;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel *);
};

