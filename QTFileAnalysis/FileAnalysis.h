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
#include <QtCore/QCoreApplication>

#pragma comment(lib,"ws2_32.lib")

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
	static DWORD WINAPI connectStationThread(LPVOID lpParam);

	//֪ͨ������
	int InitManagerSocket();
	int CreateManagerSocket();
	void SendIPAndStatus();
	static DWORD WINAPI connectManagerThread(LPVOID lpParam);

	//�����ļ�
	void CreateAnalyzeThread();
	void AnalyzeFile();
	static DWORD WINAPI AnalyzeFileThread(LPVOID lpParam);

	//֪ͨ��������ǰ�����ļ�����
	int InitDistributeSocket();
	void SendAnalyzingFileNum();
	static DWORD WINAPI connectDistributeThread(LPVOID lpParam);

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
	char* ack = "ack";

	std::string distributeIP;
	int distributePort;

	//status
	bool isReceiving;
	bool isAnalysing;

	//�ܽ����ļ���
	int totalNum = 0;
	//analysis list
	std::queue<QString> analysisList;
	std::queue<int> sizeList;

	//Ҫ�����������߳���
	int threadNum = 2;
	//�ļ��б�����
	HANDLE hMutex;
	//�ļ�������
	HANDLE fileMutex;
	//�ȴ�ʱ��
	int millisec = 1000*2;
	//�������Ƿ�ɾ��
	bool deleteAfter = true;
	//���˶����ļ���ʼ����
	int beginAnalyzeFileNums = 10;

	//sendAnalysis Status sec
	double toDistributeSec = 0.2;



	SOCKET stationSocket;
	SOCKET distributeSocket;
	SOCKET managerSocket;
	SOCKET analysisSocket;
};