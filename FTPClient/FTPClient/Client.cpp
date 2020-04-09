#include"Client.h"
#include<sstream>
#include<fstream>
#include<io.h>


using namespace std;

void Client::prepare()
{
	/*初始化winsock*/
	WORD ws_version = MAKEWORD(2, 2);   //指定Winsock version
	WSADATA wsaData;                    //WSA 函数的参数
	WSAStartup(ws_version, &wsaData);

	//设置服务器的地址
	serverCommandAddr.sin_family = AF_INET;
	serverCommandAddr.sin_port = htons(21);
	inet_pton(AF_INET, SERVER_ADDR, &serverCommandAddr.sin_addr);

	//配置命令传输的socket
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
			//输出服务器返回的响应
			cout << "message: " << message << endl;
			//获取请求码
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



//设置用户名和密码
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
进入被动模式
*/
int Client::enterPassiveMode()
{
	command = "PASV";
	code = sendCommand();
	if (code != 0) return code;
	recvMessage();
	if (code == SOCKET_ERROR) return code;

	//解析服务器发送的数据
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

	//设置数据连接的socket
	serverDataAddr.sin_family = AF_INET;
	serverDataAddr.sin_port = htons(256 * num4 + num5);
	inet_pton(AF_INET, SERVER_ADDR, &serverDataAddr.sin_addr);

	//cout << string("(" + num[0] + "." + num[1] + "." + num[2] + "." + num[3] + ")");

	if (connect(dataSocket, (SOCKADDR *)&serverDataAddr, sizeof(serverDataAddr)) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	return 0;
}

//建立和断开数据连接
int Client::dataConnect(int mode)
{
	if (mode == passiveMode) {
		return enterPassiveMode();
	}
	else {
		return 0;
	}

}
int Client::dataDisconnect()
{
	closesocket(dataSocket);
	return 0;
}

/*
列出当前目录下的文件信息
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

	//接收并且处理数据
	//由于这里不清楚不同服务器返回的信息会不会不同，不好解析
	recvData();
	str = message;
	
	//接收完成
	recvMessage();
	if (code != DATA_TRANSFER_COMPLETE) return code;

	dataDisconnect();
	return DATA_TRANSFER_COMPLETE;
}

/*
获取指定文件的大小
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
	else if (code==550){
		recvMessage();
		return -10;
	}
	else {
		return SOCKET_ERROR;
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


int Client::downloadFile(string fileName, string directory, bool append)
{
	int size = fileSize(fileName);
	int oldSize = 0;
	if (size == -10) {
		return -10;
	}

	//注意要使用二进制模式读取文件
	ofstream newFile;
	if (append == true) {
		newFile.open(directory + "//" + fileName, ios::app | ios::binary);

		//获取文件的大小
		newFile.seekp(0, ios::end);
		oldSize = newFile.tellp();
		
		if (oldSize <= size) {
			command = "REST " + to_string(oldSize);
			sendCommand();
			if (code == SOCKET_ERROR) {
				return code;
			}
			recvMessage();
		}
	}
	else newFile.open(directory + "//" + fileName, ios::trunc | ios::binary);
	
	//打开数据连接
	code = dataConnect(passiveMode);
	if (code == SOCKET_ERROR) return code;

	command = "RETR " + fileName;
	sendCommand();
	if (code == SOCKET_ERROR) {
		return code;
	}
	recvMessage();
	if (code != 125) {
		return code;
	}

	memset(recvbuf, 0, sizeof(recvbuf));
	//接收文件
	int remainLen = size - oldSize, len = 0, curIndex = 0;
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

	cout << "文件传输完毕\n";

	dataDisconnect();
	//关闭文件
	newFile.close();
	recvMessage();
	return code;
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


int Client::uploadFile(string filePath, bool append)
{
	//检测文件是否存在
	ifstream in(filePath, ios::binary);
	if (!in.good()) {
		dataDisconnect();
		return FILE_OPEN_ERROR;
	}
	//获取文件名
	string fileName = filePath.substr(filePath.find_last_of("/") + 1);

	//打开数据连接
	code = dataConnect(passiveMode);
	if (code == SOCKET_ERROR) {
		dataDisconnect();
		return code;
	}

	//判断是否使用断点续传
	command = "STOR " + fileName;
	if (append == true) {
		int size = fileSize(filePath);
		if (size > 0) {
			in.seekg(size);
			command = "APPE " + fileName;
		}
	}
	
	sendCommand();
	if (code == SOCKET_ERROR) {
		dataDisconnect();
		return code;
	}
	recvMessage();
	if (code != DATA_CONNECT) {
		dataDisconnect();
		return code;
	}

	memset(sendbuf, 0, sizeof(sendbuf));
	int curIndex = 0;
	int len = 0, lens = 0;
	while (!in.eof()) {
		curIndex = in.read(sendbuf, 1024).gcount();
		while ((len = send(dataSocket, sendbuf + lens, curIndex - lens, 0)) != SOCKET_ERROR && lens != curIndex) {
			if (len == SOCKET_ERROR) {
				return SOCKET_ERROR;
			}
			lens += len;
		}
		lens = 0;
	}

	dataDisconnect();
	recvMessage();
	return code;
}

int Client::quit()
{
	command = "QUIT";
	sendCommand();
	recvMessage();

	return 0;
}



