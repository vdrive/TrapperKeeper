// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
#include "VendorCount.h"
#include "ConnectionModuleMessageWnd.h"
#include "iphlpapi.h"
#include "FileSharingManager.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
	m_one_hour_timer=0;
	p_file_sharing_manager = NULL;

	ReadInUserNames();
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}
	
	m_rng.Reseed(true,32);

}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	/*
	vector<PacketBuffer*>::iterator iter = v_shared_files_buffers.begin();
	while(iter != v_shared_files_buffers.end())
	{
		delete *iter;
		v_shared_files_buffers.erase(iter);
	}
	*/

	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
	// close all sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].Close();
	}
}

//
//
//
DWORD ConnectionSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//
void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status_data)
{
	m_one_hour_timer++;
	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].TimerHasFired();
	}

	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_status_data.m_connected_socket_count++;
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}

			// Get the ip of the connection and the connection status of the socket
			if(m_sockets[i].IsConnected())
				m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			//m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	status_data=m_status_data;
	
	m_status_data.ClearCounters();

	if( (m_one_hour_timer%3600) == 0 )// reload username from file every hour
		ReadInUserNames();
	
	//return ret;
}

//
//
//
void ConnectionSockets::ConnectToHosts(vector<SupernodeHost> &hosts)
{
	UINT i,j;
	// Find idle sockets to tell these hosts to
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<hosts.size();i++)
	{
		for(j=0;j<(UINT)num_socket_events;j++)
		//for(j=0;j<1;j++)
		{
			if(m_sockets[j].IsSocket()==false)
			{
				//m_sockets[j].Create();
				m_sockets[j].Connect(hosts[i]);
				break;
			}
		}
	}
}

//
//
//
void ConnectionSockets::ReportHosts(vector<SupernodeHost> &hosts)
{
	int i,j;

	// Add these hosts to the hosts vector
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check to see if they are already in the hosts vector
		bool found=false;
		for(j=(int)m_status_data.v_host_cache.size()-1;j>=0;j--)
		{
			if(hosts[i]==m_status_data.v_host_cache[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			m_status_data.v_host_cache.push_back(hosts[i]);
		}
	}
}

//
//
//
vector<VendorCount> *ConnectionSockets::ReturnVendorCounts()
{
	UINT i,j;

	// Create a vector on the heap...the memory will be freed in the main thread
	vector<VendorCount> *vendor_counts=new vector<VendorCount>;

	// Get the vendor name from all of the connected sockets. 
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		if(m_sockets[i].IsConnected())
		{
			string vendor=m_sockets[i].ReturnRemoteVendor();

			// Check the vector to see if this vendor is already in there
			bool found=false;
			for(j=0;j<vendor_counts->size();j++)
			{
				if(strcmp(vendor.c_str(),(*vendor_counts)[j].m_vendor.c_str())==0)
				{
					found=true;
					(*vendor_counts)[j].m_count++;
					break;
				}
			}

			// If not found, then create a new vendor count object
			if(!found)
			{
				VendorCount vc;
				vc.m_vendor=vendor;
				vc.m_count=1;
				vendor_counts->push_back(vc);
			}
		}
	}
	
	return vendor_counts;
}

//
//
//
void ConnectionSockets::Log(const char* log)
{
	if(m_dlg_hwnd != NULL)
		::SendMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)log,0);
}


//
//
//
void ConnectionSockets::ReConnectAll()
{
	UINT j;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		m_sockets[j].Close();
	}
}


//
//
//
void ConnectionSockets::GetRandomUserName(CString& username)
{
	if(v_usernames.size() > 0)
	{
		int index = m_rng.GenerateWord32(0,v_usernames.size()-1);
		char* new_username = new char[v_usernames[index].GetLength()+6];
		strcpy(new_username,v_usernames[index]);
		ChangeUserName(new_username);
		username = new_username;
		delete [] new_username;
	}
	else
		username = "KazaaLiteK++";
}

//
//
//
void ConnectionSockets::ChangeUserName(char * username)
{
	CString string = username;
	char* tempuser = new char[strlen(username)+6];
	int len;
	int num;
	int randnum;
	char numchar[1+1];

	//Remove all the numbers
	string.Remove('0');
	string.Remove('1');
	string.Remove('2');
	string.Remove('3');
	string.Remove('4');
	string.Remove('5');
	string.Remove('6');
	string.Remove('7');
	string.Remove('8');
	string.Remove('9');

	len = string.GetLength();
	randnum = rand()%4;

	strcpy(tempuser,string);
	for (int i = 0;i<randnum;i++)
	{
		num = rand()%10;
		_itoa(num,numchar,10);
		strcat(tempuser,numchar);
	}

	num = (len%10);
	_itoa(num,numchar,10);
	strcat(tempuser,numchar);

	strcpy(username,tempuser);
	delete [] tempuser;
}

//
//
//
void ConnectionSockets::ReadInUserNames()
{
	CStdioFile file;
	CString filestring;
	char tempstring[256+1];
	if(file.Open("c:\\syncher\\rcv\\Kazaa-Launcher\\namelist.txt", CFile::modeRead|CFile::typeText) != 0)
	{
		v_usernames.clear();
		LPTSTR data = file.ReadString(tempstring, 256);
		while (data!= NULL)
		{
			filestring = tempstring;
			filestring.TrimRight();
			filestring.TrimLeft();
			v_usernames.push_back(filestring);
			data = file.ReadString(tempstring, 256);
		}
	}
	else
	{
//		MessageBox(NULL, "Coudn't find the file \"namelist.txt\"", "ERROR", MB_OK|MB_ICONERROR);
		return;
	}
	file.Close();
}

