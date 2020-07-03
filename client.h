#ifndef CLIENT_H
#define CLIENT_H

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
#include <netinet/in.h>
#include <arpa/inet.h>
const int SERVER_PORT = 4396;

const char SERVER_IP[] =  "127.0.0.1";

const int EPOLL_SIZE = 5000;

const int BUFF_SIZE = 0xFFFF;

const int TIMEOUT = -1;

const char EXIT[] = "exit";

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

class Client {
public:
    Client();

    ~Client();

    void Start();

    void Init();

    void Close();
    
private:
    struct sockaddr_in serverAddr_;

    int socketFd_;

    int epollFd_;

    int pid_;

    int pipeFd_[2];

    bool isClientWork;

    char message_[BUFF_SIZE];
};

#endif