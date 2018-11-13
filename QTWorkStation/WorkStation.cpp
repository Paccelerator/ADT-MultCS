#include "WorkStation.h"


struct FileParam
{
	std::string fileNames[100]  ;
	int fileSize[100] ;
	int num = 0;
};
struct SendToDistribute {
	int port;
	std::string ip;
	int fileNum;
};

struct Status {
	std::string ip;
	int port;
	bool sending;
};
WorkStation::WorkStation()
{
	Init();
}

void WorkStation::Run()
{
	//����socket���ȴ� connect
	InitStationSocket();
	//connect manager send ip
	InitManagerSocket();
	while (true) {
		//֪ͨ�����ļ�
		if (!isSending) {
			InitDistributeSocket();
		}
		Sleep(1000 * 20);
	}
}

void WorkStation::Init()
{
	//����
	ips.push_back("127.0.0.1");
	//GetIps();
	GetConfig();
	InitSocket();
}

int WorkStation::InitSocket()
{
	//�����׽��ֿ�
	WORD wVersionRequested;
	//����ṹ�������洢��WSAStartup�������ú󷵻ص�Windows Sockets���ݡ�������Winsock.dllִ�е����ݡ�
	WSADATA wsaData;
	int err;
	//��������1.1�汾��winsock��MAKEWORD(2,2)��2.2�汾
	wVersionRequested = MAKEWORD(1, 1);
	//����WSA��֮ǰclientûִ�������һֱ����
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
	return 1;
}

void WorkStation::GetConfig()
{
	GetIps();
	port = 8989;
	isSending = false;
	auto path= QCoreApplication::applicationDirPath() +"/config.config";
	QFile file(path);
	if (file.exists()) {
		file.open(QFile::ReadOnly | QFile::Text);
		auto portstr=file.readLine();
		port=atoi(portstr);
		file.close();
	}

	//��ȡ��������IP�Ͷ˿�
	auto managerPath = QCoreApplication::applicationDirPath() + "/man.config";
	QFile mfile(managerPath);
	if (mfile.exists()) {
		mfile.open(QFile::ReadOnly);
		QTextStream in(&mfile);
		while (!in.atEnd()) {
			QStringList infoList = in.readLine().split(',');
			this->managerIP = infoList[0].toStdString();
			this->managerPort = infoList[1].toInt();
		}
		mfile.close();
	}

	//��ȡ��������IP�Ͷ˿�
	auto distributePath = QCoreApplication::applicationDirPath() + "/dis.config";
	QFile dfile(distributePath);
	if (dfile.exists()) {
		dfile.open(QFile::ReadOnly);
		QTextStream in(&dfile);
		while (!in.atEnd()) {
			QStringList infoList = in.readLine().split(',');
			this->distributeIP = infoList[0].toStdString();
			this->distributePort = infoList[1].toInt();
		}
		dfile.close();
	}


}

void WorkStation::SendFile()
{

	char infochar[1024*100];
	char recvBuffer[1024];
	
	//��ȡ�б�
	auto fileList = GetFileList();
	//
	auto path = QCoreApplication::applicationDirPath();
	path += "/file/";
	//��ȡ�ļ���Ϣ
	FileParam fileParam;
	int index = 0;
	for each (QString filePath in fileList)
	{
		
		QFileInfo info(path+filePath);
		auto qname = info.fileName().toStdString();
		fileParam.fileNames[index]=qname;
		fileParam.fileSize[index]=info.size();
		index++;

	}
	fileParam.num = index;

	//�ṹ��ת��char[]
	memcpy(infochar, &fileParam, sizeof(fileParam));

	//�ı�־λ
	isSending = true;

	//��һ���ȷ����ļ���Ϣ
	send(analysisSocket, infochar, sizeof(infochar), NULL);

	//����ȷ���Է����������ļ���Ϣ
	recv(analysisSocket, recvBuffer, 1024, NULL);

	//ѭ�������ļ�
	for each(QString filePath in fileList)
	{
		QFile file(path+filePath);
		auto res=file.open(QFile::ReadOnly);

		QDataStream stream(&file);

		char Buffer[1024];
		DWORD dwNumberOfBytesRead;

		//�Ѷ����ܴ�С
		long totalBytes = 0;
		int actualSendBytes = 0;
		do
		{
			dwNumberOfBytesRead = stream.readRawData(Buffer, sizeof(Buffer));
			actualSendBytes = ::send(analysisSocket, Buffer, dwNumberOfBytesRead, 0);

			//��ֹ�����������ʧ����
			totalBytes += actualSendBytes;
			stream.device()->seek(totalBytes);
		} while (dwNumberOfBytesRead && actualSendBytes>0);

		file.close();

		//�ȴ��Է�ȷ��,ȷ������ٴ�����һ���ļ�
		recv(analysisSocket, recvBuffer, 1024, NULL);

		//����ȷ���ź� fix me

		//ģ��ȽϾõķ���
		Sleep(100);
	}
	isSending = false;
	
}

