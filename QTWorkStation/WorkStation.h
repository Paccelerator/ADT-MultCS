#pragma once
#include <vector>
#include <string>
#include <QNetworkInterface>
#include <Windows.h>
#include <iostream>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdatastream.h>
class WorkStation
{
public:
	WorkStation();

	//entry
	void Run();
	//ͨ�÷���
	void Init();
	int InitSocket();
	//����������IP����Ϣ
	void GetConfig();

	//�����ļ���������
	void SendFile();
	int InitStationSocket();
	int CreateStationSocket();
	void AcceptAnalysis();
	static DWORD WINAPI AcceptAnalysisThread(LPVOID lpParam);

	//֪ͨ���������ļ�Ҫ����
	int InitDistributeSocket();
	int CreateDistributeSocket();
	void NoticeDistributeServer();
	static DWORD WINAPI connectDistributeThread(LPVOID lpParam);

	//֪ͨ������
	int InitManagerSocket();
	int CreateManagerSocket();
	void SendIPAndStatus();
	static DWORD WINAPI connectManagerThread(LPVOID lpParam);

	~WorkStation();
private:
	void GetIps();
	QStringList GetFileList();
private:
	int port;
	std::vector<std::string> ips;

	std::string distributeIP;
	int distributePort;

	std::string managerIP;
	int managerPort;

	//status
	bool isSending;

	SOCKET stationSocket;
	SOCKET distributeSocket;
	SOCKET managerSocket;
	SOCKET analysisSocket;
};

