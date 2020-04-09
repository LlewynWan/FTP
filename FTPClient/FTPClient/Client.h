/*ע��ͷ�ļ�˳��*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include<iostream>
#include<string>
#pragma comment(lib, "ws2_32.lib")      //���붯̬���ӿ�

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
	//��ʼ�����	
	void prepare();
	void disconnect();
	int connectServer();

	//��¼
	void setPassword(string username);
	void setUsername(string password);
	int login();
	
	//��������
	int dataConnect(int mode);
	int dataDisconnect();
	//���뱻��ģʽ��ʹ�ñ���ģʽ�������ݴ������Ҫ�ر��������ӣ�֮�������¿�����		
	int enterPassiveMode();


	//�г�Ŀ¼�µ������ļ�
	int list(string & str);
	
	//��ȡ�ļ���С
	int fileSize(string fileName);

	//�����ļ�
	int downloadFile(string fileName, string directory=".");

	//�ϴ��ļ�
	int uploadFile(string fileName);

	//�ر�����
	int quit();


private:
	//�û���������
	string username;
	string password;

	//���������socket��ַ
	SOCKADDR_IN serverCommandAddr;
	//���������socket��ַ
	SOCKADDR_IN serverDataAddr;
	//���ڴ��������socket
	SOCKET commandSocket;
	//�������ݴ����socket
	SOCKET dataSocket;

	//�������������
	int sendCommand();
	int recvMessage();
	//�������������
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

