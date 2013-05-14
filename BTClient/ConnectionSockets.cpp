// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
//#include "VendorCount.h"
#include "ConnectionModuleMessageWnd.h"
#include "iphlpapi.h"
//#include "FileSharingManager.h"
#include "afxinet.h"
#include "afxwin.h"
#include <iostream>
#include "zlib.h"
#include "ClientData.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
//	m_one_hour_timer=0;
//	p_file_sharing_manager = NULL;

//	ReadInUserNames();

	char * temp = CreatePeerID();
	strcpy(m_peer_id,temp);
	delete [] temp;
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	m_config.ReadConfig();

	// Init the parent pointers and message window handles


	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
		if(m_config.m_sharing == true)
		{
			m_sockets[i].SetSharing(true);
		}
		m_sockets[i].SetIncrement(m_config.m_sharing_increment);
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
	



//	m_rng.Reseed(true,32);

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

int ConnectionSockets::CheckNumberOfConnections()
{

	int count = 0;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(int i=0;i<num_socket_events;i++)
	{

		if(m_sockets[i].IsConnected())
			{
				count++;
			}
	}

	return count;
}

void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status_data)
{
//	m_one_hour_timer++;
	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
//		m_sockets[i].TimerHasFired();
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
			//if(m_sockets[i].IsConnected())
			//	m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			//m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	status_data=m_status_data;
	
	m_status_data.ClearCounters();
/*
	if( (m_one_hour_timer%3600) == 0 )// reload username from file every hour
		ReadInUserNames();
*/	
	//return ret;
}

//
//
//
void ConnectionSockets::AddNewClient(SOCKET hSocket)
{
	bool found=false;
	for(int i=0;i<NumberOfSocketEvents;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			found=true;
			m_sockets[i].Attach(hSocket);
			m_sockets[i].InitTorrent(m_torrent);
			break;
		}
	}

	if(!found)
	{
		closesocket(hSocket);
	}
}

void ConnectionSockets::KeepConnectionsAlive()
{
	LogMessage("Keep Connections Alive");


	m_mod_data.v_ips.clear();
	for(int i=0;i<NumberOfSocketEvents;i++)
	{


		if((m_sockets[i].IsConnected()==true) && (m_sockets[i].IsSocket()))
		{
			//every 2 min we want to make sure the client knows we're still interested
			if (m_sockets[i].IsSocket() && (m_config.m_send_interested == true))
			{
				m_sockets[i].SendInterested();
			}
			if (m_sockets[i].IsSocket()&&(m_config.m_send_keepalives == true))
			{
				m_sockets[i].SendKeepAlive();
			}
			if (m_sockets[i].IsSocket()&&(m_config.m_send_unchoke == true))
			{
				m_sockets[i].SendUnChoke();
			}
			
			if (m_sockets[i].IsSocket()&& (m_config.m_send_haves == true))
			{
				m_sockets[i].SendHave();
			}
			
			if (m_sockets[i].IsSocket() && (m_config.m_send_requests == true))
			{
				m_sockets[i].SendRequest();
			}
			int buffers = m_sockets[i].RequestSocketInfo();
			string ipstr = m_sockets[i].GetIpString();
			m_mod_data.v_ips.push_back(ipstr);
			char* msg = new char[1024+1];
			sprintf(msg,"Socket number %d, Data Buffers %d",i,buffers);
			if(PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0) == false)
			{
				delete [] msg;
			}
			
		}
		else
		{
			m_sockets[i].SetDisconnected();
		}

	}

	PostMessage(m_dlg_hwnd,WM_REPORT_MOD_DATA,(WPARAM)&m_mod_data,0);
	LogMessage("End Keep Connections Alive");

}

void ConnectionSockets::ReportToDialog(char * msg)
{

	if (PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0)== false)
	{
		delete [] msg;
	}

}

void ConnectionSockets::ReportDataReceived(int data_received, int next_request, int piece, char * ip, char * name, bool is_seed)
{
	LogMessage("Report Data Posting");
	char* msg = new char[512+1];
	sprintf(msg,"Socket received %d bytes of Data from client %s, for piece %d next request starting at block %d",data_received, ip,piece ,next_request);
	if(PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0)== false)
	{
		delete [] msg;
	}
	LogMessage("Report Log Posted");

	// client will be deleted in Report Data
	ClientData * client = new ClientData(name,ip,data_received);
	client->m_is_seed = is_seed;
	client->m_bytes_sent = 0;
	if (PostMessage(m_dlg_hwnd,WM_REPORT_DATA,(WPARAM)client,0)== false)
	{
		delete client;
	}
	LogMessage("Report Data Posted");
