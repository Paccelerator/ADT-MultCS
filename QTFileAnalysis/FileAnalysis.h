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
class FileAnalysis
{
public:
	FileAnalysis();
	//entry
	void Run();
	//ͨ�÷���
	void Init();
	int InitSocket();
	//����������IP����Ϣ
	void GetConfig();

	//distribute
	int InitAnalysisSocket();
	int CreateAnalysisSocket();
	int AcceptDistributeSocket();
	void GetWorkStationInfo();
	static DWORD WINAPI AcceptDistributeThread(LPVOID lpParam);

	//workstation
	int InitWorkStationSocket();
	int RecevieFile();

	//֪ͨ������
	int InitManagerSocket();
	int CreateManagerSocket();
	void SendIPAndStatus();
	static DWORD WINAPI connectManagerThread(LPVOID lpParam);

	~FileAnalysis();
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

