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
#include "sciter-x-window.hpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

char * WsGetErrorInfo();
string pack_varint(int d);
string pack_data(string d);



class frame : public sciter::window {
public:
	BEGIN_FUNCTION_MAP
		FUNCTION_2("DoTask", DoTask);
	END_FUNCTION_MAP
	sciter::value  DoTask(sciter::value serverAddr, sciter::value serverPort);
	frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}
};

extern frame *pwin;

class CwssRecver
{
public:
	CwssRecver();
	~CwssRecver();
	bool AsyncGet(string serverAddr, string serverPort);
	void SetFlag(int i);
	string GetOnlinePlayer();
	string GetMaxPlayer();
	string GetMotd();
	string GetLastStateInfo();
	string GetFavicon();
	bool init();
	friend void ThFunc_AsyncGet(CwssRecver *cr, string serverAddr, string serverPort);
	friend bool GetServerInfo(CwssRecver *cr, string serverAddr, string serverPort);
private:
	struct { 
		string maxPlayer;
		string onlinePlayer;
		string motd;
		string favicon;
	} m_status;
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