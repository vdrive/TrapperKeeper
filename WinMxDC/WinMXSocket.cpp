#include "StdAfx.h"
#include "winmxsocket.h"
#include "ConnectionSockets.h"
#include "FileSharingManager.h"

#define WINMX_NUM_SHARE_SEND_PER_TIMER 20
#ifdef _DEBUG
#define WINMX_MAX_FILES_SHARE 100
#else
#define WINMX_MAX_FILES_SHARE 3000
#endif

WinMXSocket::WinMXSocket(void)
{
	srand((unsigned)time(NULL));
	//memset(&m_NodeInfo, 0, sizeof(m_NodeInfo));
	m_connection_state = Idle;
	//m_csBaseUserName = "anonymous000";
	m_wLineType=7;
	m_wTransferPort=6699;
	//p_NodeInfo=NULL;
	m_dwMinTimer=0;
	// Initialize Critical Section
	::InitializeCriticalSection(&m_ClientLock);
	p_temp_buf=NULL;
	p_temp_buf_size=0;
	p_temp_header_buf=NULL;
	p_temp_header_buf_size=0;
}

//
//
//
WinMXSocket::~WinMXSocket(void)
{
	// Delete Critical Section
	::DeleteCriticalSection(&m_ClientLock);
	if(p_temp_buf!=NULL)
		delete [] p_temp_buf;
	if(p_temp_header_buf!=NULL)
		delete [] p_temp_buf;
}

//
//
//

void WinMXSocket::InitParent(ConnectionSockets* parent)
{
	p_sockets = parent;
}


//
//
//
void WinMXSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		/*
		CString log;
		log.Format("Connecting error: %d",error_code);
		//p_parent->Log(log);
		ConnectToPeerCache("216.127.74.62",7952);
		*/
	}
}

//
//
//
void WinMXSocket::OnClose(int error_code)
{
	Close();
	ResetData();
	/*
	CString log;
	log.Format("Connection closed. Error code: %d",error_code);
	//p_parent->Log(log);
	ConnectToPeerCache("216.127.74.62",7952);
	*/
}

//
//
//
void WinMXSocket::OnReceive(int error_code)
{
	::EnterCriticalSection(&m_ClientLock);
	if(m_bRecevingPeerCache)
	{
		ReceiveSocketData(149);
	}
	else
	{
		switch(m_connection_state)
		{
			case Connecting_Init:
			{
				ReceiveSocketData(1);
				break;
			}
			case Connecting_Init_Received:
			{
				break;
			}
			case Connecting_Up_Key_Sent:
			{
				ReceiveSocketData(16);
				break;
			}
			case Connecting_Down_Key_Received:
			{
				break;
			}
			case Connecting_Login_Request_Sent:
			{
				ReceiveSocketData(6);
				break;
			}
			case Connecting_Login_Reply_Received:
			{
				break;
			}
			case Connected:
			{
				if(!m_msg_header_received)
					ReceiveSomeSocketData(4-p_temp_header_buf_size);
				else
					ReceiveSomeSocketData(m_msg_len-p_temp_buf_size);
				break;
			}
		}
	}
	::LeaveCriticalSection(&m_ClientLock);
}

