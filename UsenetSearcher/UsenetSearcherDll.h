#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "UsenetSearcherDlg.h"
#include "UsenetSocket.h"
#include "ProjectKeywords.h"
#include "UsenetSearcherCom.h"
#include "SMTPSocket.h"

class UsenetSearcherDll :	public Dll
{
public:

	UsenetSearcherDll(void);
	~UsenetSearcherDll(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void Log(char *source, char *buf);
	void GetProjects();
	void SwitchServers(char *buf);

	int m_server_index;

	CTime m_last_project_update;

	vector<ProjectKeywords> v_project_keywords;
	vector<UsenetSocket *> v_usenet_socket_ptrs;

	SMTPSocket m_smtp_socket;
	UsenetSearcherCom m_com;
	HWND m_dlg_hwnd;

private:

	UsenetSearcherDlg m_dlg;
};
