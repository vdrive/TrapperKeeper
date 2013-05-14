#include "StdAfx.h"
#include "BTSeedInflatorDll.h"
#include "BTIPDataHeader.h"
//
//
//
BTSeedInflatorDll::BTSeedInflatorDll(void)
{

	m_num_connections = 10;
}

//
//
//
BTSeedInflatorDll::~BTSeedInflatorDll(void)
{
}

//
//
//
void BTSeedInflatorDll::DllInitialize()
{
	//m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void BTSeedInflatorDll::DllUnInitialize()
{

	int size = (int)v_threads.size();
	for (int j = 0; j < size; j++)
	{
		v_threads[0]->StopThread();
		v_threads.erase(v_threads.begin());
	}


}

//
//
//
void BTSeedInflatorDll::DllStart()
{

	p_com = new BTSeedInflatorCom();

	p_com->InitParent(this);
	p_com->Register(this,6824);

	srand( (unsigned)time( NULL ) );

}

//
//
//
void BTSeedInflatorDll::DllShowGUI()
{
	//m_dlg.ShowWindow(SW_NORMAL);
	//m_dlg.BringWindowToTop();
}

//
//
//

UINT InflateSeedThreadProc(LPVOID pParam)
{

	BTIPThreadData * tempdata = (BTIPThreadData *)pParam;
	if (tempdata == NULL)
	{
		return 1;
	}


	BTIPList ips;
	BTIP tempip;

	TrackerConnection tracker;

	bool done = false;

	int iter = 0;
	int failure = 0;

	while (!done)
	{
		
		bool sucess = tracker.GetPeers(&tempdata->m_data,"");


		// if it can't connect try again a few times
		if (sucess == false)
		{
			failure += 1;
			// after 30 failures give up kill the thread
			if (failure > 30)
			{
				done = true;
				return 30;
			}
		}

		iter +=1;
		//2 minutes * 720 = 24 hours
		if (iter >= 720 * 5)
		{
			done = true;
			return 720;
		}

		Sleep(3*60*1000);


	}


//	delete pParam;
	return 0;
}


void BTSeedInflatorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	BTIPDataHeader * header = (BTIPDataHeader *)data;
	switch(header->appcode)
	{
		case BTIPDATA_LOW_SEED_TORRENT:	
		{

			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			BTIPThreadData * tempdata = new BTIPThreadData;
			tempdata->m_data.ReadFromBuffer(ptr);
			tempdata->p_com = p_com;
			tempdata->source_name = source_name;

			int found = 0;
			
			for (int i = 0; i< (int)v_torrents.size(); i++)
			{
				if (tempdata->m_data.m_torrent_id == v_torrents[i].m_torrent_id)
				{
					found = 1;
					break;
				}
				
			}
			
			if (found == 0)
			{
				v_torrents.push_back(tempdata->m_data);
				for (int j = 0; j < m_num_connections; j++)
				{
				//	AfxBeginThread(InflateSeedThreadProc,tempdata,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
					InflationThread * newthread = new InflationThread;
					newthread->MakeNewConnection(tempdata->m_data,tempdata->source_name);
					v_threads.push_back(newthread);
				}
			}
			break;
		}

		case BTIPDATA_STOP_SINGLE_INFLATION:
		{
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			int  torrent_id = *((int *)ptr);
			for (int i = 0; i< (int)v_torrents.size(); i++)
			{
				if (torrent_id == v_torrents[i].m_torrent_id)
				{
					v_torrents.erase(v_torrents.begin()+i);
					break;
				}
				
			}

			for (int j = 0; j < (int)v_threads.size(); j++)
			{
				if (torrent_id == v_threads[j]->GetID())
				{
					//v_threads[j]->StopThread();
					v_threads[j]->b_killThread = 1;
				}

			}
		
			break;
		}

	}

	
 }

