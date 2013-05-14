// SamplePlugInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCSupplyDlg.h"
#include "DllInterface.h"
#include "Mmsystem.h"	//needed for timegettime()

// DCSupplyDlg dialog

IMPLEMENT_DYNAMIC(DCSupplyDlg, CDialog)
DCSupplyDlg::DCSupplyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DCSupplyDlg::IDD, pParent)
{
}

DCSupplyDlg::~DCSupplyDlg()
{
}

void DCSupplyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, StatusListBox);
	DDX_Control(pDX, IDC_EDIT1, m_connected_hubs);
	DDX_Control(pDX, IDC_LIST2, db_activity);
	DDX_Control(pDX, IDC_EDIT2, m_matches);
}


BEGIN_MESSAGE_MAP(DCSupplyDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCLOSE, OnBnClickedClose)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL DCSupplyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//initialize the hub list collector's window
	m_listcollector.InitDlg(this);
	m_listcollector.DownloadHubList();	//downloads the hublist
	m_hublist_position=0;
	m_search_position=0;
	m_num_matches=0;
	m_loops = 0;
	b_projects_changed=true;
	b_hub_connect = true;	//connect to hubs
	m_matches.SetWindowText("0");
	int socket=this->CreateRandomSocket();
	//initialize the hub sockets
	for(int i=0;i<SCANNERS;i++)
	{
		hubsocket[i].InitDlg(this);
		hubsocket[i].SetSocket(socket);		//sets socket that'll receive data
		hubsocket[i].GetDownloadManager(m_download_manager);
	}
	resultant.InitSocket(socket);
	resultant.SetDialog(this);		//set dialog access to udpsearchresult object
	//SetTimer(0,5000,NULL);	//TIMER INTERVAL FOR EACH HUB CONNECT CHECKS
	SetTimer(1,15000,NULL);	//TIMER INTERVAL FOR EACH SEARCH
	SetTimer(2,60*1000*60*24,NULL);

	return TRUE;
}

//creates a random socket each time it's at start up
int DCSupplyDlg::CreateRandomSocket()
{
	srand(timeGetTime());
	return rand()%9000 + 500;
}
//prints out the status to the listbox
void DCSupplyDlg::StatusUpdate(char *post_data)
{
	StatusListBox.InsertString(0,post_data);
}

void DCSupplyDlg::DBUpdate(char *post_data)
{
	char buffer[100];
	m_num_matches++;
	m_matches.SetWindowText(itoa(m_num_matches,buffer,10));
	db_activity.InsertString(0,post_data);

}
//hub list has been received
void DCSupplyDlg::ReceivedHubList(vector<Hub> hub_list)
{
	CString output;
	output.Format("List contains %d servers!!!",hub_list.size());
	this->StatusUpdate("List Received");
	this->StatusUpdate(output.GetBuffer(output.GetLength()));
	this->StatusUpdate("Starting Hub Scanning...");
	m_hub_list=hub_list;
}
//
//
//

