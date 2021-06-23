#include "CwssRecver.h"


/********************
	ȫ�ֱ���������
********************/
frame *pwin;
CwssRecver g_cr = CwssRecver();
/********************
	��������������
********************/
//����ǰ�˷���������json�ַ���
wstring GetJsonFieldFromJsonString(string json, string jsonField)
{
	return pwin->call_function("GetJsonFieldFromJsonString", json.c_str(), jsonField.c_str()).to_string().c_str();
}
wstring GetJsonFieldFromJsonString(wstring json, string jsonField)
{
	return pwin->call_function("GetJsonFieldFromJsonString", json.c_str(), jsonField.c_str()).to_string().c_str();
}

//��ȡ��ǰ����Ŀ¼
string	getCurrentWorkDir()
{
	static string ApplicationPath;
	if (ApplicationPath.length() != 0)
		return ApplicationPath;
	char * buffer = new char[MAX_PATH];
	_getcwd(buffer, MAX_PATH);
	ApplicationPath = buffer;
	delete[] buffer;
	return ApplicationPath;
}

std::string WcharToChar(const wchar_t* wp, size_t m_encode = CP_ACP)
{
	std::string str;
	int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	char	*m_char = new char[len + 1];
	WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	str = m_char;
	delete m_char;
	return str;
}

std::wstring CharToWchar(const char* c, size_t m_encode = CP_ACP)
{
	std::wstring str;
	int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
	wchar_t*	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
	m_wchar[len] = '\0';
	str = m_wchar;
	delete m_wchar;
	return str;
}

char * WsGetErrorInfo()
{
	static CHAR pBuf[1024] = { 0 };
	const ULONG bufSize = 1024;
	DWORD retSize;
	LPTSTR pTemp = NULL;

	pBuf[0] = '0';

	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		GetLastError(),
		LANG_NEUTRAL,
		(LPTSTR)&pTemp,
		0,
		NULL);
	if (retSize > 0) {
		pTemp[wcslen(pTemp) - 2] = '\0'; //remove cr and newline character
		sprintf_s(pBuf, bufSize, "%0.*ws", bufSize, pTemp);
		LocalFree((HLOCAL)pTemp);
	}
	return pBuf;
}

char * WsGetErrorInfo(DWORD err)
{
	static CHAR pBuf[1024] = { 0 };
	const ULONG bufSize = 1024;
	DWORD retSize;
	LPTSTR pTemp = NULL;

	pBuf[0] = '0';

	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		err,
		LANG_NEUTRAL,
		(LPTSTR)&pTemp,
		0,
		NULL);
	if (retSize > 0) {
		pTemp[wcslen(pTemp) - 2] = '\0'; //remove cr and newline character
		sprintf_s(pBuf, bufSize, "%0.*ws", bufSize, pTemp);
		LocalFree((HLOCAL)pTemp);
	}
	return pBuf;
}

string pack_varint(int d) {
	static string o = "";
	o.clear();
	while (1)
	{
		int b = d & 0x7F;
		d >>= 7;
		o += b | (d > 0 ? 0x80 : 0);
		if (d == 0)
			break;
	}

	return o;
}

string pack_data(string d)
{
	return pack_varint(d.length()) + d;
}

/********************
	�෽��������
********************/

CwssRecver::CwssRecver()
{
}


CwssRecver::~CwssRecver()
{
	WSACleanup();
}

