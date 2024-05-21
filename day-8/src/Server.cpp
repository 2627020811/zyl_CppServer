#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include <functional>


Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr){ 
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){
    delete acceptor;
}


void Server::newConnection(Socket *sock){
    Connection *conn = new Connection(loop, sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1); //std::placeholders::_1 是 C++11 中提供的一个占位符，用于在使用 std::bind 绑定函数时指定参数的位置，待调用时再传入
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn;
}

void Server::deleteConnection(Socket * sock){
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}