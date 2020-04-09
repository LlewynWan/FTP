/*注意头文件顺序*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include<iostream>
#include<string>
#pragma comment(lib, "ws2_32.lib")      //引入动态链接库

#define SERVER_ADDR "192.168.0.106"

#define CONNECT 220
#define USERNAME_OKAY 331
#define PASSWORD_OKAY 230
#define PASSIVE_MODE 227
#define DATA_CONNECT 125
#define DATA_TRANSFER_COMPLETE 226

using namespace std;

class Client
{
public:
	//开始与结束	
	void prepare();
	void disconnect();
	int connectServer();

	//登录
	void setPassword(string username);
	void setUsername(string password);
	int login();
	
	//数据连接
	int dataConnect(int mode);
	int dataDisconnect();
	//进入被动模式（使用被动模式进行数据传输后需要关闭数据连接，之后再重新开启）		
	int enterPassiveMode();


	//列出目录下的所有文件
	int list(string & str);
	
	//获取文件大小
	int fileSize(string fileName);

	//下载文件
	int downloadFile(string fileName, string directory=".");

	//上传文件
	int uploadFile(string fileName);

	//关闭连接
	int quit();


private:
	//用户名和密码
	string username;
	string password;

	//服务端命令socket地址
	SOCKADDR_IN serverCommandAddr;
	//服务端数据socket地址
	SOCKADDR_IN serverDataAddr;
	//用于传送命令的socket
	SOCKET commandSocket;
	//用于数据传输的socket
	SOCKET dataSocket;

	//发送与接收命令
	int sendCommand();
	int recvMessage();
	//发送与接收数据
	int recvData();
	int sendData();

	int code;
	string message;
	string command;
	
	char sendbuf[1024];
	char recvbuf[1024];

	static const int passiveMode = 0;
	static const int activeMode = 1;
};

