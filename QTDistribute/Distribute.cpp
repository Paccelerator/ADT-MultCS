#include "Distribute.h"


struct StationSocketParam
{
	Distribute* point;
	SOCKET socket;
	/* data */
};
struct StationInfo {
	int port;
	std::string ip;
	int fileNum;
};

Distribute::Distribute()
{
}

void Distribute::Run()
{
}

void Distribute::Init()
{
	nowAnalysis=0;
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
	file.open(QFile::ReadOnly);
	if (file.exists()) {
		auto portstr = file.readAll().data();
		port = atoi(portstr);
	}
	file.close();

	//get analysis config
	auto analysisPath= QDir::currentPath() + "/analysis.config";
	QFile als(analysisPath);
	if (als.exists()) {
		QTextStream in(&file);
		while (in.atEnd()) {
			QStringList analysisInfo=in.readLine().split(',');
			AnalysisData info;
			info.ip = analysisInfo[0].toStdString();
			info.port = analysisInfo[1].toInt();
			this->AnalysisList.push_back(info);
		}
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

int Distribute::InitDistributeSocket()
{
	CreateDistributeSocket();
	HANDLE hThread = CreateThread(NULL, 0, Distribute::AcceptStationThread, this, 0, NULL);
	return 0;
}

int Distribute::CreateDistributeSocket()
{
	int err;
	//�����׽���
	//SOCKET m_socket=socket(AF_INET,SOCK_STREAM,0);
	distributeSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (distributeSocket == INVALID_SOCKET)
	{
		std::cout << "distributeSocket Create Failed��" << endl;
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(this->port);

	err = ::bind(distributeSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));    //�󶨱��ض˿�
	if (err == SOCKET_ERROR)
	{
		closesocket(distributeSocket);
		return FALSE;
	}
	listen(distributeSocket, 5);//��������

	return 1;
}

int Distribute::AcceptStationSocket()
{
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (true)
	{
		SOCKET socket = accept(this->distributeSocket, (SOCKADDR*)&addrClient, &len);

		if (socket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(socket);
			return;
		}


		StationSocketParam param;
		param.socket=socket;
		param.point=this;
		//��ͻ��˵ķ�����ÿ���յ�һ��accept���ڿ���һ���߳�
		//ͬʱ�п����ж��station����
		HANDLE hThread = CreateThread(NULL, 0, Distribute::GetStationInfoThread, &param, 0, NULL);
	}
}

void Distribute::GetWorkStationRequest(SOCKET socket)
{
	char* ack="ack";
	char recvBuffer[1024];
	StationInfo* info;
	recv(socket, recvBuffer, 1024, NULL);
	//char[] ת�ṹ��
	info = (StationInfo *)recvBuffer;
	std::string ip=info->ip;
	int port=info->port;
	//ȷ��
	send(socket,ack,sizeof(ack),0);

	//֪ͨanalysis server
	NoticeAnalysis(ip,port);
}

DWORD Distribute::AcceptStationThread(LPVOID lpParam)
{
	Distribute* p = (Distribute*)lpParam;
	p->AcceptStationSocket();
	return 0;
}
DWORD Distribute::GetStationInfoThread(LPVOID lpParam)
{
	StationSocketParam* param = (StationSocketParam*)lpParam;
	param->point->GetWorkStationRequest(param->socket);
	return 0;
}

int Distribute::NoticeAnalysis(std::string stationIP,int stationPort)
{
	unsigned long long file_size = 0;
	//get analysis server data
	AnalysisData analysis=this->AnalysisList[this->nowAnalysis];
	this->nowAnalysis= (this->nowAnalysis+1)%this->AnalysisList.size();

	//set satation info
	StationInfo info;
	info.ip=stationIP;
	info.port=port;

	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(analysis.ip.c_str());
	addrSrv.sin_port = htons(analysis.port);
	addrSrv.sin_family = AF_INET;
	//::connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	//recv(sockClient,(char*)&file_size,sizeof(unsigned long long)+1,NULL);

	if (!::connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{


		char sendBuffer[1024];
		char recvBuffer[1024];

			//th
			//�ṹ��ת��char[]
			memcpy(sendBuffer, &info, sizeof(info));

			//��һ���ȷ����ļ���Ϣ
			send(sockClient, sendBuffer, 1024, NULL);

			//Ȼ��ȴ�ȷ��ָ��
			recv(sockClient, recvBuffer, 1024, 0);
		
		//�ж��źţ�fix me
	}

	closesocket(sockClient);//�ر��׽���
}