void CwssRecver::SetFlag(int i)
{
}
//���ڻ�ȡ�����У���Щ����ʧ����Ҫֱ��return�����������߳���ֱ��ruturn���޷����´����еķ�������Ϣ����˽��˹��̶�����һ���º���
bool GetServerInfo(CwssRecver *cr, string serverAddr, string serverPort) {
	struct addrinfo *result = NULL, hints, *pres;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	int iResult;
	//�׽���������Ϣ
	iResult = getaddrinfo(serverAddr.c_str(), serverPort.c_str(), &hints, &result);
	if (iResult != 0) {
		string logstr = "��������ʧ�� : ";
		logstr += WcharToChar(gai_strerror(iResult));
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		cr->iGetFlag = cr->FGHFAILED;
		return false;
	}
	if (result == nullptr)
	{
		string logstr = "��������ʧ�� : ";
		logstr += "result:null";
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		cr->iGetFlag = cr->FGHFAILED;
		return false;
	}
	SOCKET ClientSocket = INVALID_SOCKET;
	pres = result;
	while (true)
	{
		if (pres == nullptr)
		{
			freeaddrinfo(result);
			string logstr = "�ѱ��������µ����н���������ʧ��";
			logstr += " \n";
			pwin->call_function("DebugLog", logstr);
			return false;
		}
		ClientSocket = socket(pres->ai_family, pres->ai_socktype, pres->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			string logstr = "�����׽���ʧ�� : ";
			logstr +=  WsGetErrorInfo(WSAGetLastError());
			logstr += " \n";
			pwin->call_function("DebugLog", logstr);
			freeaddrinfo(result);
			cr->iGetFlag = cr->FCSFAILED;
			return false;
		}

		iResult = connect(ClientSocket, pres->ai_addr, (int)pres->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			string logstr;
			/*���Ż�*/
			char addrStr[INET_ADDRSTRLEN];
			const char *paddrStr = inet_ntop(AF_INET, &((SOCKADDR_IN*)&(pres->ai_addr))->sin_addr, addrStr, sizeof(addrStr));
			logstr += WsGetErrorInfo(WSAGetLastError());
			logstr += paddrStr;
			logstr += " ����ʧ��";
			logstr += " \n";
			pwin->call_function("DebugLog", logstr);
			closesocket(ClientSocket);
			cr->iGetFlag = cr->FCONFAILED;
			pres = pres->ai_next;
			continue;

		}
		else
		{
			pwin->call_function("DebugLog", "���ӳɹ���\n");
			break;
		}
	}
	freeaddrinfo(result);
	char sendbuf[512]{ 0 };
	byte egMcPingPacket[] =
	{
		0xf, 0x0, 0x0, 0x9, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68,
		0x6f, 0x73, 0x74, 0x63, 0xdd, 0x1,
	};
	char str[64]{ 0 };
	string packstr;
	packstr += '\x00';
	packstr += '\x00';
	packstr += pack_data(serverAddr.c_str()); //��װ������Ϣ
	char* portStrHex = new char[16]{0};
	short port = short(atoi(serverPort.c_str()));
	//sprintf_s(portStrHex,15, "%x", port);
	portStrHex[0] = port >> 8;
	portStrHex[1] = port & 0x00FF;
	portStrHex[2] = '\x01';
	packstr += portStrHex;//��װ�˿���Ϣ
	delete[] portStrHex;
	packstr = pack_data(packstr);
	memcpy_s(str, 64, packstr.c_str(), packstr.length());
	iResult = send(ClientSocket, (char*)str, packstr.length(), 0);
	pwin->call_function("DebugLog", WsGetErrorInfo());
	cr->iGetFlag = cr->FSENDFAILED;
	if (iResult == SOCKET_ERROR) {
		string logstr = "����ʧ��! ";
		logstr += WsGetErrorInfo(WSAGetLastError());
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		closesocket(ClientSocket);
		cr->iGetFlag = cr->FSENDFAILED;
		return false;
	}
	else
	{
		string logstr = "���ͳɹ�! ";
		logstr += iResult;
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
	}
	//string strRequest = pack_data("\x0");

	//iResult = send(ClientSocket, strRequest.c_str(), strRequest.length(), 0);

	byte sendRequest[2];
	sendRequest[0] = 0x1;
	sendRequest[1] = 0x0;
	iResult = send(ClientSocket, (char*)sendRequest, 2, 0);
	printf_s(WsGetErrorInfo());
	if (iResult == SOCKET_ERROR) {
		string logstr = "����ʧ��! ";
		logstr += WsGetErrorInfo(WSAGetLastError());
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		closesocket(ClientSocket);
		cr->iGetFlag = cr->FSENDFAILED;
		return false;
	}
	else
	{
		string logstr = "���ͳɹ�! ";
		logstr += iResult;
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
	}
	byte recvbuf[1025]{ 0 };
	recvbuf[1024] = '\0';
#ifdef  _DEBUG
	byte* temp = new byte[1024 * 64];
	unsigned int tempIndex = 0;
#endif
	wstring allRecv;
	string ascAllRecv;
	iResult = 1;
	//Server���ص����ݰ��л�������ݰ�����ǰ׺����δ����ʾȥ��ǰ׺��ֱ������json�ı���ͷ��{
	for (int i = 0; iResult > 0 && recvbuf[0] != '{'; i++)
	{
		iResult = recv(ClientSocket, (char*)recvbuf, 1, 0);
	}
	allRecv += recvbuf[0]; //��json��ͷ�����żӻ�
	ascAllRecv += recvbuf[0];
#ifdef  _DEBUG
	temp[tempIndex++] = recvbuf[0];
#endif
	iResult = recv(ClientSocket, (char*)recvbuf, 1024, 0);
	while (iResult > 0)
	{
		string logstr = "���ճɹ���";
		logstr += iResult;
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		for (int i = 0; i < iResult; i++)
		{
			allRecv += recvbuf[i];
			ascAllRecv += recvbuf[i];
#ifdef  _DEBUG
			temp[tempIndex] = recvbuf[i];
			tempIndex++;
#endif
		}
		if (iResult != 1024)//С��1024˵�������һ�η���
			break;
		iResult = recv(ClientSocket, (char*)recvbuf, 1024, 0);
	}
	if (iResult == SOCKET_ERROR) {
		string logstr = "����ʧ��";
		logstr += WsGetErrorInfo(WSAGetLastError());
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		closesocket(ClientSocket);
		cr->iGetFlag = cr->FRECVFAILED;
		return false;
	}
#ifdef  _DEBUG
	{
		FILE *fp;
		string faviconPath = getCurrentWorkDir() + "\\ServerIcons\\";
		if (_access(faviconPath.c_str(), 0) == -1)//�ļ��в�����ʱ�������ļ���
			_mkdir(faviconPath.c_str());
		faviconPath = faviconPath + serverAddr.c_str() + ".txt";//��imgд�뵽�ļ���
		fopen_s(&fp, faviconPath.c_str(), "wb");
		for (int i = 0; i < tempIndex; i++)
		{
			fprintf_s(fp, "%c", temp[i]);
		}
		fclose(fp);
	}
#endif
	allRecv = UTF8ToUnicode(ascAllRecv);
	pwin->call_function("DebugLog", "\n-----------------------------\n");
	pwin->call_function("DebugLog", sciter::value(allRecv));
	pwin->call_function("DebugLog", "\n-----------------------------\n");
#ifdef  _DEBUG
	delete[] temp;
#endif
	if ((allRecv.length() != 0) && (allRecv.find(L"online") != allRecv.npos))
	{
		cr->m_status.onlinePlayer = GetJsonFieldFromJsonString(GetJsonFieldFromJsonString(allRecv, "players"),"online").c_str();
		cr->m_status.maxPlayer = GetJsonFieldFromJsonString(GetJsonFieldFromJsonString(allRecv, "players"), "max").c_str();
		wstring motdJson = GetJsonFieldFromJsonString(allRecv, "description");
		cr->m_status.motd += GetJsonFieldFromJsonString(motdJson, "text");
		if (cr->m_status.motd == L"\"\"") //�е�ʱ����������ص�textΪ�գ����ʱ��ȡ����json�ı���������˫���ţ�����Ĵ��뽫˫����ȥ��
			cr->m_status.motd.clear();
		if (pwin->call_function("GetMotdType", motdJson) == L"array")
		{
			wstring motdExtra = pwin->call_function("TranslateExtraMotd", motdJson).to_string().c_str();
			cr->m_status.motd += motdExtra;
		}
	}
	wstring faviconBase64 = pwin->call_function("GetJsonFieldFromJsonString", allRecv.c_str(), "favicon").to_string().c_str();
	faviconBase64 = faviconBase64.substr(1, faviconBase64.length() - 2);//ȥ����β����
	int iBase64Begin = faviconBase64.find_first_of(',') + 1;
	faviconBase64 = faviconBase64.substr(iBase64Begin, faviconBase64.length() - iBase64Begin + 1);//ȡ��img�е�base64����
	string imgData = base64_decode(WcharToChar(faviconBase64.c_str()));//base64����
	FILE *fp;
	string faviconPath = getCurrentWorkDir() + "\\ServerIcons\\";
	if (_access(faviconPath.c_str(), 0) == -1)//�ļ��в�����ʱ�������ļ���
		_mkdir(faviconPath.c_str());
	faviconPath = faviconPath + serverAddr.c_str() + ".png";//��imgд�뵽�ļ���
	fopen_s(&fp, faviconPath.c_str(), "wb");
	for (int i = 0; i < imgData.length(); i++)
	{
		fprintf_s(fp, "%c", imgData[i]);
	}
	fclose(fp);
	cr->m_status.favicon = faviconPath;
	OutputDebugStringW(faviconBase64.c_str());
	fopen_s(&fp,"test.txt", "w");
	fprintf_s(fp, "%ws", allRecv.c_str());
	fclose(fp);
	closesocket(ClientSocket);
	cr->iGetFlag = cr->FSUCCESSFUL;
	return true;
}

