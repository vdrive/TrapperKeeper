#pragma once
#include "..\sampleplugin\dll.h"
#include "..\TKCom\TKComInterface.h"
#include "PioletPoisonerDlg.h"
#include "..\StatusSource\MyComInterface.h"
#include <pdh.h>
#include "ConnectionModule.h"
#include "..\PioletSpoofer\PoisonEntry.h"
#include "..\PioletSpoofer\PoisonEntries.h"
#include "..\DCMaster\DataBaseInfo.h"
#include <afxmt.h>	// for CCriticalSection


class PioletPoisoner :	public Dll
{
public:
	PioletPoisoner(void);
	~PioletPoisoner(void);
	
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	
	//override function to receive data from the Interface
	//void DataReceived(char *source_name, void *data, int data_length);
	void OnTimer(UINT nIDEvent);
	void Log(const char *text,COLORREF color=0xFFFFFFFF,bool bold=false,bool italic=false);
	//PoisonEntry GetPoisonEntry(char* filename);
	void OnReceivedFileRequest(PoisonEntry* pe);

	CPioletPoisonerDlg m_dlg;
	//UINT m_max_poison_entries;

private:
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
//	void ClearOldPoisonEntries();

	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;

	ConnectionModule* p_connection_module;
	//MyComInterface m_com;
	//vector<PoisonEntry*> v_poison_entries;
	//DataBaseInfo m_processed_db_info;
	CCriticalSection m_critical_section;
};
