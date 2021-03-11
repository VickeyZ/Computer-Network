#define _CRT_SECURE_NO_WARNINGS  
#include"WebServer.h"


void SendError(Message msg)
{
	char Protocol[] = "HTTP/1.0 400 Bad Request \r\n";
	char ServerName[] = "Server:My Web Server \r\n";
	char Length[] = "Content-Length:2048 \r\n";
	char Type[] = "Content-Type:text\\html \r\n\r\n";
	char Content[] = "<html><head><title>MY-WEB</title></head><font size=+5><br>find ERROR!</font></html>";

	send(msg.SocketClient, Protocol, strlen(Protocol), 0);
	send(msg.SocketClient, ServerName, strlen(ServerName), 0);
	send(msg.SocketClient, Length, strlen(Length), 0);
	send(msg.SocketClient, Type, strlen(Type), 0);
	send(msg.SocketClient, Content, strlen(Content), 0);
	closesocket(msg.SocketClient);

}

void Send_Message(string dir, Message msg)
{
	FILE* Filename;
	int sp;
	if((Filename = fopen(dir.c_str(), "rb")) == NULL)
	{
		SendError(msg);
		return;
	}
	else 
	{
		strcpy(msg.MessageData, "HTTP/1.1 200 OK\n");
		strcat(msg.MessageData, "Content-Type: text/html;charset=ISO-8859-1\nContent-Length: ");
		fseek(Filename, 0, SEEK_END);
		int Filelen = ftell(Filename);
		rewind(Filename);
		char* buf = (char*)malloc(Filelen * sizeof(char));
		char length[10];   
		_itoa(Filelen, length, 10);
		strcat(msg.MessageData, length);
		strcat(msg.MessageData, "\n\n");
		int res = send(msg.SocketClient, msg.MessageData, strlen(msg.MessageData), 0);
		if(res == SOCKET_ERROR) 
		{
			cout<<"MessageSend Failed"<<endl;
			*msg.Actived = false;
			return;
		}
		cout<<"MessageSend Succeed"<<endl;

		fread(buf, 1, Filelen, Filename);
		res = send(msg.SocketClient, buf, Filelen, 0);
		if(res == SOCKET_ERROR) 
		{
			cout<<"MessageSend Failed"<<endl;
			*msg.Actived = false;
			return;
		}
	}
}

void Handle_Message(Message msg)
{
	int i = 0, cnt = 0;
	bool flag = false;
	bool post_flag = false;
	string str = "";
	string type = "";
	string MessageData = "";

	if(msg.MessageData == "" || msg.MessageData == "\n") 
	{
		*msg.Actived = false;
		return;
	}
	//GET
	if(msg.MessageData[0] == 'G') 
	{   
		type = "GET";
		MessageData = msg.MessageData + 4;
		int pos = MessageData.find(' ');
		MessageData = MessageData.substr(0, pos);
	}
	//POST
	if(msg.MessageData[0] == 'P') 
	{   
		type = "POST";
		MessageData = msg.MessageData + 5;
		int pos = MessageData.find(' ');
		MessageData = MessageData.substr(0, pos);
	}

	if(type == "POST") 
	{ 
		bool login_flag = false;
		bool pass_flag = false;
		string usrname = "";
		string password = "";
		string msg_MessageData(msg.MessageData);
		int pos_login = msg_MessageData.find("login");
		int pos_pwd = msg_MessageData.find("pass");
		usrname = msg_MessageData.substr(pos_login + 6, pos_pwd - pos_login - 7);
		password = msg_MessageData.substr(pos_pwd + 5);
		if(usrname == "3770" && password == "3770") 
		{
			char respon[200];
			strcpy(respon, "<html><body>Welcome,");
			strcat(respon, usrname.c_str());
			strcat(respon, "!</body></html>\n");
			int len = strlen(respon);
			char length[20];
			sprintf(length, "%d", len);
			strcpy(msg.MessageData, "HTTP/1.1 200 OK\n");
			strcat(msg.MessageData, "Content-Type: text/html;charset=gb2312\nContent-Length: ");
			strcat(msg.MessageData, length);
			strcat(msg.MessageData, "\n\n");
			strcat(msg.MessageData, respon);
			cout<<"Login Succeed!"<<endl;

			int res = send(msg.SocketClient, msg.MessageData, 10000, 0);
			if(res == SOCKET_ERROR) 
			{
				cout<<"MessageSend Failed"<<endl;
				*msg.Actived = false;
				return;
			}
			cout<<"MessageSend Succeed"<<endl;
			*msg.Actived = false;
			return;
		}
		else 
		{
			char respon[200];
			strcpy(respon, "<html><body>Info Incorrect</body></html>\n");
			int len = strlen(respon);
			char length[20];
			sprintf(length, "%d", len);
			strcpy(msg.MessageData, "HTTP/1.1 200 OK\n");
			strcat(msg.MessageData, "Content-Type: text/html;charset=gb2312\nContent-Length: ");
			strcat(msg.MessageData, length);
			strcat(msg.MessageData, "\n\n");
			strcat(msg.MessageData, respon);
			cout<<"Login Failed!"<<endl;
			int res = send(msg.SocketClient, msg.MessageData, 10000, 0);

			if(res == SOCKET_ERROR) 
			{
				cout<<"MessageSend Failed"<<endl;
				*msg.Actived = false;
				return;
			}
			cout<<"MessageSend Succeed"<<endl;
			*msg.Actived = false;
			return;
		}

		*msg.Actived = false;
		return;
	}
	else if(type == "GET" && MessageData != "") 
	{
		memset(msg.MessageData, 0, sizeof(msg.MessageData));
		if(MessageData.substr(0, 5) == "/dir/") 
		{
			string str = "";
			string str1 = "";
			string password;
			string usrname;
			string dir;

			bool txt_flag = false;
			int pos = MessageData.find('.');
			str = MessageData.substr(pos + 1);
			if(str == "") 
			{
				*msg.Actived = false;
				return;
			}
			if(str == "txt") 
				dir = "src/txt/" + MessageData.substr(5);
			else if(str == "html") 
				dir = "src/html/" + MessageData.substr(5);

			Send_Message(dir, msg);
		}
		else if(MessageData.substr(0, 5) == "/img/") 
		{
			string dir = "src/img/" + MessageData.substr(5);
			Send_Message(dir, msg);
		}

	}
	closesocket(msg.SocketClient);
	*msg.Actived = false;
}