//
//
//
void WinMXSocket::SocketDataReceived(char *data,unsigned int len)
{
	BYTE* ptr = (BYTE*)data;
	 
	//Retrieving server nodes
	if(m_bRecevingPeerCache)
	{
		if(len!=149)
		{
			Close();
			return;
		}
		char Buff[512] = {'\0'};
		BYTE *pNodeBuff = (BYTE *)Buff + sizeof(Buff) / 2;
		PARENTNODEINFO FreeNode;
		PARENTNODEINFO NodeInfo[10];
		int i = 0, j = 0, iFree = 0;
		WORD wHostIndex = 0;
		WORD wPortIndex = 0;

		if(*ptr == 0x38)
		{
			ptr++;
			if(GetCryptKeyID(ptr) != 0x54)
			{
				Close();
				return;
			}
			else
				ptr+=16;

			// Decrypt Node Information
			DecryptFrontCode(ptr, pNodeBuff);
			memcpy(&NodeInfo, pNodeBuff, 120);

	
			// Sort by Free Secondary Num
			for(j = 0 ; j <= 9 ; j++)
			{
				iFree = j;
				for(i = j + 1 ; i < 10 ; i++)
				{
					if(NodeInfo[i].bFreeSec > NodeInfo[iFree].bFreeSec || (NodeInfo[i].bFreeSec == NodeInfo[iFree].bFreeSec && NodeInfo[i].bFreePri < NodeInfo[iFree].bFreePri))
						iFree = i;
				}
				FreeNode = NodeInfo[j];
				NodeInfo[j] = NodeInfo[iFree];
				NodeInfo[iFree] = FreeNode;
			}
		}
		Close();
		//p_parent->ServerNodesReceived(m_NodeInfo);
		/*
		for(int i=0;i<=9;i++)
		{
			PARENTNODEINFO new_node;
			memcpy(&new_node, &NodeInfo[i], sizeof(PARENTNODEINFO));
			v_parent_nodes.push_back(new_node);
		}
		vector<PARENTNODEINFO>::iterator iter = v_parent_nodes.begin();
		while(iter != v_parent_nodes.end())
		{
			if(iter->dwNodeIP != 0 && iter->wTCPPort != 0)
			{
				//p_NodeInfo = &(m_NodeInfo[i]);
				ConnectToSupernode(iter->dwNodeIP, iter->wTCPPort);
				v_parent_nodes.erase(iter);
				break;
			}
			else
				v_parent_nodes.erase(iter);
		}
		*/
	}
	else
	{
		switch(m_connection_state)
		{
			case Connecting_Init:
			{
				if(*data == 0x31)
				{
					m_connection_state = Connecting_Init_Received;
					SendUpKey();
					//ReceiveSocketData(16);
				}
				else
					Close();
				break;
			}
			case Connecting_Init_Received:
			{
				break;
			}
			case Connecting_Up_Key_Sent:
			{
				if(GetCryptKeyID((BYTE *)data) != 0x53)
				{
					Close();
				}
				else
				{
					GetCryptKey((BYTE *)data, &m_dwUPKey, &m_dwDWKey);
				}
				m_connection_state=Connecting_Down_Key_Received;
				SendLoginInfo();
				break;
			}
			case Connecting_Down_Key_Received:
			{
				break;
			}
			case Connecting_Login_Request_Sent:
			{
				if(len != 6)
				{
					Close();
					break;
				}
				//::EnterCriticalSection(&m_ClientLock);
				m_dwDWKey = DecryptMXTCP((BYTE *)data, 6, m_dwDWKey);
				//::LeaveCriticalSection(&m_ClientLock);

				WORD msg_id = *(WORD*)data;
				WORD msg_len = *(WORD*)(data+2);
				if(msg_id == WTID_LOGINACCEPT && msg_len==2)
				{
					//Connection accepted
					// Make User Name
					m_csWPNUserName.Format("%s_%05u", m_csBaseUserName, *(WORD *)(data + 4));

					//m_wParentTCPPort = p_NodeInfo->wTCPPort;
					//m_wParentUDPPort = p_NodeInfo->wUDPPort;
					//m_dwParentIP = p_NodeInfo->dwNodeIP;
					
					ConnectedToSupernode();
				}
				else
				{
					Close();
				}
				break;
			}
			case Connecting_Login_Reply_Received:
			{
				break;
			}
			case Connected:
			{
				/*
				if(!m_msg_header_received)
				{
					if(len == 4)
					{
						m_msg_header_received = true;
						BYTE header[4];
						memset(header,'\0',4);
						memcpy(&header[0],data,4);
						//::EnterCriticalSection(&m_ClientLock);
						m_dwDWKey = DecryptMXTCP(header, 4, m_dwDWKey);
						//::LeaveCriticalSection(&m_ClientLock);
						WORD wMsgType = *(WORD *)header;
						WORD wMsgLen = *(WORD *)&header[2];
						m_msg_type = wMsgType;
						m_msg_len = wMsgLen;
						if(!wMsgLen)
							break;
						else
							ReceiveSomeSocketData(m_msg_len);
					}
					else
					{
						p_parent->Log("Header Received Error: Received data length didn't match with header message length. Closing socket");
						Close();
					}

				}
				else
				{
					if(len == m_msg_len)
					{
						m_msg_header_received = false;
						BYTE* buf = new BYTE[len];
						memset(buf,'\0',len);
						memcpy(&buf[0],data,len);
						// Decrypt Data
						//::EnterCriticalSection(&m_ClientLock);
						m_dwDWKey = DecryptMXTCP(buf, len, m_dwDWKey);
						//::LeaveCriticalSection(&m_ClientLock);
						// Analysis Message
						WPNPMessageReceived(m_msg_type, m_msg_len, buf);
						delete [] buf;
					}
					else
					{
						p_parent->Log("Data Received Error: Received data length didn't match with header message length. Closing socket");
						Close();
					}
					break;
				}
				*/
				break;
			}
		}
	}
}

