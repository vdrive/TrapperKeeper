// PioletPoisoner.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "PioletPoisoner.h"
#include "WSocket.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()
#include "..\PioletSpoofer\PioletSpooferHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE PioletPoisonerDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("PioletPoisoner.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(PioletPoisonerDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(PioletPoisonerDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("PioletPoisoner.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(PioletPoisonerDLL);
	}
	return 1;   // ok
}


PioletPoisoner::PioletPoisoner(void)
{
}

PioletPoisoner::~PioletPoisoner(void)
{
}

//
//
//
void PioletPoisoner::DllInitialize()
{
	//m_max_poison_entries = 100000;
	BOOL ret=AfxInitRichEdit();	// for rich edit in the log window
	WSocket::Startup(); // Start Windows Sockets
	m_dlg.Create(IDD_PIOLET_POISONER_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
}

//
//
//
void PioletPoisoner::DllUnInitialize()
{
	WSocket::Cleanup();
	ClosePerformanceCounters();
	m_dlg.DestroyWindow();
	delete p_connection_module;
	
/*	vector<PoisonEntry*>::iterator iter = v_poison_entries.begin();
	while(iter != v_poison_entries.end())
	{
		delete *iter;
		v_poison_entries.erase(iter);
	}
*/
}

//
//
//
void PioletPoisoner::DllStart()
{
	InitPerformanceCounters();

	p_connection_module = new ConnectionModule(this);
	p_connection_module->InitParent(this);

	m_proc_percent_usage=0;
	m_bandwidth_in_bytes=0;
	m_current_bandwdith_in_bits_per_second=0;
	m_mem_usage=0;

	//m_com.Register(this, 35);

	m_dlg.SetTimer(1,1*1000,0);	// set 1 second timer
	m_dlg.SetTimer(2,60*1000,0); //1 min
}

//
//
//
void PioletPoisoner::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
/*
void PioletPoisoner::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
//	char log[1024];
	char* ptr = (char*)data;
	PioletSpooferHeader* header = (PioletSpooferHeader*)ptr;
	ptr+= sizeof(PioletSpooferHeader);
	switch (header->op)
	{
		case PioletSpooferHeader::Poisoner_Init:
		{
			PioletSpooferHeader header;
			header.op = PioletSpooferHeader::Poisoner_Init_Response;
			header.size = 0;
			m_com.SendReliableData(source_name, &header, sizeof(header));
			break;
		}
		
		case PioletSpooferHeader::PoisonEntry:
		{
			PoisonEntries entries;
			entries.ReadFromBuffer(ptr);

			CSingleLock singleLock(&m_critical_section);
			singleLock.Lock(INFINITE);
			if(singleLock.IsLocked())
			{
				
				for(UINT i=0; i<entries.v_poison_entries.size(); i++)
				{
					PoisonEntry* pe = new PoisonEntry();
					*pe = entries.v_poison_entries[i];
					v_poison_entries.push_back(pe);
				}
				if(v_poison_entries.size() > m_max_poison_entries + 1000)
				{
					while(v_poison_entries.size() > m_max_poison_entries)
					{
						vector<PoisonEntry*>::iterator iter = v_poison_entries.begin();
						delete (*iter);
						v_poison_entries.erase(iter);
					}
				}				
				singleLock.Unlock();
			}
			break;
		}
		
		case PioletSpooferHeader::Poisoner_DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}

	}
}
*/
//
//
//
void PioletPoisoner::OnTimer(UINT nIDEvent)
{
	m_dlg.KillTimer(nIDEvent);
	switch(nIDEvent)
	{
		case 1:	// 1 second timer
		{
			CheckPerformanceCounters();
			p_connection_module->TimerHasFired();
			m_dlg.SetTimer(1,1000,NULL);
			break;
		}
		case 2: //1 min
		{
			//ClearOldPoisonEntries();
			p_connection_module->OneMinuteTimer();
			m_dlg.SetTimer(2,60*1000,NULL);
		}
	}
}

//
//
//
void PioletPoisoner::InitPerformanceCounters()
{
	UINT timer_ret=1;

	int i;

	char iai_buf[1024];
	DWORD iai_buf_len=1024;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;

	GetAdaptersInfo(iai,&iai_buf_len);

	// Remove (,) and / from the description of the interface
	while(strchr(iai->Description,'(')!=NULL)
	{
		*strchr(iai->Description,'(')='[';
	}
	while(strchr(iai->Description,')')!=NULL)
	{
		*strchr(iai->Description,')')=']';
	}
	while(strchr(iai->Description,'/')!=NULL)
	{
		*strchr(iai->Description,'/')='_';
	}

	m_keynames[0]="\\Processor(0)\\% Processor Time";
	m_keynames[1]="\\Network Interface(";
	m_keynames[1]+=iai->Description;
	m_keynames[1]+=")\\Bytes Total/sec";
	m_keynames[2]="\\Network Interface(";
	m_keynames[2]+=iai->Description;
	m_keynames[2]+=")\\Current Bandwidth";

    m_pdh=0;

    // Create the pdh query
    if(PdhOpenQuery(NULL,0,&m_pdh))
	{
		MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
		return;
	}

    // ADD A COUNTER TO THE QUERY
	for(i=0;i<3;i++)
	{
		PDH_STATUS error=PdhAddCounter(m_pdh,m_keynames[i].c_str(),NULL,&m_pdh_counters[i]);

		if(error)
		{
			MessageBox(NULL,"Error adding counter to the pdh query","Error",MB_OK);
			return;
		}
	}
    
//	m_dlg.SetTimer(6,TIMER_LENGTH*1000,0);
}

//
//
//
void PioletPoisoner::CheckPerformanceCounters()
{
	UINT i;

    // Collect the pdh query data
	if(PdhCollectQueryData(m_pdh))
	{
		MessageBox(NULL,"Error collecting pdh query data","Error",MB_OK);
		return;
	}

	// Check the processor usage counter
	PDH_FMT_COUNTERVALUE value;
    PDH_STATUS error;

	error=PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox("Error formatting pdh counter data");
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_proc_percent_usage=value.longValue;

/*
	// If less than 50 % kick off another mod if there is less than min count of them
	if((m_proc_percent_usage<50) & (m_connection_manager.ReturnModCount()<(UINT)m_min_module_count))	// was 60
	{
		m_connection_manager.AddModule();

		char log[1024];
		sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)\n",
			m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
		m_log_window_manager.Log(log,0x00888888);
	}
*/
	// Check the bandwidth counter
	error=PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox("Error formatting pdh counter data");
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_bandwidth_in_bytes=value.longValue;

	// Check the current bandwidth in bits per second
	error=PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value);
	if(error)
	{
		return;
	}

	m_current_bandwdith_in_bits_per_second=value.longValue;	// this should be 100 Mbps
	
	double bps=m_bandwidth_in_bytes*8;

	// Measure the processes mem usage
	HANDLE hHandle;
	DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS);
	
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
	GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));

	CloseHandle(hHandle);

	m_mem_usage=(UINT)pmc.WorkingSetSize;	// in bytes

	// Put commas in the mem measurement value
	char mem_buf[100];
	string mem;
	_itoa(m_mem_usage/1024,mem_buf,10);
	strrev(mem_buf);
	for(i=0;i<strlen(mem_buf);i++)
	{
		if((i>0) && (i%3==0))
		{
			mem+=",";
		}
		
		mem+=mem_buf[i];
	}
	strcpy(mem_buf,mem.c_str());
	strrev(mem_buf);
	
	// Display the status
	char buf[1024];
	sprintf(buf,"[ %u %% ] - [ %.2f KB/sec - %.2f %% ] - [ %s KB ]",m_proc_percent_usage,((double)m_bandwidth_in_bytes)/1024,
		(100*bps)/m_current_bandwdith_in_bits_per_second,mem_buf);
	m_dlg.GetDlgItem(IDC_Computer_Status_Static)->SetWindowText(buf);