/*	if (data_requested > 65536)
	{
		sprintf(msg,"*****ERROR %d bytes requested****** request ignored",data_requested);
		PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0);
	}
	*/
}

void ConnectionSockets::ReportDataSent(int data_sent, int piece, char * ip, char * name)
{
	char* msg = new char[512+1];
	sprintf(msg,"Socket sent %d bytes of Data to client %s, for torrent %s piece %d ",data_sent, ip, name,piece);
	if(PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0)== false)
	{
		delete [] msg;
	}
	ClientData * client = new ClientData();
	client->m_bytes_sent = data_sent;
	client->m_is_seed = false;
	strcpy(client->m_torrentname,name);
	strcpy(client->m_ip,ip);

	// client will be deleted in Report Data
	if (PostMessage(m_dlg_hwnd,WM_REPORT_DATA,(WPARAM)client,0)== false)
	{
		delete client;
	}
	LogMessage("Report Data Posted");

}

void ConnectionSockets::ReportClientId(char * id, char * ip)
{
	LogMessage("Report Client ID posting");
	char* msg = new char[512+1];
	sprintf(msg,"Connected to Client %s with id %s",ip,id);
	if(PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0)== false)
	{
		delete [] msg;
	}
	LogMessage("Report Client ID posted");
}

void ConnectionSockets::ReportOnConnect(char * ip)
{
	LogMessage("Report On Connect posting");
	char* msg = new char[256+1];
	sprintf(msg,"On Connect Called with %s ",ip);
	if(PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)msg,0)== false)
	{
		delete [] msg;
	}
	LogMessage("Report On Connect posted");

}

void ConnectionSockets::LogMessage(char * msg)
{
	/*
	CFile logfile;
	if (logfile.Open("BTSocketLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
	{
		logfile.SeekToEnd();
		logfile.Write(msg, (unsigned int)strlen(msg));
		logfile.Write("\r\n",2);
		logfile.Close();
	}
	*/

}
void ConnectionSockets::MakeNewConnection(char * ip, int port, bool is_seed)
{
//	LogMessage("Make New Connection Started");
	for(int i=0;i<NumberOfSocketEvents;i++)
	{
		//LogMessage("Is Socket Check Started");
		if(m_sockets[i].IsSocket()==false)
		{
			m_sockets[i].Create();
		}
//		LogMessage("Is Socket Check Ended");
//		LogMessage("Is Connected Started");
		if(m_sockets[i].IsConnected()==false)
		{
			LogMessage("Init Torrent");
			bool torrentok = m_sockets[i].InitTorrent(m_torrent);
			if (torrentok == false)
			{
				break;
			}
			LogMessage("Connect");
			m_sockets[i].Connect(ip,port);
			LogMessage("SetIp");
			m_sockets[i].SetIp(ip);
			LogMessage("Set Temp");
			m_sockets[i].SetTempConnect();
			if(is_seed == true)
			{
				m_sockets[i].SetAsSeed();
			}
			break;
		}
	//	LogMessage("Is Connected Ended");
	}
//	LogMessage("Make New Connection Ended");
}

char * ConnectionSockets::GetTrackerMessage(TorrentFile * tf,const char *url,char * peer_id, int port)
{
	char * trackerURL = new char[512];
	char buffer[20];
	char portint[32];
	_itoa(port,portint,10);
	strcpy(trackerURL, url);
	strcat(trackerURL, "?info_hash=");
	strcat(trackerURL, tf->GetInfoHashString().c_str());
	strcat(trackerURL, "&peer_id=");
	strcat(trackerURL, peer_id);
	strcat(trackerURL, "&port=");
	strcat(trackerURL, portint);
	strcat(trackerURL, "&uploaded=0&downloaded=0&left=0");
	itoa(tf->GetLength(), buffer, 10);
	//strcat(trackerURL, "0");
	strcat(trackerURL, buffer);
	strcat(trackerURL, "&numwant=1000");
	//strcat(trackerURL, "&compact=1");
	//Log("TODO: implement compact decoding ");
	return trackerURL;
}