void ThFunc_AsyncGet(CwssRecver *cr,string serverAddr,string serverPort) {
	bool flag = true;
	flag = GetServerInfo(cr, serverAddr, serverPort);
	if (!flag)
	{
		pwin->call_function("AddServerInfo", "ImgErr.jpg", "<div id=\"statusDot\" style=\"display:inline-block;position:relative;left:10px;width:10px;height:10px;border-radius:5px;background:red;\"></div><span style=\"text-align:left;padding-left:20px;display:inline-block;width:40px;\">����</span>", sciter::value(cr->GetLastStateInfo()), sciter::value("/ QWQ/"));
		pwin->call_function("DebugLog", cr->GetLastStateInfo());
		return;
	}
	if (cr->GetOnlinePlayer().length() == 0)
	{
		pwin->call_function("AddServerInfo", "ImgErr.jpg", "<div id=\"statusDot\" style=\"display:inline-block;position:relative;left:10px;width:10px;height:10px;border-radius:5px;background:red;\"></div><span style=\"text-align:left;padding-left:20px;display:inline-block;width:40px;\">����</span>", sciter::value(cr->GetLastStateInfo()), sciter::value("/ QWQ/"));
	}
	else
	{
		wstring playerNum = cr->GetOnlinePlayer() + L" / "+ cr->GetMaxPlayer();
		pwin->call_function("AddServerInfo", sciter::value(cr->GetFavicon()), "<div id=\"statusDot\" style=\"display:inline-block;position:relative;left:10px;width:10px;height:10px;border-radius:5px;background:green;\"></div><span style=\"text-align:left;padding-left:20px;display:inline-block;width:40px;\">����</span>", sciter::value(cr->GetMotd()), playerNum);
	}
}
bool CwssRecver::AsyncGet(string serverAddr, string serverPort)
{
	thread t = thread(ThFunc_AsyncGet,this, serverAddr, serverPort);
	t.detach();
	return true;
}