//
//
//
void WinMXSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	if(data_len==0)
		return;
	if(!m_msg_header_received)
	{
		if(p_temp_header_buf==NULL)
		{
			p_temp_header_buf = new char[data_len];
			memcpy(p_temp_header_buf,data,data_len);
			p_temp_header_buf_size = data_len;
		}
		else
		{
			char * temp_buf = new char[p_temp_header_buf_size+data_len];
			memcpy(temp_buf,p_temp_header_buf,p_temp_header_buf_size);
			memcpy(&temp_buf[p_temp_header_buf_size],data,data_len);
			delete [] p_temp_header_buf;
			p_temp_header_buf_size = p_temp_header_buf_size+data_len;
			p_temp_header_buf = new char[p_temp_header_buf_size];
			memcpy(p_temp_header_buf, temp_buf, p_temp_header_buf_size);
			delete [] temp_buf;
		}
		if(p_temp_header_buf_size==4)
		{
			m_msg_header_received=true;
			//::EnterCriticalSection(&m_ClientLock);
			m_dwDWKey = DecryptMXTCP((BYTE*)p_temp_header_buf, 4, m_dwDWKey);
			//::LeaveCriticalSection(&m_ClientLock);
			WORD wMsgType = *(WORD *)p_temp_header_buf;
			WORD wMsgLen = *(WORD *)(p_temp_header_buf+2);
			m_msg_type = wMsgType;
			m_msg_len = wMsgLen;
			delete [] p_temp_header_buf;
			p_temp_header_buf=NULL;
			p_temp_header_buf_size=0;
			if(wMsgLen)
				ReceiveSomeSocketData(m_msg_len);
		}
		else if(p_temp_header_buf_size < 4)
		{
			ReceiveSomeSocketData(4-p_temp_header_buf_size);
		}
		else if(p_temp_header_buf_size > 4)
		{
			//p_parent->Log("Header Received Error: Received data length didn't match with header message length. Closing socket");
			Close();
		}
	}
	else
	{
		if(p_temp_buf==NULL)
		{
			p_temp_buf = new char[data_len];
			memcpy(p_temp_buf,data,data_len);
			p_temp_buf_size = data_len;
		}
		else
		{
			char * temp_buf = new char[p_temp_buf_size+data_len];
			memcpy(temp_buf,p_temp_buf,p_temp_buf_size);
			memcpy(&temp_buf[p_temp_buf_size],data,data_len);
			delete [] p_temp_buf;
			p_temp_buf_size = p_temp_buf_size+data_len;
			p_temp_buf = new char[p_temp_buf_size];
			memcpy(p_temp_buf, temp_buf, p_temp_buf_size);
			delete [] temp_buf;
		}


		if(p_temp_buf_size == m_msg_len)
		{
			m_msg_header_received = false;
			// Decrypt Data
			//::EnterCriticalSection(&m_ClientLock);
			m_dwDWKey = DecryptMXTCP((BYTE*)p_temp_buf, m_msg_len, m_dwDWKey);
			//::LeaveCriticalSection(&m_ClientLock);
			// Analysis Message
			WPNPMessageReceived(m_msg_type, m_msg_len, (const BYTE *)p_temp_buf);
			delete [] p_temp_buf;
			p_temp_buf=NULL;
			p_temp_buf_size=0;

		}
		else if(p_temp_buf_size < m_msg_len)
		{
			ReceiveSomeSocketData(m_msg_len-p_temp_buf_size);
		}
		else if(p_temp_buf_size > m_msg_len)
		{
//			p_parent->Log("WARNING: Received more data than expected. Closing socket");
			Close();
		}
	}
}

//
//
//
int WinMXSocket::Close()
{
	ResetData();
	//p_sockets->Log("Socket closed");
	//TRACE("FastTrack Socket Closed\n");
	return TEventSocket::Close();
}

//
//
//
void WinMXSocket::ResetData()
{
	m_status_data.Clear();
	m_bRecevingPeerCache=false;
	m_connection_state = Idle;
	m_dwUPKey=0;
	m_dwDWKey=0;
	m_dwMinTimer=0;
	m_msg_header_received=false;
	m_msg_len=0;
	m_msg_type=0;
	if(p_temp_buf!=NULL)
	{
		delete [] p_temp_buf;
		p_temp_buf=NULL;
	}
	p_temp_buf_size=0;
	if(p_temp_header_buf!=NULL)
	{
		delete [] p_temp_header_buf;
		p_temp_header_buf=NULL;
	}
	p_temp_header_buf_size=0;
	m_num_shares_sent=0;
	m_wLineType = rand()%4 + 7;
}

