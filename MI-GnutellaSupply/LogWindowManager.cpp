// LogWindowManager.cpp

#include "stdafx.h"
#include "LogWindowManager.h"
#include "GnutellaSupplyDll.h"

//
//
//
LogWindowManager::LogWindowManager()
{
}

//
//
//
LogWindowManager::~LogWindowManager()
{
	m_dlg.DestroyWindow();
}

//
//
//
void LogWindowManager::InitParent(GnutellaSupplyDll *parent)
{
	Manager::InitParent(parent);

	// Create the log window
	m_dlg.Create(IDD_Log_Dialog,&p_parent->m_dlg);
}

//
//
//
void LogWindowManager::ShowWindow()
{
	m_dlg.CenterWindow();

	// Hiding and showing brings it to the front
	m_dlg.ShowWindow(SW_HIDE);
	m_dlg.ShowWindow(SW_SHOW);
}

//
//
//
void LogWindowManager::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	m_dlg.Log(text,color,bold,italic);
}