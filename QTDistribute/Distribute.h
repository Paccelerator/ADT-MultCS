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
#include <time.h>
#include <queue>
class Distribute
{
public:
	Distribute();
	//entry
	void Run();
	//ͨ�÷���
	void Init();
	int InitSocket();
	//����������IP����Ϣ
	void GetConfig();

	//distribute
	int InitStationSocket();
	int CreateStationSocket();
	int AcceptStationSocket();
	void GetWorkStationRequest();
	static DWORD WINAPI AcceptStationThread(LPVOID lpParam);
	~Distribute();
private:
	void GetIps();
private:
	int port;
	std::vector<std::string> ips;

	std::string workStationIP;
	int workStationPort;

	std::string managerIP;
	int managerPort;


	//status
	bool isReceiving;
	bool isAnalysing;

	SOCKET stationSocket;
	SOCKET distributeSocket;
	SOCKET managerSocket;
	SOCKET analysisSocket;
};

