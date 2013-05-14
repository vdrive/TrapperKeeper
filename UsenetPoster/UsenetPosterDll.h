#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "UsenetPosterDlg.h"
#include "UsenetPosterCom.h"
#include "UsenetSocket.h"
#include "SMTPSocket.h"

class UsenetSocket;

class UsenetPosterDll :	public Dll
{
public:
	UsenetPosterDll(void);
	~UsenetPosterDll(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void Log(char *log_source, char *buf);
	void StartPosting(UsenetSocket *socket_ptr);
	void GetServerAccounts();

	void ServerBanned(UsenetSocket *socket_ptr, char *buf);

	vector<UsenetSocket *> v_usenet_socket_ptrs;

	UsenetPosterDlg m_dlg;
	SMTPSocket m_smtp_socket;


private:
	UsenetPosterCom m_com;
	
};