//
//
//
/*
int WinMXSocket::ConnectToPeerCache(const char* IP, unsigned short port)
{
	Close();
	Create();
	if(v_parent_nodes.size()==0)
	{
		m_bRecevingPeerCache=true;
		return TEventSocket::Connect(IP,port);
	}
	else
	{
		vector<PARENTNODEINFO>::iterator iter = v_parent_nodes.begin();
		while(iter != v_parent_nodes.end())
		{
			if(iter->dwNodeIP != 0 && iter->wTCPPort != 0)
			{
				//p_NodeInfo = &(m_NodeInfo[i]);
				ConnectToSupernode(iter->dwNodeIP, iter->wTCPPort);
				v_parent_nodes.erase(iter);
				break;
			}
			else
				v_parent_nodes.erase(iter);
		}
	}
	return TRUE;
}
*/
//
//
//
int WinMXSocket::ConnectToSupernode(SupernodeHost host)
{
	Close();
	Create();
	m_status_data.m_host=host;
	m_status_data.m_connect_time=CTime::GetCurrentTime();
	m_bRecevingPeerCache=false;
	m_connection_state = Connecting_Init;
	if(p_sockets!=NULL)
		p_sockets->GetRandomUserName(m_csBaseUserName);
	
	/*
	CString log;
	CString address = GetIP(host.m_ip);
	address.AppendFormat(":%d",host.m_port);
	log.Format("Connecting to %s", address);
	p_parent->Log(log);
	*/
	return TEventSocket::Connect(host.m_ip,host.m_port);
}

//
//
//
void WinMXSocket::SendUpKey()
{
	BYTE buff[16];
	// Send UP Key Block
	CreateCryptKeyID(0x52, buff);
	SendSocketData((unsigned char*)buff,16);
	m_connection_state=Connecting_Up_Key_Sent;
}

//
//
//
void WinMXSocket::SendLoginInfo()
{
	int buf_len = 9 + m_csBaseUserName.GetLength();
	char* buf = new char[buf_len];
	//char buf[128];
	//ZeroMemory(buf,128);
	
	// Send Login Request
	*(WORD *)buf = WTID_LOGINREQUEST;
	*(WORD *)(buf+2) = (WORD)buf_len-4;
	lstrcpy(buf + 4, m_csBaseUserName);
	memcpy(buf + m_csBaseUserName.GetLength() + 5, &m_wLineType, 2);	
	memcpy(buf + m_csBaseUserName.GetLength() + 7, &m_wTransferPort, 2);	
	
	m_dwUPKey = EncryptMXTCP((BYTE *)buf, buf_len, m_dwUPKey);

	SendSocketData((unsigned char*)buf,buf_len);
	m_connection_state = Connecting_Login_Request_Sent;
	delete [] buf;
}

//
//
//
BOOL WinMXSocket::SendQueStatus(WORD wULMaxNum, WORD wULFreeNum, WORD wULQueNum)
{
	// Send Que Status
	if(m_connection_state!=Connected)
		return FALSE;

	char szQueStatus[MAXTEXTBUFFSIZE] = "";
	
	if(wULFreeNum)
		sprintf(szQueStatus, "%u %u of %u available", QUESTATUSBASE + wULFreeNum - wULQueNum, wULFreeNum, wULMaxNum);
	else 
		sprintf(szQueStatus, "%u %u in queue (%u of %u available)", QUESTATUSBASE + wULFreeNum - wULQueNum, wULQueNum, wULFreeNum, wULMaxNum);

	CString log;
	log.Format("Message \"%s\" sent",szQueStatus);
//	p_parent->Log(log);
	return SendFormatMessage(WTID_QUESTATUS, "S", szQueStatus);
}

//
//
//
BOOL WinMXSocket::SendMessage(WORD wType, WORD wLen, unsigned char *pData)
{
	if(m_connection_state!=Connected)
		return FALSE;
	//char Buff[MAXSENDMESSAGESIZE] = {'\0'};
	unsigned char* Buff = new unsigned char[wLen + sizeof(WORD)*2];
	memcpy(Buff + sizeof(WORD)*2, pData, wLen);

	// Encrypt
	*(WORD *)(Buff) = wType;
	*(WORD *)(Buff + sizeof(WORD)) = wLen;
	wLen += (sizeof(WORD) * 2);
	m_dwUPKey = EncryptMXTCP((BYTE *)Buff, wLen, m_dwUPKey);
	SendSocketData((unsigned char*)Buff,wLen);
	delete [] Buff;
	return TRUE;
}

