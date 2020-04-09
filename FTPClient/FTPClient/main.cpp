/*注意头文件顺序*/
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")      //引入动态链接库

#include"Client.h"

string comd[] = {
	"列出目录文件信息",
	"获取指定文件大小",
	"上传文件",
	"下载文件",
	"关闭连接并退出"
};

void intro()
{
	cout << "\n可以使用的命令:\n";
	for (int i = 0; i < 5; i++) {
		cout << "\t" << i + 1 << ". " << comd[i] << endl;
	}
}


int test()
{
	int res;
	Client client;
	client.prepare();
	res = client.connectServer();


	////登录
	//string username, password;
	//cout << "请输入账户名:";
	//cin >> username;
	//cout << "请输入密码:";
	//cin >> password;
	//client.setUsername(username);
	//client.setPassword(password);
	//res = client.login();
	//while (res != PASSWORD_OKAY) {
	//	if (res == SOCKET_ERROR) {
	//		cout << "网络连接错误" << endl;
	//		break;
	//	}
	//	else {
	//		cout << "账号或者密码错误" << endl;
	//	}
	//	cout << "请输入账户名:";
	//	cin >> username;
	//	cout << "请输入密码:";
	//	cin >> password;
	//	client.setUsername(username);
	//	client.setPassword(password);
	//	res = client.login();
	//}
	//cout << "登录成功\n";

	client.setUsername("anonymous");
	client.setPassword("");
	client.login();

	do {
		char command;
		int size = 0, mode;
		string listRes, fileName, dir;
		intro();
l:		cin >> command;
		switch (command)
		{
		case '1':
			//list命令
			res = client.list(listRes);
			if (res != DATA_TRANSFER_COMPLETE) {
				cout << "传输数据出错\n";
			}
			else {
				cout << "列出文件信息：\n";
				cout << listRes << endl;
			}
			break;
		case '2':
			//获取指定文件的大小
			cout << "请输入文件名\n";
			cin >> fileName;
			size = client.fileSize(fileName);
			if (size != -10) {
				cout << fileName << "文件大小为" << size << "字节" <<  endl;
			}
			break;
		case '3':
			//上传文件
			cout << "请输出要传入的文件路径:";
			cin >> fileName;
			size = client.fileSize(fileName.substr(fileName.find_last_of('/')));
			if (size != -10) {
				cout << "文件在服务端已经存在了，请选择覆盖文件还是断点续传（1.覆盖文件2.断点续传）:";
				int i; cin >> i;
				if (i == 1) {
					mode = false;
				}
				else if (i == 2) {
					mode = true;
				}
			}
			res = client.uploadFile(fileName, mode);  
			if (res != DATA_TRANSFER_COMPLETE) {
				cout << "文件传输失败：";
				if (res == FILE_OPEN_ERROR) {
					cout << "无法打开文件\n";
				}
				else if (res == SOCKET_ERROR) {
					cout << "网络连接失败\n";
				}
			}
			break;
		case '4':
			//下载文件
			cout << "请输入要下载的文件名:";
			cin >> fileName;
			cout << "请输入存储的目录:";
			cin >> dir;
			res = client.downloadFile(fileName, dir);
			if (res != DATA_TRANSFER_COMPLETE) {
				cout << "文件传输失败\n";
			}
			break;
		case '5':
			client.disconnect();
			cout << "已退出" << endl;
			return 0;
			break;
		default:
			goto l;
		}
	} while (1);
}


int main()
{
	test();

	return 0;
}