//	sprintf(buf, "Poison Entries Size: %u", v_poison_entries.size());
//	m_dlg.GetDlgItem(IDC_POISON_ENTRIES_SIZE_STATIC)->SetWindowText(buf);

}

//
//
//
void PioletPoisoner::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void PioletPoisoner::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	m_dlg.Log(text, color, bold, italic);
}

//
//
//
/*
PoisonEntry PioletPoisoner::GetPoisonEntry(char* filename)
{
	PoisonEntry pe;
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock(INFINITE);
	if(singleLock.IsLocked())
	{
		//searching from the end
		if(v_poison_entries.size() > 0)
		{
			vector<PoisonEntry*>::iterator iter = v_poison_entries.end()-1;
			while(iter >= v_poison_entries.begin())
			{
				if(stricmp(filename, (*iter)->m_filename.c_str())==NULL)
				{
					pe = *(*iter);
					break;
				}
				if(iter == v_poison_entries.begin())
					break;
				else
					iter--;
			}
		}
		singleLock.Unlock();
	}
	return pe;
}
*/

//
//
//
/*
void PioletPoisoner::ClearOldPoisonEntries()
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock(INFINITE);
	if(singleLock.IsLocked())
	{
		vector<PoisonEntry*>::iterator iter = v_poison_entries.begin();
		while(iter != v_poison_entries.end())
		{
			CTimeSpan ts = CTime::GetCurrentTime() - (*iter)->m_timestamp;
			LONGLONG seconds_elapse = ts.GetTotalSeconds();
			if(seconds_elapse > 60*60) //60 mins
			{
				delete (*iter);
				v_poison_entries.erase(iter);
			}
			else
				break;
		}
		singleLock.Unlock();
	}
}
*/

//
//
//
void PioletPoisoner::OnReceivedFileRequest(PoisonEntry* pe)
{
	p_connection_module->OnReceivedFileRequest(pe);
}