//
//
//
BOOL WinMXSocket::SendFormatMessage(WORD wType, char *lpszFormat, ...)
{
	if(m_connection_state!=Connected)
		return FALSE;

	// Data Format
	char data[MAXSENDMESSAGESIZE];
	::ZeroMemory(&data,MAXSENDMESSAGESIZE);

	va_list Args;
	va_start(Args, lpszFormat);
	WORD wLen = 0;
	char *lpText = NULL;
	char *pBuff = &data[0];

	while(*lpszFormat){
		switch(*lpszFormat){
			case 'S' :	// NULL Terminated String
				lpText = va_arg(Args, char *);
				lstrcpy(pBuff + wLen, lpText);
				wLen += (lstrlen(lpText) + 1);
				break;

			case 's' :	// Not NULL Terminated String
				lpText = va_arg(Args, char *);
				lstrcpy(pBuff + wLen, lpText);
				wLen += (lstrlen(lpText));
				break;

			case 'B' :	// BYTE
				pBuff[wLen] = va_arg(Args, BYTE);
				wLen += 1;
				break;
			 
			case 'W' :	// WORD
				*((WORD *)(pBuff + wLen)) = va_arg(Args, WORD);
				wLen += 2;
				break;

			case 'D' :	// DWORD
				*((DWORD *)(pBuff + wLen)) = va_arg(Args, DWORD);
				wLen += 4;
				break;
			 
			default :
				ASSERT(0);
				break;
			}
	
		lpszFormat++;
		}

	pBuff[wLen] = 0;
	va_end(Args);

	BYTE Buff[MAXSENDMESSAGESIZE];
	ZeroMemory(Buff,MAXSENDMESSAGESIZE);
	memcpy(Buff + sizeof(WORD)*2, data, wLen);

	// Encrypt
	*(WORD *)(Buff) = wType;
	*(WORD *)(Buff + sizeof(WORD)) = wLen;
	wLen += (sizeof(WORD) * 2);
	m_dwUPKey = EncryptMXTCP(Buff, wLen, m_dwUPKey);
	SendSocketData((unsigned char*)Buff,wLen);
	return TRUE;
}