char * ConnectionSockets::CreatePeerID()
{
	char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	char * id = new char [21];
	strcpy(id, "-AZ2202-");
	srand( (unsigned)time( NULL ) );
	for(int i = 8; i < 20; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[20] = 0;
	return id;
}


void ConnectionSockets::RegisterWithTracker(TorrentFile * tf, int port)
{

	vector <string>::iterator v1_Iter;
	vector<string> v_list = tf->GetAnnounceList();
	unsigned short * peers = NULL;
	for(v1_Iter = v_list.begin(); v1_Iter != v_list.end( ) ;v1_Iter++) 
	{
		char * trackerURL = GetTrackerMessage(tf,(*v1_Iter).c_str(),m_peer_id, port);
		int size = SendTrackerInfo(trackerURL, &peers, "");
		// for now we're ignoring the set of peers we get from this registration and using the main list
		// We want to hit the guys we hit hard so we're using the same list for everyone
		peers = NULL;
	}

}

void ConnectionSockets::InitTorrent(TorrentFile torrent)
{
	m_torrent = torrent;
	strcpy(m_mod_data.m_torrent_name,torrent.GetName().c_str());

}

unsigned char * ConnectionSockets::GetHandshakeMessage()
{
	LogMessage("GetHandshake Started");
	unsigned char * message = new unsigned char[68];
	unsigned short * info_hash = m_torrent.GetInfoHash();
	string id = m_peer_id;
	string protocol = "BitTorrent protocol";
	int len = (int)protocol.length();
	int cur_pos = 0;
	message[0] = len;
	cur_pos++;
	memcpy(&message[1], protocol.c_str(), len); // protocol
	cur_pos += len;
	for(int i = 0; i < 8 ; i++ ) { // reserve bits
		message[i+cur_pos] = 0;
	}
	cur_pos += 8;
	for(int i = 0 ; i < 20; i++) {
		message[i+cur_pos] = (TCHAR)info_hash[i];

	}
	cur_pos += 20;
	memcpy(&message[cur_pos], id.c_str(), id.length());
	cur_pos += (int)id.length();

	LogMessage("GetHandshake Returning");
	return message;

}

//
//
//
//void ConnectionSockets::ConnectToHosts(vector<SupernodeHost> &hosts)
//{
//	UINT i,j;
//	// Find idle sockets to tell these hosts to
//	int num_socket_events=ReturnNumberOfSocketEvents();
//	for(i=0;i<hosts.size();i++)
//	{
//		for(j=0;j<(UINT)num_socket_events;j++)
//		//for(j=0;j<1;j++)
//		{
//			if(m_sockets[j].IsSocket()==false)
//			{
//				//m_sockets[j].Create();
//				m_sockets[j].Connect(hosts[i]);
//				break;
//			}
//		}
//	}
//}

//
//
//
//void ConnectionSockets::ReportHosts(vector<SupernodeHost> &hosts)
//{
//	int i,j;
//
//	// Add these hosts to the hosts vector
//	for(i=0;i<(int)hosts.size();i++)
//	{
//		// Check to see if they are already in the hosts vector
//		bool found=false;
//		for(j=(int)m_status_data.v_host_cache.size()-1;j>=0;j--)
//		{
//			if(hosts[i]==m_status_data.v_host_cache[j])
//			{
//				found=true;
//				break;
//			}
//		}
//
//		if(!found)
//		{
//			m_status_data.v_host_cache.push_back(hosts[i]);
//		}
//	}
//}
/*
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
//			string vendor=m_sockets[i].ReturnRemoteVendor();

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
host 38.119.66.27 
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
*/

int ConnectionSockets::SendTrackerInfo(char * url,unsigned short **body, string referer)
{

	// From BT scraper, we'll connect in the same way to register our individual ID
	CInternetSession session("Azereus 2.2.0.0");
	CHttpConnection * pServer = NULL;
	CHttpFile* pFile = NULL;
	bool bProgressMode = false; 
	bool bStripMode = false; 
	bool isGzipped = false;
	string line = "";
	int size = 0;
	unsigned short * buf = *body;
	try {
		CString strServerName; 
		CString strObject; 
		INTERNET_PORT nPort; 
		DWORD dwServiceType; 

		if (!AfxParseURL(url, dwServiceType, strServerName, strObject, nPort) || 
		dwServiceType != INTERNET_SERVICE_HTTP) 
		{ 
			//Log( _T("Error: can only use URLs beginning with http://"));
			
			line =  "request failed - can only use URLs beginning with http://";
		} 

		if (bProgressMode) 
		{ 
			//Log( _T("Opening Internet...")); 
			VERIFY(session.EnableStatusCallback(TRUE)); 
		} 
		
		pServer = session.GetHttpConnection(strServerName, INTERNET_FLAG_DONT_CACHE, nPort); 
		
		string file = url;
		int file_start = -1;
		if(file.length() > 8) {
			file_start = (int)file.find("//");
			file_start = (int)file.find("/", file_start+2);
		}
		if(file_start > 0) file = file.substr(file_start);
		else file = "";
		//cout << "file: " << file << endl;
		
		
 		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,file.c_str()); 

		pFile->SendRequest(); 

		DWORD dwRet; 
		pFile->QueryInfoStatusCode(dwRet); 

		// if access was denied, prompt the user for the password 

		if (dwRet == HTTP_STATUS_DENIED) 
		{ 
			DWORD dwPrompt; 
			dwPrompt = pFile->ErrorDlg(NULL, ERROR_INTERNET_INCORRECT_PASSWORD, 
			FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL); 

			// if the user cancelled the dialog, bail out 

			if (dwPrompt != ERROR_INTERNET_FORCE_RETRY) 
			{ 
				//Log(_T("Access denied: Invalid password\n")); 
				line =   "request failed - Access denied: Invalid password";
			} 

			pFile->SendRequest(); 
			pFile->QueryInfoStatusCode(dwRet); 
		} 

		CString strNewLocation; 
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation); 

		// were we redirected? 
		// these response status codes come from WININET.H 

		if (dwRet == HTTP_STATUS_MOVED || 
		dwRet == HTTP_STATUS_REDIRECT || 
		dwRet == HTTP_STATUS_REDIRECT_METHOD) 
		{ 
			CString strNewLocation; 
			pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation); 

			int nPlace = strNewLocation.Find(_T("Location: ")); 
			if (nPlace == -1) 
			{ 
				//Log( _T("Error: Site redirects with no new location")); 
				line =  "request failed - Site redirects with no new location";
			} 

			strNewLocation = strNewLocation.Mid(nPlace + 10); 
			nPlace = strNewLocation.Find('\n'); 
			if (nPlace > 0) 
				strNewLocation = strNewLocation.Left(nPlace); 

			// close up the redirected site 

			pFile->Close(); 
			delete pFile; 
			pServer->Close(); 
			delete pServer; 

			if (bProgressMode) 
			{ 
				//Log( _T("Caution: redirected to ")); 
				//Log((LPCTSTR) strNewLocation); 
			} 

			// figure out what the old place was 
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort)) 
			{ 
				//Log(_T("Error: the redirected URL could not be parsed.")); 
				line =  "request failed - the redirected URL could not be parsed.";
			} 

			if (dwServiceType != INTERNET_SERVICE_HTTP) 
			{ 
				//Log(_T("Error: the redirected URL does not reference a HTTP resource."));
				line =  "request failed - the redirected URL does not reference a HTTP resource";
			} 

			// try again at the new location 
			pServer = session.GetHttpConnection(strServerName, nPort); 
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
			//pFile->AddRequestHeaders(szHeaders); 
			pFile->SendRequest(); 

			pFile->QueryInfoStatusCode(dwRet); 
			if (dwRet != HTTP_STATUS_OK) 
			{ 
				cout << _T("Error: Got status code ") << dwRet << endl; 
				line =  "request failed - Got status code " + dwRet;
			} 
		}

		//cout << _T("Status Code is ") << dwRet << endl; 
		CString content_type = "";
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF , content_type);
		if(content_type.Find(": gzip") > 0) isGzipped = true;

		const int read_size = 1024;
		TCHAR sz[read_size+1]; 
		UINT nRead = pFile->Read(sz, read_size); 
		//cout << nRead << " ";
		int curPos = 0;
 		size = nRead;
		int cur_size = 10240;
		buf = new unsigned short[cur_size];
		while (nRead > 0) 
		{ 
			if(size > cur_size) {
				int new_size = cur_size * 2;
				unsigned short *temp = new unsigned short[new_size];
				for(int i = 0; i < cur_size; i++) {
					temp[i] = buf[i];
				}
				delete [] buf;
				buf = temp;
				cur_size = new_size;
			}
			for(UINT i = 0; i < nRead; i++) {
				//if(!isGzipped) cout << sz[i];
				buf[i+curPos] = sz[i];
			}
			curPos += nRead;
			nRead = pFile->Read(sz, read_size); 
			//cout << nRead << " ";
			size += nRead;
		}
		//cout << endl;
		unsigned short * temp = new unsigned short[size+1];
		for(int i = 0; i < size; i++) {
			temp[i] = buf[i];
		}
		//if(!isGzipped) cout << endl;
		delete [] buf;
		buf = temp;
	}
	catch (exception e) {
		//Log(e.what());
		line = "request failed - status code N/A";
	}
	catch (CInternetException * e) {
		TCHAR error[256];
		if(e->GetErrorMessage(error, 256)) {
//			Log(error);
		}
		e->Delete();
		e = NULL;
		line = "request failed - status code N/A";
	}
	if(pFile != NULL) {
		pFile->Flush();
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	if(pServer != NULL) {
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	if(isGzipped) {
		unsigned short *data = NULL;
		size = UnZip(&data, buf, size);
		if(size == 0) {
			delete [] data;
			data = NULL;
		}
		delete [] buf;
		*body = data;
	}			
	else *body = buf;
	//cout << endl << "SIZE: " << size << endl;
	return size;

}

int ConnectionSockets::UnZip(unsigned short ** dest, unsigned short * source, int len) 
{
		bool write = false;
		//if(write) TorrentFile::WriteFile((unsigned short*)source, len, cur_title, "zip");
		Bytef *uncompr = NULL;
		z_stream strm;
		uLong comprLen = 10000*sizeof(int); /* don't overflow on MSDOS */
		uLong uncomprLen = comprLen;

		int zip_status = 0;
		//dest = new unsigned short[size*3];
		uncompr = new Bytef[uncomprLen];
		strcpy((char*)uncompr, "garbage");

		// The fields next_in, avail_in, zalloc, zfree and opaque must be initialized
		Bytef * byte_src = new Bytef[len];
		for(int i = 0; i < len; i++) {
			byte_src[i] = (Bytef)source[i];
		}
		//strm.next_in = (Bytef*)source;
		strm.next_in = byte_src;
		strm.avail_in = 0;
		strm.zalloc = (alloc_func)0;
		strm.zfree = (free_func)0;
		strm.opaque = (voidpf)0;
		strm.next_out = (Bytef*)uncompr;

		zip_status = inflateInit2 (&strm, 15+16); 
		if(zip_status != Z_OK) { 
			delete [] uncompr;
			delete [] byte_src;
			return 0;
		}
		int size = 0;
		while (strm.total_out < uncomprLen && strm.total_in < comprLen) {
			size++;
			strm.avail_in = strm.avail_out = 1; /* force small buffers */
			zip_status = inflate(&strm, Z_NO_FLUSH);
			if (zip_status == Z_STREAM_END) break;
			if (zip_status == Z_DATA_ERROR) {
				zip_status = inflateSync(&strm);
			}
			if (zip_status == Z_BUF_ERROR) {
				cout << strm.msg;
			}
			if(zip_status != Z_OK) { 
				//TorrentFile::WriteFile(source, len, cur_title, "zip");
				//TorrentFile::WriteFile((unsigned short*)uncompr, size, cur_title, "txt");
				delete [] uncompr;
				delete [] byte_src;
				return 0;
			}
			
		}
		zip_status = inflateEnd(&strm); 
		if(zip_status != Z_OK) { 
			delete [] uncompr;
			delete [] byte_src;
			return 0;
		}
		//if(write) TorrentFile::WriteFile((unsigned short*)uncompr, size, cur_title, "txt");
		unsigned short * data = *dest;
		data = new unsigned short[size];
		for(int i = 0; i < size; i++) {
			data[i] = uncompr[i];
		}
		delete [] uncompr;
		delete [] byte_src;
		*dest = data;
		return size;
}


