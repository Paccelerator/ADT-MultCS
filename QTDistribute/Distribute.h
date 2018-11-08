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

struct AnalysisData
{
	std::string ip;
	int port;
	/* data */
};

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
	int InitDistributeSocket();
	int CreateDistributeSocket();
	int AcceptStationSocket();
	void GetWorkStationRequest(SOCKET socket);
	static DWORD WINAPI AcceptStationThread(LPVOID lpParam);
	//�ö��߳�����ȡҪ�����ļ���station
	static DWORD WINAPI GetStationInfoThread(LPVOID lpParam);

	//analysis
	int InitDistributeSocket();
	int CreateDistributeSocket();
	int AcceptStationSocket();
	int NoticeAnalysis(std::string stationIP,int stationPort);
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
	//File Analysis
	std::vector<AnalysisData> AnalysisList;
	int nowAnalysis;
	//status
	bool isReceiving;
	bool isAnalysing;

	SOCKET stationSocket;
	SOCKET distributeSocket;
	SOCKET managerSocket;
	SOCKET analysisSocket;
};