//
//
//
void WinMXSocket::WPNPMessageReceived(WORD msg_type, WORD len, const BYTE* data)
{
	//CString log;
	//log.Format("Message Received. Type: %2X, Len:%d, Data: %s", msg_type, len, data);
//	p_parent->Log(log);

	switch(msg_type)
	{
		case WTID_FILE_REQUEST:
		{
#ifdef WINMX_SWARMER
			char* ptr = (char*)data;
			CString network = ptr;
			ptr += network.GetLength()+1;
			CString user = ptr;
			ptr += user.GetLength()+1;
			CString file = ptr;
			ptr += file.GetLength()+1;
			int option = *(int*)ptr;

			PacketBuffer buff;
			buff.PutStr(network,network.GetLength());
			buff.PutByte('\0');
			buff.PutStr(user,user.GetLength());
			buff.PutByte('\0');
			buff.PutStr(file,file.GetLength());
			buff.PutByte('\0');
			SendMessage(WTID_FILE_REQUEST_REPLY,buff.Size(),buff.p_data);
#endif
			break;
		}
		case WTID_SERVER_PING:
		{
			p_sockets->m_status_data.m_ping_count++;
			ExtractHosts(data,len);
			break;
		}
		case WTID_SEARCH_HIT:
		{
			p_sockets->m_status_data.m_query_hit_count++;
			const BYTE* ptr = data;
			int search_id = *(int*)ptr;

			// Find the project supply query that is related to this search ID
			//ProjectSupplyQuery *psq=NULL;
			ProjectSupplyQuery find_psq;
			find_psq.m_search_id=search_id;
			hash_set<ProjectSupplyQuery>::iterator psq=NULL;
			::EnterCriticalSection(&(p_sockets->m_psq_lock));
			psq=p_sockets->hs_project_supply_queries.find(find_psq);
			// If it is still equal to NULL, then return
			if(psq==p_sockets->hs_project_supply_queries.end() || psq==NULL)
			{
				//TRACE("Couldn't find search ID for this query result\n");
				::LeaveCriticalSection(&(p_sockets->m_psq_lock));
				return;
			}

			ProjectSupplyQuery this_psq = *psq;
			::LeaveCriticalSection(&(p_sockets->m_psq_lock));


			// Find the host connection status data for this project
			ProjectStatus *status=&p_sockets->m_status_data.v_project_status[this_psq.m_project_status_index];
			vector<QueryHit> *query_hits=&p_sockets->m_status_data.v_project_status[this_psq.m_project_status_index].v_query_hits;



			ptr+=4;
			int ip = *(int*)ptr;
//#ifdef _DEBUG
//			in_addr addr;
//			addr.S_un.S_addr = ip;
//			CString ip_str = inet_ntoa(addr);
//#endif
			ptr+=4;
			WORD port = *(WORD*)ptr;
			ptr+=2;
			ptr+=5; //(always 4e 5b 04 00 a9)
			CString file_path = (char*)ptr;
			ptr += file_path.GetLength()+1;
			BYTE hash[16];
			memcpy(&hash, ptr, 16);
			ptr+=16;
			UINT filesize = *(UINT*)ptr;
			ptr+=4; 
			ptr+=4; //unknown, filesize again?
			ptr+=2; //unknown (0b 4d (always, so far)(port?, weird placement, if so and weird port num)
			WORD bitrate = *(WORD*)ptr;
			ptr+=2;
			int sample_frequency = *(int*)ptr;
			ptr+=4;
			int duration = *(int*)ptr;
			ptr+=4;
			//byte line_type = *(byte*)ptr;
			ptr++;
			//int unknown_ip = *(int*)ptr;
#ifdef _DEBUG
			int ip_2 = *(int*)ptr;
			in_addr addr2;
			addr2.S_un.S_addr = ip_2;
			CString ip_str2 = inet_ntoa(addr2);

#endif
			ptr+=4;
#ifdef _DEBUG
			CString username = (char*)ptr;
			CString path=file_path;
#endif
			//ptr+=username.GetLength()+1;
			ptr+=strlen((char*)ptr)+1;
			//CString upload_slots = (char*)ptr;
			GetFilename(file_path);
//			p_parent->ReceivedSearchHit(search_id,file_path,filesize,bitrate,sample_frequency,duration,(int)line_type,username,GetIP(ip),port,upload_slots);


			int ip1,ip2,ip3;
			char ip_str[16];
			in_addr addr;
			addr.S_un.S_addr = ip;
			strcpy(ip_str,inet_ntoa(addr));;
			sscanf(ip_str,"%d.%d.%d",&ip1,&ip2,&ip3);

			//this result is from us
			if( (filesize % 137 == 0) )
			{
				status->IncrementFakeTrackQueryHitCount(0);
				return;
			}
			int track = -1;
			bool music_project=false;
			if(this_psq.v_keywords.size()>0)
				music_project=true;

			//check for required keywords and assign associated track number
			CString lowered_file_path = file_path;
			lowered_file_path.MakeLower();
			int weight = CheckRequiredKeywords(this_psq,lowered_file_path,filesize);
			if(weight >= 100)
			{
				if(music_project)
					track=CheckQueryHitRequiredKeywords(this_psq,lowered_file_path,filesize);
				else
					track=0;
			}
			else
				return;

			if( (music_project && track >0) || !music_project)
			{
				// Save this query hit to be reported back to the supply manager
				status->IncrementTrackQueryHitCount(track);
				QueryHit qh;
				qh.m_search_id = this_psq.m_search_id;
				qh.m_port=port;
				qh.m_ip=ip;
				qh.m_file_size=filesize;
				qh.m_filename = file_path;
				qh.m_track=track;
				qh.m_project = this_psq.m_project.c_str();
				qh.m_timestamp = CTime::GetCurrentTime();
				memcpy(qh.m_hash, hash, 16);
				qh.m_project_id = this_psq.m_project_id;
				qh.m_bitrate=bitrate;
				qh.m_sample_frequency=sample_frequency;
				qh.m_duration=duration;

				query_hits->push_back(qh);
			}
			break;
		}
		default:
		{
#ifdef _DEBUG
			TRACE("Unknown msg type: %d\n",msg_type);
			char tmp_buf[1024];
			::ZeroMemory(&tmp_buf,1024);
			memcpy(tmp_buf,data,len);
#endif
			break;
		}
	}
}

//
//
//
void WinMXSocket::ConnectedToSupernode()
{
	m_status_data.m_up_time=CTime::GetCurrentTime();	// we are now "up"
	m_connection_state = Connected;

/*
	CString log;
	in_addr in;
	in.S_un.S_addr = m_dwParentIP;
	CString address = inet_ntoa(in);
	address.AppendFormat(":%d",m_wParentTCPPort);
	log.Format("Connected to %s", address);
//	p_parent->Log(log);
*/
	SendQueStatus(6, 6, 0);
	
	//SendFormatMessage(WTID_ENUMCHATROOM, "D", 0);
	m_dwMinTimer = 0;
}

//
//
//
int WinMXSocket::Search(const char* str, int search_type, int search_id)
{
	if(m_connection_state!=Connected)
		return FALSE;
	int buf_len = (int)strlen(str)+1+13;
	BYTE* buf=new BYTE[buf_len];
	memset(buf,0,buf_len);

	*(WORD *)buf = WTID_SEARCH;
	*(WORD *)(buf+2) = (WORD)buf_len-4;
	*(int *)(buf+4) = (int)search_id;
	*(BYTE *)(buf+10) = (BYTE)search_type;
	lstrcpy((char*)buf + 12, str);
	
	m_dwUPKey = EncryptMXTCP((BYTE *)buf, buf_len, m_dwUPKey);

	SendSocketData((unsigned char*)buf,buf_len);
	delete [] buf;

//	CString log;
//	log.Format("Searching \"%s\" , search ID: %d",str,m_search_id);
//	p_parent->Log(log);
	return TRUE;
}

