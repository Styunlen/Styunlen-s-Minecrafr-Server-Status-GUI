#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <thread>
#include <direct.h>
#include "base64.h"
#include "stringTools.h"
#include "sciter-x-window.hpp"
#include <io.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

char * WsGetErrorInfo();
string pack_varint(int d);
string pack_data(string d);



class frame : public sciter::window {
public:
	BEGIN_FUNCTION_MAP
		FUNCTION_3("DoTask", DoTask);
	END_FUNCTION_MAP
	sciter::value DoTask(sciter::value serverAddr, sciter::value serverPort, sciter::value isRefresh/* �Ƿ�Ϊˢ�²��� */);
	frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}
};

extern frame *pwin;

class CwssRecver
{
public:
	CwssRecver();
	~CwssRecver();
	bool AsyncGet(string serverAddr, string serverPort, bool isRefresh);
	void SetFlag(int i);
	wstring GetOnlinePlayer();
	wstring GetMaxPlayer();
	wstring GetMotd();
	string GetLastStateInfo();
	string GetFavicon();
	bool init();
	friend void ThFunc_AsyncGet(CwssRecver *cr, string serverAddr, string serverPort, bool isRefresh/* �Ƿ�Ϊˢ�²��� */);
	friend bool GetServerInfo(CwssRecver *cr, string serverAddr, string serverPort);
private:
	struct { 
		wstring maxPlayer;
		wstring onlinePlayer;
		wstring motd;
		string favicon;
	} m_status;
	bool is_inited = false;
	/*��־��ȡ״̬
		FSUCCESSFUL,	һ������
		FRAW,			��ճ�ʼ��ʱ��״̬
		FINITFAILED,	�׽��ֿ��ʼ��ʧ��
		FCSFAILED,		�����׽���ʧ��
		FGHFAILED,		��������ʧ��
		FCONFAILED,		����ʧ��
		FSENDFAILED,	����ʧ��
		FRECVFAILED,	����ʧ��
	*/
	enum { FSUCCESSFUL, FRAW, FINITFAILED, FCSFAILED, FGHFAILED, FCONFAILED, FSENDFAILED, FRECVFAILED} iGetFlag = FRAW;
};

/* Sciter�Ľű��Ѿ��ṩ����෽��������ļ�����ǰ���֮��ķ���������
��˼��˵��˵�һЩ����ʵ���������ڸ��ӣ���������ǰ�˵Ľű����������ʵ��һЩ���ӹ���
����JSON�ַ����Ķ�ȡ
*/

wstring GetJsonFieldFromJsonString(string json, string jsonField);
wstring GetJsonFieldFromJsonString(wstring json, string jsonField);