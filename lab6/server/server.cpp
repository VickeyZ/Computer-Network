#include <iostream>
#include <cstring>
#include <vector>
#include <mutex>
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFFER 256
using namespace std;

typedef pair<string, int> ip_port;

void HandleConnection(int cfd)
{
    char hello[] = "hello\n";
    send(cfd, hello, strlen(hello), 0);

    char RBuffer[MAXBUFFER] = {0};
    char SBuffer[MAXBUFFER] = {0};
    while (true)
    {
        recv(cfd, RBuffer, MAXBUFFER, 0);
        memset(SBuffer, 0, MAXBUFFER);
        mt.lock();
        switch (RBuffer[0])
        {
            case 0:
            {
                for (auto it = clientList.begin(); it != clientList.end(); )
                {
                    if ((*it).first == cfd)
                    {
                        it = clientList.erase(it);
                        break;
                    }
                    else
                        ++it;
                }
                break;
            }
            case 1:
            {
                time_t t;
                time(&t);
                cout<<cfd<<"GetTheTime: "<<'\n';
                SBuffer[0] = 11;
                sprintf(SBuffer + strlen(SBuffer), "%ld", t);
                send(cfd, SBuffer, strlen(SBuffer), 0);
                break;
            }
            case 2:
            {
                cout<<cfd<<"GetTheName: "<<'\n';
                SBuffer[0] = 12;
                gethostname(SBuffer + strlen(SBuffer), sizeof(SBuffer) - sizeof(char));
                send(cfd, SBuffer, strlen(SBuffer), 0);
                break;
            }
            case 3:
            {
                cout<<cfd<<"GetTheClients: "<<'\n';
                SBuffer[0] = 13;
                for (auto& it: clientList)
                {
                    sprintf(SBuffer + strlen(SBuffer), "%s", it.second.first.c_str());
                    sprintf(SBuffer + strlen(SBuffer), "^");
                    sprintf(SBuffer + strlen(SBuffer), "%d", it.second.second);
                    sprintf(SBuffer + strlen(SBuffer), "$");
                }
                send(cfd, SBuffer, strlen(SBuffer), 0);
                break;
            }
            case 4:
            {
                string msg(RBuffer + 1);
                size_t pos0 = msg.find("^"), pos1 = msg.find("$");
                string ip_addr = msg.substr(0, pos0);
                int port = atoi(msg.substr((pos0 + 1), pos1 - pos0 - 1).c_str());
                string content = msg.substr(pos1 + 1);
                int sock = -1;
                for (auto it = clientList.begin(); it != clientList.end(); ++it)
                {
                    if (it->second.first == ip_addr && it->second.second == port)
                    {
                        sock = it->first;
                        break;
                    }
                }
                cout<<cfd<<" Send message to "<<ip_addr<<":"<<port<<'\n';
                SBuffer[0] = 14;
                if (-1 == sock)
                    sprintf(SBuffer + 1, "Fail.\n");
                else
                {
                    SendMsg(sock, content);
                    sprintf(SBuffer + 1, "Success.\n");
                }
                send(cfd, SBuffer, strlen(SBuffer), 0);
                break;
            }
            case 5:
                break;
        }
        memset(RBuffer, 0, MAXBUFFER);
        mt.unlock();
    }
}

void SendMsg(int sock, const string& message)
{
    char msg[MAXBUFFER] = {0};
    msg[0] = 20;
    sprintf(msg + 1, "%s", message.c_str());
    send(sock, msg, strlen(msg), 0);
}


class SocketServer
{
private:
    int sockfd;
    sockaddr_in sin;

    static void* HandleThread(void* cfd)
    {
        HandleConnection(*(int*)cfd);
        return nullptr;
    }

    const int MaxConnection = 16;
public:
    SocketServer();
    ~SocketServer();
    void run();
};

vector<pair<int, ip_port>> clientList;
mutex mt;

int main()
{
    ios::sync_with_stdio(false);
    SocketServer server;
    server.run();
}

SocketServer::SocketServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(3770);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (sockaddr*)&sin, sizeof(sin));
    listen(sockfd, MaxConnection);
}

SocketServer::~SocketServer()
{
    close(sockfd);
}

void SocketServer::run()
{
    cout<<"Listening\n";
    while (true)
    {
        sockaddr_in client;
        unsigned int clientAddrLength = sizeof(client);
        int connection_fd = accept(sockfd, (sockaddr*)&client, (socklen_t*)&clientAddrLength);
        clientList.push_back(pair<int, ip_port>(connection_fd, ip_port(inet_ntoa(client.sin_addr), ntohs(client.sin_port))));
        cout<<inet_ntoa(client.sin_addr)<<":"<<ntohs(client.sin_port)<<" connected.\n";
        pthread_t connection_thread;
        pthread_create(&connection_thread, nullptr, HandleThread, &connection_fd);
    }
}



