#include"Client.h"
#include<sstream>
#include<fstream>

using namespace std;

void Client::prepare()
{
	/*��ʼ��winsock*/
	WORD ws_version = MAKEWORD(2, 2);   //ָ��Winsock version
	WSADATA wsaData;                    //WSA �����Ĳ���
	WSAStartup(ws_version, &wsaData);

	//���÷������ĵ�ַ
	serverCommandAddr.sin_family = AF_INET;
	serverCommandAddr.sin_port = htons(21);
	inet_pton(AF_INET, SERVER_ADDR, &serverCommandAddr.sin_addr);

	//����������socket
	commandSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}


void Client::disconnect()
{
	closesocket(commandSocket);
	WSACleanup();
}


int Client::connectServer()
{
	if (connect(commandSocket, (SOCKADDR *)&serverCommandAddr, sizeof(serverCommandAddr)) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}
	else {
		return recvMessage();
	}
}


int Client::sendCommand()
{
	int len = command.size();
	for (int i = 0; i < len; i++) {
		sendbuf[i] = command[i];
	}
	sendbuf[len++] = '\r';
	sendbuf[len++] = '\n';

	cout << "command:" << command << endl;

	int sendLen = 0;
	while (sendLen < len) {
		int r = send(commandSocket, sendbuf+sendLen, len-sendLen, 0);
		if (r == SOCKET_ERROR) {
			return (code = SOCKET_ERROR);
		}
		sendLen += r;
	}
	return 0;
}

int Client::recvMessage()
{
	message = "";

	int curIndex = 0;
	int len;
	while ((len = recv(commandSocket, recvbuf + curIndex, sizeof(char) * (1024 - curIndex), 0)) != SOCKET_ERROR) {
		curIndex += len;
		if (recvbuf[curIndex - 1] == '\n' && recvbuf[curIndex - 2] == '\r') {
			for (int i = 0; i < curIndex - 2; i++) {
				message.push_back(recvbuf[i]);
			}
			//������������ص���Ӧ
			cout << "message: " << message << endl;
			//��ȡ������
			code = 0;
			int i = 0;
			while (isdigit(message[i])) {
				code *= 10;
				code += message[i] - '0';
				i++;
			}
			return code;
		}
	}
	return (code = SOCKET_ERROR);
}

int Client::sendData()
{
	return 0;
}
int Client::recvData()
{
	message = "";

	int curIndex = 0;
	int len;
	while ((len = recv(dataSocket, recvbuf + curIndex, sizeof(char) * (1024-curIndex), 0)) != SOCKET_ERROR) {
		curIndex += len;
		if (recvbuf[curIndex - 1] == '\n' && recvbuf[curIndex - 2] == '\r') {
			for (int i = 0; i < curIndex - 2; i++) {
				message.push_back(recvbuf[i]);
			}
			return 0;
		}
	}
	return SOCKET_ERROR;
}



//�����û���������
void Client::setUsername(string username)
{
	this->username = username;
}
void Client::setPassword(string password)
{
	this->password = password;
}

int Client::login()
{	
	command = "USER " + username;
	sendCommand();
	if (code==SOCKET_ERROR) return code;
	recvMessage();
	if (code != USERNAME_OKAY) return code;

	command = "PASS " + password;
	sendCommand();
	if (code == SOCKET_ERROR) return code;
	recvMessage();
	if (code != PASSWORD_OKAY && code != SOCKET_ERROR) {
		recvMessage();
		return 530;
	}
	return PASSWORD_OKAY;
}


