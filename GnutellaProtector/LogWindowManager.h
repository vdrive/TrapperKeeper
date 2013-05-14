// LogWindowManager.h
#pragma once

#include "Manager.h"
#include "LogDlg.h"

class GnutellaProtectorDll;

class LogWindowManager : public Manager
{
public:
	LogWindowManager();
	~LogWindowManager();
	void InitParent(GnutellaProtectorDll *parent);

	void ShowWindow();

	void Log(char *text,COLORREF color=0,bool bold=false,bool italic=false);

private:
	CLogDlg m_dlg;
};