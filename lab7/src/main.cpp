#pragma comment(lib,"ws2_32.lib")
#include"WebServer.h"

int main()
{
	Web_Server webserver;
	webserver.Begin();
	system("pause");
}