/*
���뱻��ģʽ
*/
int Client::enterPassiveMode()
{
	command = "PASV";
	sendCommand();
	recvMessage();

	//�������������͵�����
	string num[6];
	int i = 0, j = 0;
	while (message[i] != '(') i++;
	i++;
	int addrStart = i;
	while (1) {
		if (message[i] == ',' || message[i] == ')') {
			num[j++] = message.substr(addrStart, i - addrStart);
			addrStart = i + 1;
			if (message[i] == ')') break;
		}
		i++;
	}
	int num4 = 0, num5 = 0;
	for (int i = 0; i < num[4].size(); i++) {
		num4 *= 10;
		num4 += num[4][i] - '0';
	}
	for (int j = 0; j < num[5].size(); j++) {
		num5 *= 10;
		num5 += num[5][j] - '0';
	}
	//cout << num4 << ' ' << num5 << endl;

	/*for (int i = 0; i < 6; i++) {
		cout << num[i] << endl;
	}*/
	
	dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//�����������ӵ�socket
	serverDataAddr.sin_family = AF_INET;
	serverDataAddr.sin_port = htons(256 * num4 + num5);
	inet_pton(AF_INET, SERVER_ADDR, &serverDataAddr.sin_addr);

	//cout << string("(" + num[0] + "." + num[1] + "." + num[2] + "." + num[3] + ")");

	if (connect(dataSocket, (SOCKADDR *)&serverDataAddr, sizeof(serverDataAddr)) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	return 0;
}

//�����ͶϿ���������
int Client::dataConnect(int mode)
{
	return enterPassiveMode();
}
int Client::dataDisconnect()
{
	closesocket(dataSocket);
	return 0;
}

/*
�г���ǰĿ¼�µ��ļ���Ϣ
*/
int Client::list(string & str)
{
	dataConnect(passiveMode);
	if (code != PASSIVE_MODE) {
		return code;
	}

	command = "LIST";
	sendCommand();
	if (code == SOCKET_ERROR) return code;
	recvMessage(); 
	if (code != DATA_CONNECT) return code;

	//���ղ��Ҵ�������
	//�������ﲻ�����ͬ���������ص���Ϣ�᲻�᲻ͬ�����ý���
	recvData();
	str = message;
	
	//�������
	recvMessage();
	if (code != DATA_TRANSFER_COMPLETE) return code;

	dataDisconnect();
	return DATA_TRANSFER_COMPLETE;
}

/*
��ȡָ���ļ��Ĵ�С
*/
int Client::fileSize(string fileName)
{
	command = "SIZE " + fileName;
	sendCommand();
	recvMessage();


	if (code == 213) {
		stringstream ss(message);
		int size;
		ss >> size; 
		ss >> size; 
		return size;
	}
	else {
		return 0;
	}
}


void writeFile(string fileName, char buf[], int len)
{
	ofstream out(fileName, ios::app);
	for (int i = 0; i < len; i++) {
		out << buf[i];
	}
	out.close();
}


int Client::downloadFile(string fileName, string directory)
{
	int size = fileSize(fileName);

	dataConnect(passiveMode);

	//���ȴ������ļ�
	//ע��Ҫʹ�ö�����ģʽ��ȡ�ļ�
	ofstream newFile(directory + "//" + fileName, ios::trunc | ios::binary);

	command = "RETR " + fileName;
	sendCommand();
	recvMessage();

	memset(recvbuf, 0, sizeof(recvbuf));
	//�����ļ�
	int remainLen = size, len = 0, curIndex = 0;
	while ( remainLen > 0 ) {
		len = recv(dataSocket, recvbuf + curIndex, sizeof(char) * (1024 - curIndex), 0);
		if (len == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		remainLen -= len;
		curIndex += len;

		if (curIndex == 1024) {
			newFile.write(recvbuf, curIndex);
			curIndex = 0;
		}
	}
	newFile.write(recvbuf, curIndex);

	//�ر��ļ�
	newFile.close();
}	

void readFile(string fileName)
{
	char buf[1024];
	fill(buf, buf + 1024, 0);
	ifstream in(fileName);
	in.read(buf, 1024);
	cout << buf;

	cout << strlen(buf);
}


int Client::uploadFile(string fileName)
{
	dataConnect(passiveMode);
	
	cout << "�����ļ���" << endl;
	command = "STOR " + fileName;
	sendCommand();
	recvMessage();

	cout << "��Ҫ���͵��ļ�" << endl;
	ifstream in(fileName);
	memset(sendbuf, 0, sizeof(sendbuf));
	int curIndex = 0;
	int len = 0, lens = 0;
	while (!in.eof()) {
		curIndex = in.read(sendbuf, 1024).gcount();
		while ((len = send(dataSocket, sendbuf + lens, curIndex - lens, 0)) != SOCKET_ERROR && lens != curIndex) {
			lens += len;
		}
		lens = 0;
	}

	dataDisconnect();
	recvMessage();

	return 0;
}


int Client::quit()
{
	command = "QUIT";
	sendCommand();
	recvMessage();

	return 0;
}


