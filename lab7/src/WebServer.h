#pragma once
#include<iostream>
#include<stdio.h>
#include<cstring>  
#include<string>
#include<winsock.h>  
#include<thread>
#include<cstdio>  
#include<cstdlib>  
#include<fstream>  
#include<windows.h>

#define SERVER_PORT 3770
#define BUF_SIZE 1024
#define QUEUE_SIZE 10
#define MAX 100

using namespace std;


struct Message
{
	char* MessageData;
	bool* Actived;
	int SocketClient;
	int Id;
	Message(char* data, bool* actived, int client, int id): MessageData(data), Actived(actived), SocketClient(client), Id(id) { }
};

struct Message_close
{
	bool* Actived;
	int SocketServer;
	Message_close(bool* actived, int server): Actived(actived), SocketServer(server) { }
};

class Web_Server
{
private:
	char buffer[BUF_SIZE];
	sockaddr_in serverChannel;
	thread* ThreadArray[MAX];
	thread* Thread;
	char CateDir[50];
	char usrname[50];
	bool Actived[MAX];
	int SocketServer;
	int SocketClient;
	friend void Handle_Message(Message msg);
	friend void SendError(Message msg);
	friend void CloseSignal(Message_close msg);
public:
	Web_Server()
	{
		//WinSock
		WORD wVersion;
		WSADATA WsaData;
		int flag;

		//Initialize WinSock 
		wVersion = MAKEWORD(2, 2);
		flag = WSAStartup(wVersion, &WsaData);
		if(flag)
			printf("WSAStartup() Failed!\n");

		if(LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wVersion) != 2)
		{
			WSACleanup();
			printf("Invalid Winsock version!\n");
		}
	}
	bool Begin();
};

