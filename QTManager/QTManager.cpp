#include "QTManager.h"

struct ThreadSocketParam
{
	SOCKET socket;
	QTManager* point;
};


QTManager::QTManager(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	Init();
}

void QTManager::Init()
{
	qRegisterMetaType<QVariant>("QVariant");
	auto res=QObject::connect(this, SIGNAL(EmitStationData(QVariant)),this,SLOT(UpdateStation(QVariant)) );
	res=QObject::connect(this, SIGNAL(EmitAnalysisData(QVariant)), this, SLOT(UpdateAnalysis(QVariant)));
	InitView();
	InitStationSocket();
	InitAnalysisSocket();
}

void QTManager::InitView()
{
	QStandardItemModel* model = new QStandardItemModel(ui.tableView);
	model->setColumnCount(7);
	model->setHeaderData(0, Qt::Horizontal, "type");
	model->setHeaderData(1, Qt::Horizontal, "ip");
	model->setHeaderData(2, Qt::Horizontal, "port");
	model->setHeaderData(3, Qt::Horizontal, "sending");
	model->setHeaderData(4, Qt::Horizontal, "receving");
	model->setHeaderData(5, Qt::Horizontal, "analyzing");
	model->setHeaderData(6, Qt::Horizontal, "online");
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setModel(model);
}

int QTManager::InitStationSocket()
{
	CreateStationSocket();
	HANDLE hThread = CreateThread(NULL, 0, QTManager::AcceptStationThread, this, 0, NULL);
	return 0;
}

int QTManager::CreateStationSocket()
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
	//�����׽���
	//SOCKET m_socket=socket(AF_INET,SOCK_STREAM,0);
	this->acceptStationSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (acceptStationSocket == INVALID_SOCKET)
	{
		std::cout << "socket Create Failed��" << endl;
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(this->acceptStationPort);

	err = ::bind(acceptStationSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));    //�󶨱��ض˿�
	if (err == SOCKET_ERROR)
	{
		closesocket(acceptStationSocket);
		return FALSE;
	}
	listen(acceptStationSocket, 5);//��������

	return 1;
}

int QTManager::AcceptStation()
{
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (true)
	{
		SOCKET socket = accept(this->acceptStationSocket, (SOCKADDR*)&addrClient, &len);

		if (socket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			return 0;
		}
		//��ͻ��˵ķ�����ÿ���յ�һ��accept���ڿ���һ���߳�
		//������ֵ
		ThreadSocketParam param;
		param.socket = socket;
		param.point = this;

		HANDLE hThread = CreateThread(NULL, 0, QTManager::GetStationStatusThread, &param, 0, NULL);
	}
	return 0;
}

DWORD QTManager::AcceptStationThread(LPVOID lpParam)
{
	QTManager* p = (QTManager*)lpParam;
	p->AcceptStation();
	return 0;
}

int QTManager::GetStationStatus(SOCKET socket)
{
	int recvFlag;
	//�ж��ǲ��ǵ�һ��
	bool first = true;
	char recvBuffer[1024];
	StationViewStatus viewStatus;
	while (true) {
		//����״̬��Ϣ
		recvFlag=recv(socket, recvBuffer, 1024, 0);
		if (recvFlag != SOCKET_ERROR)
		{
			StationStatus* status = (StationStatus*)recvBuffer;
			if (first) {
				viewStatus.ip = status->ip;
				viewStatus.port = status->port;
			}
			viewStatus.sending = status->sending;
			viewStatus.isOnline = true;
		}
		else {
			//������
			viewStatus.isOnline = false;
			closesocket(socket);
			QVariant v;
			v.setValue(viewStatus);
			emit EmitStationData(v);
			break;
		}
		QVariant v2;
		v2.setValue(viewStatus);
		emit EmitStationData(v2);

		//���߶Է��������
		send(socket, ack, sizeof(ack), 0);

	}
	return 0;
}

DWORD QTManager::GetStationStatusThread(LPVOID lpParam)
{
	ThreadSocketParam* p = (ThreadSocketParam*)lpParam;
	p->point->GetStationStatus(p->socket);
	return 0;
}

int QTManager::InitAnalysisSocket()
{
	CreateAnalysisSocket();
	HANDLE hThread = CreateThread(NULL, 0, QTManager::AcceptAnalysisThread, this, 0, NULL);
	return 0;
}

int QTManager::CreateAnalysisSocket()
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
	//�����׽���
	//SOCKET m_socket=socket(AF_INET,SOCK_STREAM,0);
	this->acceptAnalysisSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (acceptAnalysisSocket == INVALID_SOCKET)
	{
		std::cout << "socket Create Failed��" << endl;
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(this->acceptAnalysisPort);

	err = ::bind(acceptAnalysisSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));    //�󶨱��ض˿�
	if (err == SOCKET_ERROR)
	{
		closesocket(acceptAnalysisSocket);
		return FALSE;
	}
	listen(acceptAnalysisSocket, 5);//��������

	return 1;
}