//
//
//
void WinMXSocket::GetFilename(CString& path)
{
	int index = path.ReverseFind('\\');
	if(index > 0)
		path.Delete(0,index+1);
}

//
//
//
const char* WinMXSocket::GetIP(int ip)
{
	in_addr in;
	in.S_un.S_addr = ip;
	return inet_ntoa(in);	
}

//
//
//
void WinMXSocket::SendStopSearching(int search_id)
{
	if(m_connection_state == Connected)
	{
		unsigned char id[4];
		*(int*)id = search_id;
		SendMessage(WTID_STOP_SEARCHING,4,id);
	}
}

//
//
//
bool WinMXSocket::IsConnected()
{
	if(m_connection_state == Connected)
		return true;
	else
		return false;
}

//
//
//
SupernodeHost WinMXSocket::ReturnHostIP()
{
	return m_status_data.m_host;
}

//
//
//
void WinMXSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	if(m_connection_state == Connected)
	{
		// Make sure this is a fully connected socket
		if((IsSocket()==false) || (m_connection_state != Connected))
		{
			return;
		}
		switch(psq.m_search_type)
		{
			case ProjectKeywords::search_type::audio:
			{
				Search(psq.m_search_string.c_str(), 0x02, psq.m_search_id);
				break;
			}
			case ProjectKeywords::search_type::video:
			{
				Search(psq.m_search_string.c_str(), 0x03, psq.m_search_id);
				break;
			}
			case ProjectKeywords::search_type::software:
			{
				Search(psq.m_search_string.c_str(), 0x01, psq.m_search_id);
				break;
			}
		}
	}
}

//
//
//
void WinMXSocket::TimerHasFired()
{
	// Check to see if we are even a socket
	if(IsSocket()==false)
	{
		return;
	}

	// If we haven't fully connected within 30 seconds, this connection has timed out
	if(m_connection_state != Idle && m_connection_state != Connected)
	{
		if((CTime::GetCurrentTime()-m_status_data.m_connect_time).GetTotalSeconds()>30)
		{
			Close();
			return;
		}
	}

	// If connection established and and it's been 60 seconds since we last sent a ping, do it now
	if(m_connection_state == Connected)
	{
		m_dwMinTimer++;
		if(m_dwMinTimer >= 6) //5 sec timer, so 30 sec
		{
			if( (rand()%100)!=0)
			{
				int total_avail_conns = rand()%7 + 4;
				int free_avail_conns = rand()%total_avail_conns + 1;
				SendQueStatus(total_avail_conns,free_avail_conns, 0);
			}
			else
			{
				int total_avail_conns = rand()%7 + 4;
				int in_queue = rand()%50 + 1;
				SendQueStatus(total_avail_conns,0, in_queue);			
			}
			m_dwMinTimer = 0;
		}

#ifdef WINMX_SPOOFER
		SendShare();
#endif
	}
}

//
// Return Values (Track)
// ---------------------
// -1 : query hit is not a match
//  0 : query hit is a match but there are no required keywords, so it does not correspond with any track number
// >0 : query hit is a match and is associated with the specified track number
//
int WinMXSocket::CheckQueryHitRequiredKeywords(ProjectSupplyQuery& psq, const char *lfilename,unsigned int file_size)
{
	UINT i,j;

	// Check the file size
	if(file_size<psq.m_supply_size_threshold)
	{
		return -1;
	}

	// Check the kill words
	bool killed=false;
	for(i=0;i<psq.v_killwords.size();i++)
	{
		if(strstr(lfilename,psq.v_killwords[i])!=NULL)
		{
			return -1;
		}
	}
	
	// Check the required keywords, to determine the track.
	if(psq.v_keywords.size()==0)
	{
		return 0;
	}

	// Check all of the required keywords tracks
	for(i=0;i<psq.v_keywords.size();i++)
	{
		// Check the required keywords for this track
		bool found=true;
		for(j=0;j<psq.v_keywords[i].v_keywords.size();j++)
		{
			if(strstr(lfilename,psq.v_keywords[i].v_keywords[j])==NULL)
			{
				found=false;
				break;
			}
		}

		// If we found a matching track
		if(found)
		{
			return psq.v_keywords[i].m_track;
		}
	}
	
	// None of the required keywords matched
	return -1;
}