//Timer that is called every 5 seconds to do status checks on hub sockets
void DCSupplyDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	//::AfxMessageBox("TIMER");
	/*int x=0;
	int hubs_connected=0;
	bool conducted_search = false;
	if(nIDEvent==2 || b_projects_changed)	//if projects have changed get the new vector
	{
		if(p_pm->HaveProjectsChanged())
		{
			m_keywords_vector=&p_pm->m_project_keywords_vector;
		}
		b_projects_changed=false;
		return;
	}
	if(m_keywords_vector->v_projects.size()>0)
	{
		//	TRACE("%d Projects",p_pm->m_project_keywords_vector.v_projects.size());
		if(m_hublist_position>=signed(m_hub_list.size()))
			m_hublist_position=0;
		while(x<SCANNERS && m_hublist_position<(signed)m_hub_list.size())
		{
			Hub current_hub=m_hub_list[m_hublist_position];
			//set a default port number to 411
			if(!hubsocket[x].GetConnected() && m_hub_list[m_hublist_position].m_connected!=true && b_hub_connect==true)
			{
				hubsocket[x].Close();
				if(hubsocket[x].HubConnect(current_hub.m_address,411)==-1)
				{
					m_hub_list[m_hublist_position].m_connected=false;
				}
				else
				{
					m_hub_list[m_hublist_position].m_connected=true;
				}

				//add something like\
				//hubsocket[x].
				//hubsocket[x].HubConnect("wirefire.no-ip.com",411);
				//hubsocket[x].HubConnect("monkeys.no-ip.com",411);
				//hubsocket[x].HubConnect("starglowone.dynu.net",411);
				//hubsocket[x].HubConnect("renate.bounceme.net",10000);
				//hubsocket[x].HubConnect("budweiser420.servebeer.com",411);
				//hubsocket[x].HubConnect(" woodsrealm.utbnet.com",711);
				m_hublist_position++;	//increment position in list
			}
			else
			{
				if(hubsocket[x].GetSearch() && nIDEvent==1 && hubsocket[x].GetConnected())	//okay to start searching
				{
					//TRACE("DCSupply DCSupplyDlg::OnTimer()\n");
					if((UINT)m_search_position<m_keywords_vector->v_projects.size())
					{
						hubsocket[x].Search(m_keywords_vector->v_projects[m_search_position].m_supply_keywords.m_search_string.c_str());
						char searching_for[100]="Searching for ";
						this->StatusUpdate(strcat(strcat(searching_for,(char*) m_keywords_vector->v_projects[m_search_position].m_supply_keywords.m_search_string.c_str()),hubsocket[x].GetConnectedHost()));
						resultant.SetProjectData(m_keywords_vector->v_projects[m_search_position].m_supply_keywords,(char*)m_keywords_vector->v_projects[m_search_position].m_project_name.c_str());
						conducted_search= true;
						hubs_connected++;		//increment the hubs connected
					}
					else
					{
						//kill all timers
						//KillTimer(0);
						//KillTimer(1);
						//KillTimer(2);
						//::AfxMessageBox("Finished searching all projects");
						for(int j=0;j<SCANNERS;j++)
						{
							hubsocket[j].Disconnect();	//disconnect from current hub
						}
						b_hub_connect=true;
						return;

					}
						//hubsocket[x].Search("vicente fernandez");
				}
			}
			
			x++;	//increment hubsocket
		}
		if(conducted_search)	//flag search was done
		{
			m_search_position++;	//increment the position of the search
			conducted_search = false;
			b_hub_connect = false;
			char buffer[10];
			m_connected_hubs.SetWindowText(itoa(hubs_connected,buffer,10));
		}
	}
*/
	if(nIDEvent==2 || b_projects_changed)	//if projects have changed get the new vector
	{
		if(p_pm->HaveProjectsChanged())
		{
			m_keywords_vector=&p_pm->m_project_keywords_vector;
		}
		b_projects_changed=false;
		if(nIDEvent==2)	//set timer again
		{
			resultant.DCMasterInsert();
			SetTimer(1,15000,NULL);	//TIMER INTERVAL FOR EACH SEARCH
		}
		return;
	}
	if((UINT)m_keywords_vector->v_projects.size()>0 && m_hub_list.size()>0)
	{
		
		//after three cycles
		if(m_loops>3)
		{
			//insert all the data collected into the dc master database
			//resultant.DCMasterInsert();
			KillTimer(1);
			//::AfxMessageBox("Done Searching");
			//KillTimer(2);
			b_hub_connect = true;
			m_loops = 0;	//reset the loop counter
			return;
		}
		if(b_hub_connect)
		{
			char buffer[10];
			m_connected_hubs.SetWindowText(itoa(0,buffer,10));
			m_search_position = 0; //reset the search index 
			m_loops++;
			ConnectAllHubs();
			b_hub_connect = false;
			SetTimer(1,20000,NULL);	//TIMER INTERVAL FOR EACH SEARCH
		}
		if(nIDEvent==1)
		{
			SearchConnectedHubs();
			HubRetry();
			m_download_manager.RemoveDownloads();
		}
		if((UINT)m_search_position>=m_keywords_vector->v_projects.size())
		{
			//clear all content
			db_activity.ResetContent();
			StatusListBox.ResetContent();
			DisconnectAll();	//disconnect all hubs
			b_hub_connect = true;	//start with next batch of hubs
		}
	}
	CDialog::OnTimer(nIDEvent);
}

