#include <iostream>
#include "server.h"

using namespace std;

Server::Server()
{
	serverAddr_.sin_family = AF_INET;
	serverAddr_.sin_port = htons(SERVER_PORT);
	serverAddr_.sin_addr.s_addr = inet_addr(SERVER_IP);
	socketFd_ = 0;
	epollFd_ = 0;
}

Server::~Server()
{
	clientsList_.clear();
	Close();
}

void Server::start()
{
	static struct epoll_event events[EPOLL_SIZE];
	init();
	int ret;
	while(true)
	{
		int eventsCnt = epoll_wait(epollFd_,events,EPOLL_SIZE,TIMEOUT);
		if(eventsCnt<0)
		{
			perror("Epoll events error!");
			break;
		}
		
		for(int i = 0;i<eventsCnt;++i)
		{
			int event_fd = events[i].data.fd;
			//有新的连接
			if(event_fd == socketFd_)
			{
				struct sockaddr_in clientAddr;
				socklen_t addrlen = sizeof(struct sockaddr);
				int clientFd = accept(socketFd_,(struct sockaddr*)&clientAddr,&addrlen);
				cout<<"client from: "<<inet_ntoa(clientAddr.sin_addr)<<" : "
					<<ntohs(clientAddr.sin_port) << ", clientFd: "<< clientFd << endl;
				
				AddSocketFdToEpoll(clientFd, epollFd_, true);
                clientsList_.push_back(clientFd);
                cout << "Add newClinent : " << clientFd << " to epoll..." << endl;
                cout << "Now there are " << clientsList_.size() << " clients in the ChatRoom..." << endl;

                char message[BUFF_SIZE];
                bzero(message, BUFF_SIZE);
                sprintf(message, SERVER_WELCOME, clientFd);
                ret = send(clientFd, message, BUFF_SIZE, 0);
                if (ret < 0)
                {
                    perror("Send message to error!...");
                    CloseClientFd(clientFd);
                    exit(-1);
                }
                bzero(message, BUFF_SIZE);
                sprintf(message, CLIENT_JOIN, clientFd);
                SendToAllClient(message, clientFd);
			}
			else//有消息
			{
				SendBroadCastMessage(event_fd);//接受并广播
			}
		}
	}
	Close();

}

void Server::init()
{
	//socket基本操作
	int ret;
	cout<<"start server..."<<endl;
	//创建套接字
	socketFd_ = socket(AF_INET,SOCK_STREAM,0);
	if(socketFd_<0)
	{
		perror("create socket error!");
		exit(-1);
	}
	//绑定端口
    ret = bind(socketFd_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_));
    if (ret < 0)
    {
        perror("bind socket error!");
        exit(-1);
    }
	//监听
    ret = listen(socketFd_, 6);
    if (ret < 0)
    {
        perror("listen socket error!");
        exit(-1);
    }
    cout << "Start to listen: " << SERVER_IP << ":" << SERVER_PORT << endl;
    //内核中创建事件
    epollFd_ = epoll_create(EPOLL_SIZE);
    if (epollFd_ < 0)
    {
        perror("create epoll error!");
        exit(-1);
    }
	
    //添加监听事件
    AddSocketFdToEpoll(socketFd_, epollFd_, true);
}

void Server::SendBroadCastMessage(const int clientFd)
{
    char buff[BUFF_SIZE], message[BUFF_SIZE];
    bzero(buff, BUFF_SIZE);
    bzero(message, BUFF_SIZE);

    int len = recv(clientFd, buff, BUFF_SIZE, 0);
    if (len == 0)
    {
        CloseClientFd(clientFd);
    }
    else
    {
        if (clientsList_.size() == 1)
        {
            // 发送提示消息
            send(clientFd, CAUTION, strlen(CAUTION), 0);
            return;
        }

        sprintf(message, SERVER_MESSAGE, clientFd, buff);
        cout << message << endl;
        SendToAllClient(message, clientFd);
    }
    return ;
}


void Server::SendToAllClient(const char *message, const int clientFd)
{
    int ret = -1;
    for (auto &item : clientsList_)
    {
        if (item == clientFd)
        {
            continue;
        }
        ret = send(item, message, BUFF_SIZE, 0);
        if (ret < 0)
        {
            perror("Send broadCast message error!...");
            CloseClientFd(item);
        }
    }
    return ;
}

void Server::Close()
{
    close(socketFd_);
    close(epollFd_);
    cout << "Server Closed!..." << endl;
}

void Server::CloseClientFd(const int clientFd)
{
    close(clientFd);
    clientsList_.remove(clientFd);

    cout << "Client:" << clientFd
         << " quited!...\nNow there are "
         << clientsList_.size()
         << " client in the char room"
         << endl;
}

int main()
{
    Server server;
    server.start();
	return 0;
}