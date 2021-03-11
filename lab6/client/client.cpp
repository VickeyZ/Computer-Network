#include <iostream>
#include <vector>
#include <regex>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFFER 256
#define NORMAL "\033[0m"
using namespace std;

mutex mt;

struct Message
{
    long type;
    char text[MAXBUFFER];
};

void HandleConnection(int cfd)
{
    char buffer[MAXBUFFER];
    recv(cfd, buffer, MAXBUFFER, 0);
    cout<<buffer<<NORMAL<<"> ";
    fflush(stdout);

    Message msg;
    key_t key = ftok("/",'a');
    int msqid = msgget(key, IPC_CREAT | 0666);
    while (true)
    {
        memset(buffer, 0, MAXBUFFER);
        recv(cfd, buffer, MAXBUFFER, 0);
        if (20 == buffer[0])
        {
            cout<<buffer + 1<<'\n';
            continue;
        }
        msg.type = buffer[0];
        strcpy(msg.text, buffer + 1);
        msgsnd(msqid, &msg, MAXBUFFER, 0);
    }
}

class SocketClient
{
private:
    int socketfd;
    sockaddr_in serverAddr;
    pthread_t ConnectionThread;
    int msqid;

    void help(); //显示输出界面
    void connect(const string& ip_addr, const int& port);

    static void* HandleThread(void* cfd)
    {
        HandleConnection(*(int*)cfd);
        return nullptr;
    }

public:
    SocketClient();
    ~SocketClient();
    
    void run();
};

SocketClient::SocketClient()
{
    socketfd = -1;
    key_t msgkey = ftok("/",'a');
    msqid = msgget(msgkey, IPC_CREAT | 0666);
}

SocketClient::~SocketClient()
{
    close(socketfd);
}

void SocketClient::run()
{
    help();
    while (true)
    {
        cout<<"> ";
        string line;
        getline(cin, line);
        regex whitespace("\\s+");
        vector<string> inputs(sregex_token_iterator(line.begin(), line.end(), whitespace, -1),
                                        sregex_token_iterator());
        if (inputs[0] == "")
            continue;
        else if (inputs[0] == "Connect")
        {
            if (-1 != socketfd)
            {
                cout<<"Connected!\n"<<NORMAL;
            }
            else
            {
                connect(inputs[1], stoi(inputs[2]));
            }
        }
        else if (inputs[0] == "Close")
        {
            if (-1 == socketfd)
                cout<<"No connection.\n"<<NORMAL;
            else
            {
                char buffer = 50;
                send(socketfd, &buffer, sizeof(buffer), 0);
                mt.lock();
                pthread_cancel(ConnectionThread);
                mt.unlock();
                close(socketfd);
                socketfd = -1;
                cout<<"Connection closed.\n"<<NORMAL;
            }
        }
        else if (inputs[0] == "GetTheTime")
        {
            char buffer = 1;
            Message timemsg;
            send(socketfd, &buffer, sizeof(buffer), 0);
            msgrcv(msqid, &timemsg, MAXBUFFER, 11, 0);
            time_t time = atol(timemsg.text);
            cout<<" Server time: "<<ctime(&time)<<NORMAL;
            
        }
        else if (inputs[0] == "GetTheName")
        {
            char buffer = 2;
            send(socketfd, &buffer, sizeof(buffer), 0);
            Message namemsg;
            msgrcv(msqid, &namemsg, MAXBUFFER, 12, 0);
            cout<<"Server host name: "<<namemsg.text<<'\n'<<NORMAL;
        }
        else if (inputs[0] == "GetTheClients")
        {
            char buffer = 3;
            send(socketfd, &buffer, sizeof(buffer), 0);
            Message peermsg;
            msgrcv(msqid, &peermsg, MAXBUFFER, 13, 0);
            
            char* ptr = peermsg.text;
            int count = 0, flag = 1;
            while (*ptr)
            {
                if ('^' == *ptr)
                    cout<<' ';
                else if ('$' == *ptr)
                {
                    cout<<'\n';
                    flag = 1;
                }
                else
                {
                    if (flag)
                    {
                        cout<<count++<<' ';
                        flag = 0;
                    }
                    cout<<(*ptr);
                }
                ptr++;
            }
            cout<<NORMAL;
        }
        else if (inputs[0] == "Send")
        {
            char buffer[MAXBUFFER] = {0};
            buffer[0] = 4;
            sprintf(buffer + strlen(buffer), "%s", inputs[1].c_str());
            sprintf(buffer + strlen(buffer), "^");
            sprintf(buffer + strlen(buffer), "%s", inputs[2].c_str());
            sprintf(buffer + strlen(buffer), "$");
            for (int i = 3; i < inputs.size(); ++i)
            {
                sprintf(buffer + strlen(buffer), "%s", inputs[i].c_str());
                if (i != inputs.size())
                {
                    sprintf(buffer + strlen(buffer), " ");
                }
                else
                {
                    sprintf(buffer + strlen(buffer), "\n");
                }
            }
            send(socketfd, buffer, MAXBUFFER, 0);
            Message statusmsg;
            msgrcv(msqid, &statusmsg, MAXBUFFER, 14, 0);
            cout<<statusmsg.text<<NORMAL;
        }
        else if (inputs[0] == "Receive")
        {
            char buffer[MAXBUFFER] = {0};
            buffer[0] = 5;
            sprintf(buffer + strlen(buffer), "receive");
            send(socketfd, buffer, MAXBUFFER, 0);
        }
        else if (inputs[0] == "Exit")
        {
            if (-1 != socketfd)
            {
                char buffer = 50;
                send(socketfd, &buffer, sizeof(buffer), 0);
                close(socketfd);
                cout<<"Socket "<<socketfd<<" closed.\n"<<NORMAL;
            }
            cout<<"Bye.\n"<<NORMAL;
            exit(0);
        }
        else if (inputs[0] == "Help")
        {
            help();
        }
        else
        {
            cout<<"Fatal input!\n";
        }
    }
}

void SocketClient::connect(const string& ip_addr, const int& port)
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
    ::connect(socketfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    pthread_create(&ConnectionThread, nullptr, HandleThread, &socketfd);
}

void SocketClient::help()
{
   std::cout<<"---------START-----------\n"
            <<"Please input the command:\n"
            <<"[1] Connect [IP] [port]\n"
            <<"[2] Close\n"
            <<"[3] GetTheTime\n"
            <<"[4] GetTheName\n"
            <<"[5] GetTheClients\n"
            <<"[6] Send [IP] [port] [content]\n"
            <<"[7] Exit\n"
            <<"[8] Help\n"
            <<"----------END------------\n";
}

int main()
{
    ios::sync_with_stdio(false);
    SocketClient client;
    client.run();
}