//
//
//
int WinMXSocket::CheckRequiredKeywords(ProjectSupplyQuery& psq,const char *lfilename, UINT filesize)
{
	// Check the file size
	if(filesize<psq.m_supply_size_threshold)
	{
		return -1;
	}

	int weight=0;
	UINT j,k;
	// Extract the keywords from the supply
	vector<string> keywords;
	ExtractKeywordsFromSupply(&keywords,lfilename);

	// Check all of the keywords for this project
	for(j=0;j<psq.m_query_keywords.v_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strstr(keywords[k].c_str(),psq.m_query_keywords.v_keywords[j].keyword.c_str())!=0)
			{
				weight+=psq.m_query_keywords.v_keywords[j].weight;
			}
		}
	}

	// Check all of the exact keywords for this project
	for(j=0;j<psq.m_query_keywords.v_exact_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strcmp(keywords[k].c_str(),psq.m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
			{
				weight+=psq.m_query_keywords.v_exact_keywords[j].weight;
			}
		}
	}

	// If the weight is >= 100, then this is a query match for the current project.  Check with the kill words, just in case
	if(weight>=100)
	{
		for(j=0;j<keywords.size();j++)
		{
			// See if any of the project killwords are within the query's keywords
			bool found=false;
			for(k=0;k<psq.m_query_keywords.v_killwords.size();k++)
			{
				if(strstr(keywords[j].c_str(),psq.m_query_keywords.v_killwords[k].keyword.c_str())!=0)
				{
					found=true;
					weight=0;
					break;
				}
			}

			// If it didn't match a project killword, see if any of the project exact killwords are equal to the query's keywords
			if(!found)
			{
				for(k=0;k<psq.m_query_keywords.v_exact_killwords.size();k++)
				{
					if(strcmp(keywords[j].c_str(),psq.m_query_keywords.v_exact_killwords[k].keyword.c_str())==0)
					{
						weight=0;
						break;
					}
				}
			}

			// If the query contained a killword, the weight has been reset to 0...so move on to the next project
			if(weight==0)
			{
				break;
			}
		}
	}
	return weight;
}

//
//
//
void WinMXSocket::ExtractKeywordsFromSupply(vector<string> *keywords,const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	string cstring_string=cstring;
	strcpy(lbuf,cstring_string.c_str());

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<keywords->size();i++)
		{
			if(strcmp((*keywords)[i].c_str(),ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void WinMXSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')))
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}

//
//
//
void WinMXSocket::ExtractHosts(const BYTE* data, int len)
{
	int number_of_hosts = len/12;
	const BYTE* ptr = data;
	for(int i=0;i<number_of_hosts; i++)
	{
		int ip = *(int*)ptr;
		ptr += 6;
		short port = *(short*)ptr;
		ptr += 6;
		SupernodeHost	host;
		host.m_ip = ip;
		host.m_port = port;
		m_status_data.v_other_hosts.push_back(host);
	}
}

//
//
//
void WinMXSocket::ReportOtherHosts(vector<SupernodeHost>& hosts)
{
	for(UINT i=0;i<m_status_data.v_other_hosts.size();i++)
		hosts.push_back(m_status_data.v_other_hosts[i]);
	m_status_data.v_other_hosts.clear();
}

//
//
//
void WinMXSocket::SendShare(void)
{
#ifdef WINMX_SWARMER
	for(int i=0;i<WINMX_NUM_SHARE_SEND_PER_TIMER;i++)
	{
		if(m_num_shares_sent < WINMX_MAX_FILES_SHARE)
		{
			PacketBuffer* buf = p_sockets->p_file_sharing_manager->GetShare();
			if(buf==NULL)
				return;
			SendMessage(WTID_SHARED_LIST, buf->Size(),buf->p_data);
			delete buf;
			m_num_shares_sent++;
		}
		else
			break;
	}
#else
	PacketBuffer new_buf;
	CString folder_name;
	GetRandomShareFolderName(folder_name);
	//new_buf.PutStr("C:\\My Shared Folder\\",20);
	new_buf.PutStr(folder_name,folder_name.GetLength());
	new_buf.PutByte('\0');
	for(int i=0;i<WINMX_NUM_SHARE_SEND_PER_TIMER;i++)
	{
		if(m_num_shares_sent < WINMX_MAX_FILES_SHARE)
		{
			PacketBuffer* buf = p_sockets->p_file_sharing_manager->GetShare();
			if(buf==NULL)
				return;
			new_buf.Append(*buf);
			delete buf;
			m_num_shares_sent++;
		}
		else
			break;
	}
	SendMessage(WTID_SHARED_LIST, new_buf.Size(),new_buf.p_data);
#endif
}

//
//
//
void WinMXSocket::GetRandomShareFolderName(CString& folder)
{
	CString username;
	p_sockets->GetRandomUserName(username);
	username.Delete(username.GetLength()-3,3);
	folder.Format("C:\\Documents and Settings\\%s\\My Documents\\My Music\\",username);
}