int WorkStation::InitStationSocket()
{
	CreateStationSocket();
	HANDLE hThread = CreateThread(NULL, 0, WorkStation::AcceptAnalysisThread, this, 0, NULL);
	return 0;
}

int WorkStation::CreateStationSocket()
{
	
	int err;
	//�����׽���
	//SOCKET m_socket=socket(AF_INET,SOCK_STREAM,0);
	stationSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (stationSocket == INVALID_SOCKET)
	{
		std::cout << "socket Create Failed��" << endl;
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(this->port);

	err = ::bind(stationSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));    //�󶨱��ض˿�
	if (err == SOCKET_ERROR)
	{
		closesocket(stationSocket);
		return FALSE;
	}
	listen(stationSocket, 5);//��������

	return 1;
}

int WorkStation::InitDistributeSocket()
{
	//connect�Ļ�����Ҫ����
	//CreateDistributeSocket();
	HANDLE hThread = CreateThread(NULL, 0, WorkStation::connectDistributeThread, this, 0, NULL);
	return 0;
}
int WorkStation::CreateDistributeSocket()
{
	//�����׽��ֿ�
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}

	return 1;
}

void WorkStation::NoticeDistributeServer()
{
	unsigned long long file_size = 0;

	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(this->distributeIP.c_str());
	addrSrv.sin_port = htons(this->distributePort);
	addrSrv.sin_family = AF_INET;
	//::connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	//recv(sockClient,(char*)&file_size,sizeof(unsigned long long)+1,NULL);

	if (!::connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{


		char sendBuffer[1024];
		char recvBuffer[1024];
		//��ȡ�ļ���Ϣ
		SendToDistribute param;
		param.ip = this->ips[0];
		param.fileNum = GetFileList().size();
		param.port = this->port;

		//�ṹ��ת��char[]
		memcpy(sendBuffer, &param, sizeof(param));

		//��һ���ȷ����ļ���Ϣ
		send(sockClient, sendBuffer, 1024, NULL);

		//Ȼ��ȴ�ȷ��ָ��
		recv(sockClient, recvBuffer, 1024, 0);

		//�ж��źţ�fix me
	}

	closesocket(sockClient);//�ر��׽���
}


DWORD WorkStation::connectDistributeThread(LPVOID lpParam)
{
	WorkStation* p = (WorkStation*)lpParam;
	p->NoticeDistributeServer();
	return 0;
}

int WorkStation::InitManagerSocket()
{
	//CreateManagerSocket();
	HANDLE hThread = CreateThread(NULL, 0, WorkStation::connectManagerThread, this, 0, NULL);
	return 0;
}

int WorkStation::CreateManagerSocket()
{
	
	return 1;
}

void WorkStation::SendIPAndStatus()
{
	unsigned long long file_size = 0;
	char* ack = "ack";
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(this->managerIP.c_str());
	addrSrv.sin_port = htons(this->managerPort);
	addrSrv.sin_family = AF_INET;
	//::connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	//recv(sockClient,(char*)&file_size,sizeof(unsigned long long)+1,NULL);

	if (!::connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{


		char sendBuffer[1024];
		char recvBuffer[1024];
		//һֱ����״̬��Ϣ
		while (true)
		{
			//��ȡ״̬��Ϣ
			Status status;
			status.ip = this->ips[0];
			status.port = this->port;
			status.sending = this->isSending;

			//th
			//�ṹ��ת��char[]
			memcpy(sendBuffer, &status, sizeof(status));

			//����״̬��Ϣ
			send(sockClient, sendBuffer, 1024, NULL);

			//Ȼ��ȴ�ȷ��ָ��
			recv(sockClient, recvBuffer, 1024, 0);

			//ÿ1�뷢һ��
			Sleep(1000);
		}
		//�ж��źţ�fix me
	}

	closesocket(sockClient);//�ر��׽���
}

DWORD WorkStation::connectManagerThread(LPVOID lpParam)
{
	WorkStation* p = (WorkStation*)lpParam;
	p->SendIPAndStatus();
	return 0;
}

void WorkStation::AcceptAnalysis()
{
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (true)
	{
		this->analysisSocket = accept(stationSocket, (SOCKADDR*)&addrClient, &len);

		if (analysisSocket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(analysisSocket);
			WSACleanup();
			return;
		}
		//��ͻ��˵ķ�����ÿ���յ�һ��accept���ڿ���һ���߳�
		//������ֵ
		SendFile();
	}
}

DWORD WorkStation::AcceptAnalysisThread(LPVOID lpParam)
{
	WorkStation* p = (WorkStation*)lpParam;
	p->AcceptAnalysis();
	return 0;
}

void WorkStation::GetIps()
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

QStringList WorkStation::GetFileList()
{

	//��ȡ�ļ��б�
	auto path= QCoreApplication::applicationDirPath();
	path += "/file/";
	QDir dir(path);
	auto fileList=dir.entryList(QDir::Files);
	return fileList;
}


WorkStation::~WorkStation()
{
}
