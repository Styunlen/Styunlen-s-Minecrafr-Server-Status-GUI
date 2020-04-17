#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include "sciter-x-window.hpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

char * WsGetErrorInfo();
string pack_varint(int d);
string pack_data(string d);



class frame : public sciter::window {
public:
	BEGIN_FUNCTION_MAP
		FUNCTION_0("DoTask", DoTask);
	END_FUNCTION_MAP
	sciter::value  DoTask();
	frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}
};

extern frame *pwin;

class CwssRecver
{
public:
	CwssRecver();
	~CwssRecver();
	bool AsyncGet();
	string GetOnlinePlayer();
	string GetMaxPlayer();
	string GetMotd();
	string GetLastStateInfo();
	bool init();
private:
	struct { 
		string maxPlayer;
		string onlinePlayer;
		string motd;
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