int QTManager::AcceptAnalysis()
{
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (true)
	{
		SOCKET socket = accept(this->acceptAnalysisSocket, (SOCKADDR*)&addrClient, &len);

		if (socket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			return 0;
		}
		//��ͻ��˵ķ�����ÿ���յ�һ��accept���ڿ���һ���߳�
		//������ֵ
		ThreadSocketParam param;
		param.socket = socket;
		param.point = this;

		HANDLE hThread = CreateThread(NULL, 0, QTManager::GetAnalysisStatusThread, &param, 0, NULL);
	}
	return 1;
}

DWORD QTManager::AcceptAnalysisThread(LPVOID lpParam)
{
	QTManager* p = (QTManager*)lpParam;
	p->AcceptAnalysis();
	return 0;
}

int QTManager::GetAnalysisStatus(SOCKET socket)
{
	int recvFlag;
	//�ж��ǲ��ǵ�һ��
	bool first = true;
	char recvBuffer[1024];
	AnalysisViewStatus viewStatus;
	while (true) {
		//����״̬��Ϣ
		recvFlag = recv(socket, recvBuffer, 1024, 0);
		if (recvFlag != SOCKET_ERROR)
		{
			AnalysisStatus* status = (AnalysisStatus*)recvBuffer;
			if (first) {
				viewStatus.ip = status->ip;
				viewStatus.port = status->port;
			}
			viewStatus.analyzing = status->analyzing;
			viewStatus.receiving = status->receiving;
			viewStatus.isOnline = true;
		}
		else {
			//������
			viewStatus.isOnline = false;
			closesocket(socket);
			QVariant v;
			v.setValue(viewStatus);
			emit EmitAnalysisData(v);
			break;
		}
		QVariant v2;
		v2.setValue(viewStatus);
		emit EmitAnalysisData(v2);

		//���߶Է��������
		send(socket, ack, sizeof(ack), 0);

	}
	return 0;
}

DWORD QTManager::GetAnalysisStatusThread(LPVOID lpParam)
{
	ThreadSocketParam* p = (ThreadSocketParam*)lpParam;
	p->point->GetAnalysisStatus(p->socket);
	return 0;
}

void QTManager::UpdateStation(QVariant dataVar)
{
	StationViewStatus status = dataVar.value<StationViewStatus>();
		
	bool found = false;
	QStandardItemModel* model=(QStandardItemModel*)ui.tableView->model();
	int count=model->rowCount();
	for (int i = 0; i < count; i++) {
		if (model->item(i, 0)->text() == "station") {
			auto ipstr = status.ip.c_str();
			auto portstr = QString::number(status.port);

			if (model->item(i, 1)->text() == ipstr && model->item(i, 2)->text() == portstr) {
				found = true;
				//����
				if (!status.isOnline) {
					model->setItem(i, 6, new QStandardItem("No"));
				}
				//����
				else {
					model->setItem(i, 3, new QStandardItem(status.sending ? "Yes" : "No"));
					model->setItem(i, 6, new QStandardItem("Yes"));
					//rows[3]->setText(status.sending ? "Yes" : "No");
				}
			
			}
		}
	}
	if (!found) {

		QList<QStandardItem*> list;
		list << new QStandardItem("station");
		list << new QStandardItem(status.ip.c_str());
		list<< new QStandardItem(QString::number(status.port));
		list << new QStandardItem(status.sending ? "Yes" : "No");
		list << new QStandardItem("");
		list << new QStandardItem("");
		list << new QStandardItem("Yes");
		model->appendRow(list);
	}
}
void QTManager::UpdateAnalysis(QVariant dataVar)
{
	AnalysisViewStatus status = dataVar.value<AnalysisViewStatus>();
	bool found = false;
	QStandardItemModel* model = (QStandardItemModel*)ui.tableView->model();
	int count = model->rowCount();
	for (int i = 0; i < count; i++) {
		if (model->item(i, 0)->text() == "analysis") {
			auto ipstr = status.ip.c_str();
			auto portstr = QString::number(status.port);

			if (model->item(i, 1)->text() == ipstr && model->item(i, 2)->text() == portstr) {
				found = true;
				//����
				if (!status.isOnline) {
					model->setItem(i, 6, new QStandardItem("No"));
					//model->setData(model->index(i, 6), "No");
					//rows[6]->setText("No");
				}
				//����
				else {
					
					model->setItem(i, 4, new QStandardItem(status.receiving ? "Yes" : "No"));
					//model->setData(model->index(i, 4), status.receiving ? "Yes" : "No");
					//rows[4]->setText(status.receiving ? "Yes" : "No");
					model->setItem(i, 5, new QStandardItem(status.analyzing ? "Yes" : "No"));
					//model->setData(model->index(i, 5), status.analyzing ? "Yes" : "No");
					//rows[5]->setText(status.analyzing ? "Yes" : "No");
					model->setItem(i, 6, new QStandardItem("Yes"));
				}

			}
		}
	}
	if (!found) {

		QList<QStandardItem*> list;
		list << new QStandardItem("analysis");
		list << new QStandardItem(status.ip.c_str());
		list << new QStandardItem(QString::number(status.port));
		list << new QStandardItem("");
		list << new QStandardItem(status.receiving ? "Yes" : "No");
		list << new QStandardItem(status.analyzing ? "Yes" : "No");
		list << new QStandardItem("Yes");
		model->appendRow(list);
	}
}
