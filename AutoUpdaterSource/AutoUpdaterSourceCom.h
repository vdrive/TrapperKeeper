#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class AutoUpdaterSourceDll;

class AutoUpdaterSourceCom :
	public TKComInterface
{
public:
	AutoUpdaterSourceCom(void);
	~AutoUpdaterSourceCom(void);

	void InitParent(AutoUpdaterSourceDll *parent);

	AutoUpdaterSourceDll *p_parent;
	void GetDLLs();
//	bool CheckDlls();

	void DataReceived(char *source_ip, void *data, UINT data_length);
	
private:
		
	vector<CString> v_dll_strs;
//	vector<AppID> v_dll_list;
};
