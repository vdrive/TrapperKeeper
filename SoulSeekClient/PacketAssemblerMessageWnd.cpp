// PacketAssemblerMessageWnd.cpp

#include "stdafx.h"
#include "PacketAssemblerMessageWnd.h"
#include "PacketAssembler.h"
#include "PacketIO.h"

BEGIN_MESSAGE_MAP(PacketAssemblerMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_MESSAGE(WM_PA_SEND_RESULTS,ReturnResults)
	ON_MESSAGE(WM_PA_WRITE_TO_LOG,WriteToLog)
	ON_MESSAGE(WM_PA_POST_TO_FILE,WriteToFile)
END_MESSAGE_MAP()

//
//
//
PacketAssemblerMessageWnd::PacketAssemblerMessageWnd()
{
	p_mod=NULL;
}

//
//
//
PacketAssemblerMessageWnd::~PacketAssemblerMessageWnd()
{
}

//
//
//
void PacketAssemblerMessageWnd::InitParent(PacketAssembler *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT PacketAssemblerMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT PacketAssemblerMessageWnd::ReturnResults(WPARAM wparam,LPARAM lparam)
{
	p_mod->ReturnResults((PacketWriter*) wparam);
	return 0;
}

LRESULT PacketAssemblerMessageWnd::WriteToLog(WPARAM wparam,LPARAM lparam)
{
	if(wparam != NULL)
		p_mod->WriteToLog((char *) wparam, (int) lparam);
	return 0;
}
LRESULT PacketAssemblerMessageWnd::WriteToFile(WPARAM wparam,LPARAM lparam)
{
	if(wparam != NULL)
		p_mod->WriteToLogFile((char *) wparam);
	return 0;
}