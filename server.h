#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <netinet/in.h>
#include <arpa/inet.h>

const int SERVER_PORT = 4396;

const char SERVER_IP[] =  "127.0.0.1";

const int EPOLL_SIZE = 5000;

const int BUFF_SIZE = 0xFFFF;

const int TIMEOUT = -1;

static void AddSocketFdToEpoll(int socketFd, int epollFd, bool enableET) 
{
    struct epoll_event ev;
    ev.data.fd = socketFd;
    ev.events = EPOLLIN;
    if(enableET) 
    {
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &ev);

    //设为非阻塞
    fcntl(socketFd, F_SETFL, fcntl(socketFd, F_GETFD, 0) | O_NONBLOCK);

    //printf("SocketFd(%d) added to epoll(%d)...\n", socketFd, epollFd);
}

const char SERVER_WELCOME[] = "[Server]Welcome you join to the CharRoom!\n[Server]Your chat ID is: %d";
const char CLIENT_JOIN[] = "[Server]Welcome %d join to the CharRoom!";
const char SERVER_MESSAGE[] = "[%d]: %s";  
const char CAUTION[] =  "[Server]There is only one int the char room!";

class Server {
public:
    Server();

    ~Server();

    void init();

    void start();

    //void Connect();

    void Close();

    void CloseClientFd(const int clientFd);

    void SendBroadCastMessage(const int clientFd);
    
    void SendToAllClient(const char *message, const int clientFd = -1);
    
private:
    int socketFd_;

    struct sockaddr_in serverAddr_;

    int epollFd_;

    std::list<int> clientsList_;
};


#endif //SERVER_H