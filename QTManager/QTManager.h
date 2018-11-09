#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QTManager.h"
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
#include <QStandardItemModel>

#pragma comment(lib,"ws2_32.lib")

struct StationStatus {
	std::string ip;
	int port;
	bool sending;
};

struct StationViewStatus {
	std::string ip;
	int port;
	bool sending;
	bool isOnline;
};
Q_DECLARE_METATYPE(StationViewStatus)
struct AnalysisStatus {
	std::string ip;
	int port;
	bool analyzing;
	bool receiving;
};

struct AnalysisViewStatus {
	std::string ip;
	int port;
	bool analyzing;
	bool receiving;
	bool isOnline;
};
Q_DECLARE_METATYPE(AnalysisViewStatus)
class QTManager : public QMainWindow
{
	Q_OBJECT

public:
	QTManager(QWidget *parent = Q_NULLPTR);
	void Init();
	void InitView();
	//�������˼·��ͨ��ͬIP��ͬ�˿���������socket����satation��analysis��connect
	//�ڶ���ͨ�����͵������ݲ�ͬ������
	//������õ�һ��

	int InitStationSocket();
	int CreateStationSocket();
	int AcceptStation();
	static DWORD WINAPI AcceptStationThread(LPVOID lpParam);
	int GetStationStatus(SOCKET socket);
	static DWORD WINAPI GetStationStatusThread(LPVOID lpParam);



	int InitAnalysisSocket();
	int CreateAnalysisSocket();
	int AcceptAnalysis();
	static DWORD WINAPI AcceptAnalysisThread(LPVOID lpParam);
	int GetAnalysisStatus(SOCKET socket);
	static DWORD WINAPI GetAnalysisStatusThread(LPVOID lpParam);
signals:
	void EmitStationData(QVariant dataVar);
	void EmitAnalysisData(QVariant dataVar);
	public slots:
	void UpdateStation(QVariant dataVar);
	void UpdateAnalysis(QVariant dataVar);
private:
	Ui::QTManagerClass ui;
	SOCKET acceptStationSocket;
	SOCKET acceptAnalysisSocket;
	int acceptStationPort = 7070;
	int acceptAnalysisPort = 7071;
	char* ack = "ack";
};