sciter::value frame::DoTask(sciter::value serverAddr, sciter::value serverPort)
{
	if (!g_cr.init())
	{
		pwin->call_function("DebugLog", g_cr.GetLastStateInfo());
	}
	g_cr.AsyncGet(WcharToChar(serverAddr.to_string().c_str()), WcharToChar(serverPort.to_string().c_str()));
	return sciter::value("End Task");
}



wstring CwssRecver::GetOnlinePlayer()
{
	return m_status.onlinePlayer;
}

wstring CwssRecver::GetMaxPlayer()
{
	return m_status.maxPlayer;
}

wstring CwssRecver::GetMotd()
{
	return m_status.motd;
}

string CwssRecver::GetFavicon()
{
	return m_status.favicon;;
}

string CwssRecver::GetLastStateInfo()
{
	string info;
	switch (iGetFlag)
	{
	case CwssRecver::FSUCCESSFUL:
		info = "һ������";
		break;
	case CwssRecver::FRAW:
		info = "��δ��ʼ��";
		break;
	case CwssRecver::FINITFAILED:
		info = "�׽��ֿ��ʼ��ʧ��";
		break;
	case CwssRecver::FCSFAILED:
		info = "�׽��ִ���ʧ��";
		break;
	case CwssRecver::FGHFAILED:
		info = "��������ʧ��";
		break;
	case CwssRecver::FCONFAILED:
		info = "����ʧ��";
		break;
	case CwssRecver::FSENDFAILED:
		info = "����ʧ��";
		break;
	case CwssRecver::FRECVFAILED:
		info = "����ʧ��";
		break;
	default:
		info = "QWQ���ұ�һ��״̬�жϸ��ѵ���";
		break;
	}
	return info;
}

bool CwssRecver::init()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//���ڼ�⺯��״̬
	if (iResult != 0) {
		string logstr = "�׽��ֿ��ʼ��ʧ�� : ";
		logstr += iResult;
		logstr += " \n";
		pwin->call_function("DebugLog", logstr);
		iGetFlag = FINITFAILED;
		return false;
	}
	ZeroMemory(&m_status, sizeof(m_status));
	iGetFlag = FSUCCESSFUL;
	pwin->call_function("DebugLog", "�ѳ�ʼ���׽��ֿ�\n");
	return true;
}
