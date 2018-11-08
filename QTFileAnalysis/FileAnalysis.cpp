#include "FileAnalysis.h"


struct WorkStationInfo {
	std::string ip;
	int port;
};
struct AnalysisStatus {
	std::string ip;
	int port;
	bool analyzing;
	bool receiving;
};
struct FileParam
{
	std::string fileNames[100] = { 0 };
	int fileSize[100] = { 0 };
	int num = 0;
};
FileAnalysis::FileAnalysis()
{
}

void FileAnalysis::Run()
{
}

void FileAnalysis::Init()
{
	GetConfig();
	InitSocket();
}

int FileAnalysis::InitSocket()
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

}

void FileAnalysis::GetConfig()
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

int FileAnalysis::InitAnalysisSocket()
{
	CreateAnalysisSocket();
	HANDLE hThread = CreateThread(NULL, 0, FileAnalysis::AcceptDistributeThread, this, 0, NULL);
	return 0;
}

int FileAnalysis::CreateAnalysisSocket()
{
	int err;
	//�����׽���
	//SOCKET m_socket=socket(AF_INET,SOCK_STREAM,0);
	analysisSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (analysisSocket == INVALID_SOCKET)
	{
		std::cout << "analysisSocket Create Failed��" << endl;
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(this->port);

	err = ::bind(analysisSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));    //�󶨱��ض˿�
	if (err == SOCKET_ERROR)
	{
		closesocket(analysisSocket);
		return FALSE;
	}
	listen(analysisSocket, 5);//��������

	return 1;
}

int FileAnalysis::AcceptDistributeSocket()
{
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (true)
	{
		this->distributeSocket = accept(this->analysisSocket, (SOCKADDR*)&addrClient, &len);

		if (distributeSocket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(distributeSocket);
			WSACleanup();
			return;
		}
		//��ͻ��˵ķ�����ÿ���յ�һ��accept���ڿ���һ���߳�
		GetWorkStationInfo();
	}
}

void FileAnalysis::GetWorkStationInfo()
{
	char recvBuffer[1024];
	WorkStationInfo* info;
	recv(analysisSocket, recvBuffer, 1024, NULL);
	//char[] ת�ṹ��
	info = (WorkStationInfo *)recvBuffer;

	this->workStationIP = info->ip;
	this->workStationPort = info->port;


}

DWORD FileAnalysis::AcceptDistributeThread(LPVOID lpParam)
{
	FileAnalysis* p = (FileAnalysis*)lpParam;
	p->GetWorkStationInfo();
	return 0;
}

void FileAnalysis::GetIps()
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

int FileAnalysis::InitWorkStationSocket()
{
	HANDLE hThread = CreateThread(NULL, 0, FileAnalysis::connectStationThread, this, 0, NULL);
	return 0;
}

int FileAnalysis::RecevieFile()
{
	unsigned long long file_size = 0;
	std::string analysisContent;
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(this->workStationIP.c_str());
	addrSrv.sin_port = htons(this->workStationPort);
	addrSrv.sin_family = AF_INET;
	//::connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	//recv(sockClient,(char*)&file_size,sizeof(unsigned long long)+1,NULL);

	if (!::connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{
		isReceiving = true;
		QString path = QDir::currentPath() + "/";
		QString logName = path + "FileResult.log";
		QFile log(logName);
		char sendBuffer[1024];
		char recvBuffer[1024];
		char* ack = "ack";

		//chulilog
		log.open(QFile::Append|QFile::WriteOnly);

		//һֱ����״̬��Ϣ

			//��һ���Ƚ����ļ���Ϣ
		FileParam* param;

		recv(sockClient, recvBuffer, 1024, 0);

		param = (FileParam*)recvBuffer;

		if (param != NULL) {
			//ȷ���յ����ļ���Ϣ
			send(sockClient, ack, sizeof(ack), 0);


			for (int i = 0; i < param->num; i++) {

				auto filename = QString::fromStdString(param->fileNames[i]);
				auto tempPath = path + filename;
				file_size = param->fileSize[i];
				QFile file(tempPath);
				file.open(QIODevice::ReadWrite);

				unsigned short maxvalue = file_size;

				if (file_size > 0)
				{

					DWORD dwNumberOfBytesRecv = 0;
					DWORD dwCountOfBytesRecv = 0;

					char Buffer[1024];

					do
					{
						//һֱ�����ļ�
						dwNumberOfBytesRecv = ::recv(sockClient, Buffer, sizeof(Buffer), 0);
						file.write(Buffer, dwNumberOfBytesRecv);
						dwCountOfBytesRecv += dwNumberOfBytesRecv;

					} while ((file_size - dwCountOfBytesRecv) && dwNumberOfBytesRecv > 0);


				}
				else
				{
					std::cout << "get file size failed" << std::endl;
				}
				//������Ϻ�֪ͨ�������,��ʼ��һ���ļ�
				send(sockClient, ack, sizeof(ack), 0);

				isAnalysing = true;
				//�����ļ���Ϣ
				SYSTEMTIME st = { 0 };
				GetLocalTime(&st);
				std::string date = std::to_string(st.wYear) + "-" + std::to_string(st.wMonth) + "-" + std::to_string(st.wDay);
				std::string time = std::to_string(st.wHour) + ":" + std::to_string(st.wMinute) + ":" + std::to_string(st.wSecond);

				auto temp = filename + ", ";

				temp += QString::fromStdString(this->workStationIP + ", ");

				std::string tempPort = std::to_string(this->workStationPort) + ", ";

				temp += QString::fromStdString(tempPort);
				temp += QString::fromStdString(date + ", ");
				temp += QString::fromStdString(time + ", ");
				temp += file.read(8);
				temp += QString("%1").arg(file_size) + "\n";

				log.write(temp.toLatin1());
				file.close();
			}

		}
		log.close();
	}
	isAnalysing = false;
	isReceiving = false;
	closesocket(sockClient);//�ر��׽���
	return 0;
}

DWORD FileAnalysis::connectStationThread(LPVOID lpParam)
{
	FileAnalysis* p = (FileAnalysis*)lpParam;
	p->RecevieFile();
	return 0;
}

int FileAnalysis::InitManagerSocket()
{
	//CreateManagerSocket();
	HANDLE hThread = CreateThread(NULL, 0, FileAnalysis::connectManagerThread, this, 0, NULL);
	return 0;
}

int FileAnalysis::CreateManagerSocket()
{

	return 1;
}

void FileAnalysis::SendIPAndStatus()
{
	unsigned long long file_size = 0;

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
			AnalysisStatus status;
			status.ip = this->ips[0];
			status.port = this->port;
			status.analyzing = this->isAnalysing;
			status.receiving = this->isReceiving;

			//th
			//�ṹ��ת��char[]
			memcpy(sendBuffer, &status, sizeof(status));

			//��һ���ȷ����ļ���Ϣ
			send(sockClient, sendBuffer, 1024, NULL);

			//Ȼ��ȴ�ȷ��ָ��
			recv(sockClient, recvBuffer, 1024, 0);
		}
		//�ж��źţ�fix me
	}

	closesocket(sockClient);//�ر��׽���
}

DWORD FileAnalysis::connectManagerThread(LPVOID lpParam)
{
	FileAnalysis* p = (FileAnalysis*)lpParam;
	p->SendIPAndStatus();
	return 0;
}

FileAnalysis::~FileAnalysis()
{
}
