// PacketAssemblerMessageWnd.h

#pragma once

#define WM_INIT_THREAD_DATA				WM_USER+1
#define WM_PA_SEND_RESULTS				WM_USER+2
#define WM_PA_WRITE_TO_LOG				WM_USER+3
#define WM_PA_POST_TO_FILE				WM_USER+4

class PacketAssembler;

class PacketAssemblerMessageWnd : public CWnd
{
public:
	PacketAssemblerMessageWnd();
	~PacketAssemblerMessageWnd();
	void InitParent(PacketAssembler *mod);

	LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT ReturnResults(WPARAM wparam,LPARAM lparam);

	LRESULT WriteToLog(WPARAM wparam,LPARAM lparam);
	LRESULT WriteToFile(WPARAM wparam,LPARAM lparam);

	PacketAssembler *p_mod;

private:
	DECLARE_MESSAGE_MAP()
};