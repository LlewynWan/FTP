/*ע��ͷ�ļ�˳��*/
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")      //���붯̬���ӿ�

#include"Client.h"

string comd[] = {
	"�г�Ŀ¼�ļ���Ϣ",
	"��ȡָ���ļ���С",
	"�ϴ��ļ�",
	"�����ļ�",
	"�ر����Ӳ��˳�"
};

void intro()
{
	cout << "\n����ʹ�õ�����:\n";
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


	//��¼
	string username, password;
	cout << "�������˻���:";
	cin >> username;
	cout << "����������:";
	cin >> password;
	client.setUsername(username);
	client.setPassword(password);
	res = client.login();
	while (res != PASSWORD_OKAY) {
		if (res == SOCKET_ERROR) {
			cout << "�������Ӵ���" << endl;
			break;
		}
		else {
			cout << "�˺Ż����������" << endl;
		}
		cout << "�������˻���:";
		cin >> username;
		cout << "����������:";
		cin >> password;
		client.setUsername(username);
		client.setPassword(password);
		res = client.login();
	}
	cout << "��¼�ɹ�\n";

/*
	client.setUsername("");
	client.setPassword("");
	client.login();*/

	do {
		char command;
		int size = 0;
		string listRes, fileName, dir;
		intro();
		cin >> command;
		switch (command)
		{
		case '1':
			//list����
			res = client.list(listRes);
			if (res != DATA_TRANSFER_COMPLETE) {
				cout << "�������ݳ���\n";
			}
			else {
				cout << "�г��ļ���Ϣ��\n";
				cout << listRes << endl;
			}
			break;
		case '2':
			//��ȡָ���ļ��Ĵ�С
			cout << "�������ļ���\n";
			cin >> fileName;
			size = client.fileSize(fileName);
			if (size != 0) {
				cout << fileName << "�ļ���СΪ" << size << endl;
			}
			break;
		case '3':
			//�ϴ��ļ�
			cout << "�����Ҫ������ļ�·��:";
			cin >> fileName;
			res = client.uploadFile(fileName);
			break;
		case '4':
			//�����ļ�
			cout << "������Ҫ���ص��ļ���:";
			cin >> fileName;
			cout << "������洢��Ŀ¼:";
			cin >> dir;
			res = client.downloadFile(fileName, dir);
			break;
		case '5':
			client.disconnect();
			cout << "���˳�" << endl;
			return 0;
			break;
		default:
			break;
		}
	} while (1);
}


int main()
{
	test();

	return 0;
}