void CloseSignal(Message_close msg)
{
	string str;
	while(true) 
	{
		cin >> str;
		if(str == "quit")
		{
			while(true) 
			{
				int flag = 1;
				for(int i = 0; i < MAX; i++) 
				{
					if(msg.Actived[i]) 
					{
						flag = 0;
						break;
					}
				}
				if(flag) 
				{
					closesocket(msg.SocketServer);
					exit(0);
				}
			}
		}
		else
			cout<<"Syntex Error!"<<endl;
	}
}

bool Web_Server::Begin()
{
	int on = 1;
	memset(Actived, false, sizeof(Actived));
	Message_close msg(Actived, SocketServer);
	Thread = new std::thread(CloseSignal, msg);

	//Initialize
	memset(&serverChannel, 0, sizeof(serverChannel));
	serverChannel.sin_family = AF_INET;
	serverChannel.sin_addr.s_addr = htonl(INADDR_ANY);
	serverChannel.sin_port = htons(SERVER_PORT);


	SocketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(SocketServer < 0) 
	{
		cout<<"Socket Create Failed"<<endl;
		return false;
	}
	cout<<"Socket Create Succeed"<<endl;
	setsockopt(SocketServer, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

	//Bind
	int b = bind(SocketServer, (sockaddr*)&serverChannel, sizeof(serverChannel));
	if(b < 0) 
	{
		cout<<"Bind Failed"<<endl;
		return false;
	}
	cout<<"Bind Succeed"<<endl;

	//Listen
	int l = listen(SocketServer, 1);
	if(l < 0) 
	{
		cout<<"Listen failed"<<endl;
		return false;
	}
	cout<<"Listen Succeed"<<endl;

	int len = sizeof(serverChannel);

	while(1) 
	{
		cout<<"\nWaiting For Connection..."<<endl;

		//Accept
		SocketClient = accept(SocketServer, (sockaddr*)&serverChannel, &len);
		if(SocketClient < 0) 
			cout<<"Connect Failed"<<endl;
		else 
		{
			cout<<"Connect Succeed"<<endl;
			memset(buffer, 0, sizeof(buffer));

			int ret;
			ret = recv(SocketClient, buffer, BUF_SIZE, 0);
			if(ret == SOCKET_ERROR)
				cout<<"MessageSend Failed"<<endl;
			else if(ret == 0)
				cout<<"The Client Socket is Closed"<<endl;
			else 
			{
				cout<<"MessageSend Succeed"<<endl;
				for (int i = 0; i < MAX; i++) 
				{
					if(!Actived[i]) 
					{
						Actived[i] = true;
						Message msg(buffer, &Actived[i], SocketClient, i);
						thread temp(&Handle_Message, ref(msg));
						ThreadArray[i] = &temp;
						ThreadArray[i]->join();
						break;
					}
				}
			}
		}
	}
}