//Try to connect to all hubs, return 1 if attempted to connect, return 0 if there was some type of error
void DCSupplyDlg::ConnectAllHubs()
{
	for(int x=0;x<SCANNERS;x++)
	{
		Hub current_hub=m_hub_list[m_hublist_position];
		if(!hubsocket[x].GetConnected() && m_hub_list[m_hublist_position].m_connected!=true)
			{
				hubsocket[x].Close();
				if(hubsocket[x].HubConnect(current_hub.m_address,411)==-1)
				{
					m_hub_list[m_hublist_position].m_connected=false;
				}
				else
				{
					m_hub_list[m_hublist_position].m_connected=true;
				}
				hubsocket[x].GetRetryHubObject(v_retry_hub);
				m_hublist_position++;	//increment position in list
			}
	}
}

//search all the hubs connected and return the number of hubs that are connected
int DCSupplyDlg::SearchConnectedHubs()
{
	bool conducted_search = false;
	int hubs_connected = 0;	//reset hubs connected
	for(int x=0;x<SCANNERS;x++)
	{
		if(hubsocket[x].GetSearch() && hubsocket[x].GetConnected())	//okay to start searching
		{
				hubsocket[x].Search(m_keywords_vector->v_projects[m_search_position].m_supply_keywords.m_search_string.c_str());
				char searching_for[100]="Searching for ";
				this->StatusUpdate(strcat(strcat(searching_for,(char*) m_keywords_vector->v_projects[m_search_position].m_supply_keywords.m_search_string.c_str()),hubsocket[x].GetConnectedHost()));
				resultant.SetProjectData(m_keywords_vector->v_projects[m_search_position].m_supply_keywords,(char*)m_keywords_vector->v_projects[m_search_position].m_project_name.c_str());
				conducted_search= true;
				hubs_connected++;		//increment the hubs connected
		}
	}
	if(conducted_search)	//flag search was done
	{
		m_search_position++;	//increment the position of the search
		conducted_search = false;
		b_hub_connect = false;
		char buffer[10];
		m_connected_hubs.SetWindowText(itoa(hubs_connected,buffer,10));
	}
	return hubs_connected;
}
//fix only connect to already tried hubs
void DCSupplyDlg::HubRetry()
{
	//vector<Hub> HubRetry;
	vector<Hub>::iterator iter;

	/*UINT y = 0;
	UINT size = (UINT)m_hub_list.size();
	for(UINT x=0;x<size;x++)
	{
		if(m_hub_list[x].m_connected == false)
		{
			HubRetry.push_back(m_hub_list[x]);
		}
	}*/

	//use sockets that aren't connected to try and connected unconnected hubs
	TRACE("Retry Hubs: %d\n", v_retry_hub.size());
	iter = v_retry_hub.begin();
	for(UINT x = 0;x<SCANNERS;x++)
	{
		if(iter!=v_retry_hub.end())
		{
			if(!hubsocket[x].GetConnected())
			{
				hubsocket[x].Close();
				hubsocket[x].Connect((*iter).m_address,411);
				iter++;	//increment the retry hub index
			}
		}
		else
		{
			return;
		}

	}

	//for(iter = v_retry_hub.begin();iter != v_retry_hub.end() ; iter++)
	//{
		v_retry_hub.clear();
		//v_retry_hub.erase(iter);	//erase all hubs that couldn't connect 
	//}
}
//disconnect from all hubs
void DCSupplyDlg::DisconnectAll()
{
	for(int i = 0;i<SCANNERS;i++)
	{
		hubsocket[i].Disconnect();
	}
	for(UINT i = 0;i<m_hub_list.size(); i++)
	{
		m_hub_list[i].m_connected=false;
	}
}
void DCSupplyDlg::OnBnClickedClose()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_HIDE);
}

void DCSupplyDlg::SetProjectManager(ProjectManager &pm)
{
	p_pm=&pm;	//set project manager
}

void DCSupplyDlg::OnDestroy()
{
	//kill all timers
	KillTimer(0);
	KillTimer(1);
	KillTimer(2);
	this->DisconnectAll();
	CDialog::OnDestroy();
}
