#include "Distribute.h"



Distribute::Distribute()
{
}

void Distribute::Run()
{
}

void Distribute::Init()
{
}

int Distribute::InitSocket()
{
	//�����׽��ֿ�
	WORD wVersionRequested;
	//����ṹ�������洢��WSAStartup�������ú󷵻ص�Windows Sockets���ݡ�������Winsock.dllִ�е����ݡ�
	WSADATA wsaData;
	int err;
	//��������1.1�汾��winsock��MAKEWORD(2,2)��2.2�汾
	wVersionRequested = MAKEWORD(1, 1);
	//����WSA
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return FALSE;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return FALSE;
	}
	return 0;
}

void Distribute::GetConfig()
{
	GetIps();
	port = 8989;
	isReceiving = false;
	isAnalysing = false;
	auto path = QDir::currentPath() + "/config.config";
	QFile file(path);
	if (file.exists()) {
		auto portstr = file.readAll().data();
		port = atoi(portstr);
	}
}


Distribute::~Distribute()
{
}

void Distribute::GetIps()
{
	for each (QHostAddress address in QNetworkInterface().allAddresses())
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol)
		{
			QString ip = address.toString();
			ips.push_back(ip.toStdString());
		}
	}
}

int Distribute::InitStationSocket()
{
	return 0;
}

int Distribute::CreateStationSocket()
{
	return 0;
}

int Distribute::AcceptStationSocket()
{
	return 0;
}

void Distribute::GetWorkStationRequest()
{
}

DWORD Distribute::AcceptStationThread(LPVOID lpParam)
{
	return 0;
}
