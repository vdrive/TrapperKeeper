#include "StdAfx.h"
#include "SLSKsocket.h"
#include "ConnectionSockets.h"
#include "SLSKSpooferDlg.h"
//#include "SLSKaccountInfo.h"
#include "SLSKToDoList.h"
#include "db.h"
#include <string.h>
//#include <CString.h>
using std::string;
#include "ProjectKeywordsVector.h"
#include <fstream>



//#include "EncryptionType2.h"
//#include "cryptlib.h"	// for CryptoPP
//#include "FileSharingManager.h"

int SLSKSocket::m_peer_connections = 0;
int SLSKSocket::m_parent_connections = 0;
int SLSKSocket::m_server_connections = 0;
int SLSKSocket::m_peer_connecting = 0;
int SLSKSocket::m_parent_connecting = 0;
//CString SLSKSocket::m_user_name = "";


SLSKSocket::SLSKSocket(void)
{
	srand((unsigned)time(NULL));
	m_logged_in = false;
	m_first_query = true;
	m_time_last_parent_sent=CTime::GetCurrentTime();
	send73 = true;
	sendparent = true;
//	m_buf_offset = 0;
	buffer_total_size = 4;
	m_spoofed = false;
	//loggingin = 0;
	//numofconnections = 0;
	//srand((unsigned)time( NULL ));
	//p_in_cipher = p_out_cipher = NULL;
	//ResetData();
	m_number_of_folders = 300;
	m_number_of_files = 5000;
	m_connected_parent = false;
	m_version = 222;
	m_state_connecting = 0;

	
}

//
//
//
SLSKSocket::~SLSKSocket(void)
{
	//ResetData();
	//if(p_in_cipher)
	//	delete p_in_cipher;
	//if(p_out_cipher)
	//	delete p_out_cipher;

}

//
//
//
//
//
//

void SLSKSocket::InitParent(ConnectionSockets* parent)
{
	p_sockets = parent;

}

void SLSKSocket::InitTasks(vector<SLSKtask> t1, vector<SLSKtask> t2)
{
//	st = t1;
//	pt = t2;
}



/*void SLSKSocket::InitDialog(CSLSKSpooferDlg *parent)
{
	dlg = parent;
}
*/
/*void SLSKSocket::InitAccountInfo(SLSKaccountInfo &sai)
{
	account_info = sai;
}*/
//
//
//
/*void SLSKSocket::ResetData()
{
	m_in_xinu=0x51;
	m_out_xinu=0x51;
	m_state = SessNew;
	if(p_in_cipher != NULL)
	{
		delete p_in_cipher;
	}
	if(p_out_cipher != NULL)
	{
		delete p_out_cipher;
	}

	p_in_cipher = new FSTCipher;
	p_out_cipher = new FSTCipher;
	memset(p_in_cipher,0,sizeof(FSTCipher));
	memset(p_out_cipher,0,sizeof(FSTCipher));

	p_out_cipher->seed = 0x0FACB1238;
	p_out_cipher->enc_type = 0x29;
	m_received_header = false;
	m_message_type = 0;
	m_message_length = 0;
//	m_search_id = 0;
	m_status_data.Clear();
	m_ready_to_send_shares = false;
	m_num_shares_sent=0;
}*/

//
//
//
int SLSKSocket::Connect(CString un, SupernodeHost host)
{
	m_parent_connecting++;

	m_first_query = true;
	char *tempstring = new char[10];
	sprintf(tempstring, "%d", m_parent_connecting);
	p_sockets->setParentConnecting(tempstring);

	m_user_name = un;
	m_status_data.m_host = host;
	m_status_data.m_connect_time = CTime::GetCurrentTime();
	int c = Create();
	//TRACE("The value returned from Create() = %d\n", c);
	char ip[16];
	memset(&ip, 0, 16);
	host.GetIP(ip);

	connection_ip = ip;
	port = (int)host.getport();
	m_state_connecting = 1;
	int result = TEventSocket::Connect((char *)(LPCTSTR)connection_ip,host.m_port);

//	char* resultstring = new char[100];
//	sprintf(resultstring, "Connecting a parent... Connect returned = %d", result);
//	p_sockets->WriteToLog(resultstring);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != 10035)
		{
			char* errorstring = new char[100];
			sprintf(errorstring, "Connect error code = %d", error);
			p_sockets->WriteToLog(errorstring);
		}

	}

    
	//TRACE("The Connect value = %d\n", result);
	return result;
}

int SLSKSocket::Connect(CString un, SLSKtask *t)
{
	m_peer_connecting++;
	
    char *tempstring = new char[10];
	sprintf(tempstring, "%d", m_peer_connecting);
	p_sockets->setPeerConnecting(tempstring);
//	m_task = t;
//	delete t;
//	if (t->m_state != 1)
//	Sleep(1000);
	m_spoofed = false;
	//m_peer_connecting++;
	//char* tempstring = new char[10];
	//sprintf(tempstring, "%d", m_peer_connecting);
	//p_sockets->setPeerConnecting(tempstring);
  	m_user_name = un;
	SupernodeHost host;
	host.SetIP(t->m_ip);

	if (abs(t->m_port) == 65535 || t->m_port == 0)
	{
		int a = 0;
	}

	host.setport(t->m_port);
//	m_status_data.m_host = host;
//	m_status_data.m_connect_time = CTime::GetCurrentTime();
	Create();
	char ip[16];
	memset(&ip, 0, 16);
	host.GetIP(ip);

	tasks.push_back(t);
	m_state_connecting = 1;
	//delete t;

	//int s = tasks.size();
	connection_ip = ip;
	port = (int)host.getport();


	return TEventSocket::Connect((char *)(LPCTSTR)connection_ip,host.m_port);
}


int SLSKSocket::Connect(SupernodeHost host)
{
	//m_server_connections++;
	//CString s;
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
	//s.Append("In Connect function\n");
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);
	m_status_data.m_host=host;

	m_status_data.m_connect_time=CTime::GetCurrentTime();
//	m_state = SessConnecting;
	Create();
	char ip[16];
	memset(&ip,0,16);
	host.GetIP(ip);

	connection_ip = ip;
	port = (int)host.getport();


	p_sockets->Log("Connecting to main server...");
	m_logged_in = false;
//	CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
	//s.Append(connection_ip);
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);

	//CString s;
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
	//s.Append(connection_ip);
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);
	return TEventSocket::Connect((char *)(LPCTSTR)connection_ip,host.m_port);
}

//
//
//
int SLSKSocket::Close(int error_code)
{
	m_state_connecting = 0;

	if (p_sockets->state == 3)
	{
		if (m_spoofed == true)
		{
		m_peer_connections--;
		//CString temp;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_peer_connections);
		//tempstring.PutInt(m_peer_connections);
		//temp.AppendFormat("%d", m_peer_connections);
	//	char* tempstring = temp;
		p_sockets->PeerStatus(tempstring);
		}
		
		if (m_spoofed == false)
		{
			m_peer_connecting--;
			char* tempstring = new char[10];
			sprintf(tempstring, "%d", m_peer_connecting);
			p_sockets->setPeerConnecting(tempstring);
		}

	//	for (int i = 0; i < (int)spoofs.size(); i++)
	//	{
	//		SendSocketData((unsigned char*)spoofs[i]->GetCharPtr(), (int)spoofs[i]->Size());
//			delete spoofs[i];
	//	}
		if (tasks.size() > 0)
			p_sockets->SendTask(tasks[0]);

		spoofs.clear();
	}

	if (p_sockets->state == 2)
	{
		SupernodeHost *nh = new SupernodeHost();
		nh->SetIP(connection_ip);
		nh->setport(port);
		p_sockets->DropCachedParent(nh);
	//	if (m_connected_parent)
	//	{
			if (m_first_query == false)
			{
				if (m_parent_connections > 0)
				m_parent_connections--;
				char *tempstring = new char[10];
				sprintf(tempstring, "%d", m_parent_connections);
				//tempstring.PutInt(m_parent_connections);
			//	CString temp;
				//temp.AppendFormat("%d", m_parent_connections);
			//	char* tempstring = temp;
				p_sockets->ParentStatus(tempstring);
			}
			
			if (m_first_query == true)
			{
				if (m_parent_connecting > 0)
				m_parent_connecting--;
				char *tempstring = new char[10];
				sprintf(tempstring, "%d", m_parent_connecting);
				p_sockets->setParentConnecting(tempstring);
			}
			m_state_connecting = 0;
		//	int result = TEventSocket::Close();
		//	char* resultstring = new char[100];
		//	sprintf(resultstring, "Closing Parent... Close returned = %d", result);
		//	p_sockets->WriteToLog(resultstring);
			//TRACE("The close() value = %d\n", result);
			//return result;
		//}
	}
	int s = p_sockets->state;
	if (p_sockets->state == 1)
	{
		if (m_logged_in == true)
		{
			m_server_connections--;
			char* tempstring = new char[10];
			sprintf(tempstring, "%d", m_server_connections);
			p_sockets->ServerStatus(tempstring);
		}
		TRACE("Server socket got closed. Error code: %d\n", error_code);
	}
	return TEventSocket::Close();
}

//
//
//
void SLSKSocket::IncrementDemandCounter(char* artist)
{
	for (int i = 0; i < (int)p_sockets->daily_demand.size(); i++)
	{
		if (strcmp((const char*)(LPCTSTR)(p_sockets->daily_demand[i].m_artist), artist) == 0)
		{
			p_sockets->daily_demand[i].m_counter++;
			break;
		}
	}
}

void SLSKSocket::SendCantConnect(SLSKtask *t)
{
	buffer buf;
	buf.PutInt(0);
	buf.PutInt(1001);
	buf.PutInt(t->m_token);
	buf.PutInt((int)strlen((const char*)t->m_user_name.GetString()));
	buf.PutCharPtr((const char*)t->m_user_name.GetString(), (int)strlen((const char*)t->m_user_name.GetString()));
	buf.SetInt(buf.Size() - 4, 0);
	SendSocketData((unsigned char*)buf.GetCharPtr(), (unsigned int)buf.Size());
	delete t;
}

void SLSKSocket::GetPeerIP(SLSKtask *task)
{
	
	if (m_logged_in == false)
	{
		p_sockets->Log("Not Logged In..... Deleting task!");
		delete task;
		return;
	}

	p_sockets->Log("Getting peer ip...");
	buffer newbuffer;

//	char* tempstring = task->m_user_name;
	//p_sockets->Log((const char*)(LPCTSTR)task->m_user_name);
	newbuffer.PutInt(0);
	newbuffer.PutInt(3);
	newbuffer.PutInt((int)strlen(task->m_user_name));
	newbuffer.PutCharPtr(task->m_user_name, (int)strlen(task->m_user_name));

//	CString s = "Peer user name = ";
//	s.Append(task->m_user_name);
//	p_sockets->Log((const char*)(LPCTSTR)s);

	tasks.push_back(task);
	newbuffer.SetInt((int)newbuffer.Size() - 4, 0);

	int ret = SendSocketData((unsigned char*)newbuffer.GetCharPtr(), (unsigned int)newbuffer.Size());
	int error=0;
	if(!ret)
		error=GetLastError();

	

	//delete task;
}



void SLSKSocket::OnConnect(int error_code)
{
	//buffer buf;
	

	if (error_code != 0)
	{
		if (p_sockets->state == 3)
		{
			if (tasks[0]->m_port == 0 || abs(tasks[0]->m_port) > 65535)
			{
				int a = 0;
			}
			p_sockets->Log("ERROR CLOSING PEER SOCKET!");
			if (tasks.size() > 0)
			{
				delete tasks[0];
				tasks.clear();
			}
		}

		if (p_sockets->state == 2)
		{
			int error = WSAGetLastError();

			char* errorstring = new char[100];
			sprintf(errorstring, "error_code = %d ... onconnect error code = %d", error_code, error);
			p_sockets->WriteToLog(errorstring);
		}
	/*	if (p_sockets->state == 3)
		{
			m_peer_connections++;
			for (int i = 0; i < (int)spoofs.size(); i++)
			{
			//	SendSocketData((unsigned char*)spoofs[i]->GetCharPtr(), (int)spoofs[i]->Size());
				delete spoofs[i];
			}
			spoofs.clear();
		
		//p_sockets->Log("On ");
		char tempstring[75];
		sprintf(tempstring, "In ON Connect Close error code objects in spoofs = %d", (int)spoofs.size());
		//tempstring.PutInt((int)spoofs.size());
	//	temp.AppendFormat("%d", spoofs.size());
		//char* s = temp;
		p_sockets->Log(tempstring);
		}*/
		Close();
		return;
		
	}

	

	if (p_sockets->state == 1)
	{
		m_server_connections++;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_server_connections);
		p_sockets->ServerStatus(tempstring);
		//login in
		buffer buf2;
		buf2.PutInt(0);
		buf2.PutInt(1);
		ifstream fin;
		char un[30];
		char sessioncode[33];
		//CSLSKSpooferDlg::GetInstance()->connectioncount++;
		//int a = CSLSKSpooferDlg::GetInstance()->connectioncount;
		if (m_main_server)
		{
			fin.open("C:\\user1info.txt", ios::in);
			fin.getline(un, 30, '\n');
			fin.getline(sessioncode, 33, '\n');
			buf2.PutInt((int)strlen(un));	
			buf2.PutCharPtr(un, (int)strlen(un));
			buf2.PutInt(8);
			buf2.PutCharPtr("asdfasdf", 8);
			buf2.PutInt(155);
			buf2.PutInt(32);
			buf2.PutCharPtr(sessioncode, 32);
			buf2.PutInt(m_version);
		}
		else
		{
			
			fin.open("C:\\user2info.txt", ios::in);
			fin.getline(un, 30, '\n');
			fin.getline(sessioncode, 32, '\n');
			buf2.PutInt((int)strlen(un));
			buf2.PutCharPtr(un, (int)strlen(un));
			buf2.PutInt(8);
			buf2.PutCharPtr("asdfasdf", 8);
			buf2.PutInt(155);
			buf2.PutInt(32);
			buf2.PutCharPtr(sessioncode, 32);
			buf2.PutInt(m_version);

		}

		//m_user_name = p_sockets->getUserName();
		//p_sockets->SetUserName(m_user_name);
		//m_password = p_sockets->getPassword();
		////	p_sockets->Log((const char*)m_user_name);
		////	p_sockets->Log((const char*)m_password);
		//buf2.PutInt((int)strlen(m_user_name));
		//buf2.PutCharPtr(m_user_name, (int)strlen(m_user_name));
		//buf2.PutInt((int)strlen(m_password));
		//buf2.PutCharPtr(m_password, (int)strlen(m_password));
		//buf2.PutInt(m_version);
		buf2.SetInt((int)buf2.Size() - 4, 0 );

p_sockets->Log("Sending Handshake for main server...");
		//p_sockets->Log((const char*)(LPCTSTR)buf2.GetCharPtr());
		//loggingin++;
fin.close();
		SendSocketData((unsigned char*)buf2.GetCharPtr(), (unsigned int)buf2.Size());

		return;
	}

	if (p_sockets->state == 3)
	{
	//	TRACE("IN ON CONNECT\n");

		if (tasks[0]->m_port == 0 || abs(tasks[0]->m_port) > 65535)
			{
				int a = 0;
			}
m_state_connecting = 2;
char *tempstring;
m_peer_connecting--;
tempstring = new char[10];
sprintf(tempstring, "%d", m_peer_connecting);
p_sockets->setPeerConnecting(tempstring);

m_peer_connections++;
tempstring = new char[10];
sprintf(tempstring, "%d", m_peer_connections);
p_sockets->PeerStatus(tempstring);

buffer newbuffer;
		newbuffer.PutInt(0);
		newbuffer.PutByte(1);
		if (tasks[0]->m_state == 1)
		{
			CString s_name = p_sockets->getSpoofName();
		newbuffer.PutInt((int)strlen((const char*)(LPCTSTR)s_name));//11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr((char*)(LPCTSTR)s_name, (int)strlen((const char*)(LPCTSTR)s_name));//, 11);//m_user_name, (int)strlen(m_user_name));
	//	p_sockets->Log("Spoof name = ");
	//	p_sockets->Log((char*)(LPCTSTR)s_name);
		}
		if (tasks[0]->m_state == 2)
		{
			CString s_name = p_sockets->getSpoofName();
		newbuffer.PutInt((int)strlen((const char*)(LPCTSTR)s_name));//11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr((char*)(LPCTSTR)s_name, (int)strlen((const char*)(LPCTSTR)s_name));
			/*
		newbuffer.PutInt(11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr("HAPPYPUSSY2", 11);//m_user_name, (int)strlen(m_user_name));*/
		}
		if (tasks[0]->m_state == 3)
		{
			CString s_name = p_sockets->getSpoofName();
		newbuffer.PutInt((int)strlen((const char*)(LPCTSTR)s_name));//11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr((char*)(LPCTSTR)s_name, (int)strlen((const char*)(LPCTSTR)s_name));
		/*newbuffer.PutInt(11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr("HAPPYPUSSY3", 11);//m_user_name, (int)strlen(m_user_name));*/
		}
		if (tasks[0]->m_state == 4)
		{
			CString s_name = p_sockets->getSpoofName();
		newbuffer.PutInt((int)strlen((const char*)(LPCTSTR)s_name));//11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr((char*)(LPCTSTR)s_name, (int)strlen((const char*)(LPCTSTR)s_name));
		/*newbuffer.PutInt(11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr("HAPPYPUSSY4", 11);//m_user_name, (int)strlen(m_user_name));*/
		}
		if (tasks[0]->m_state == 5)
		{
			CString s_name = p_sockets->getSpoofName();
		newbuffer.PutInt((int)strlen((const char*)(LPCTSTR)s_name));//11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr((char*)(LPCTSTR)s_name, (int)strlen((const char*)(LPCTSTR)s_name));
	/*	newbuffer.PutInt(11);//(int)strlen(m_user_name));
		newbuffer.PutCharPtr("HAPPYPUSSY5", 11);//m_user_name, (int)strlen(m_user_name));*/
		}
		newbuffer.PutInt(1);
		newbuffer.PutChar('P');
		newbuffer.PutInt(0);
		newbuffer.SetInt((int)newbuffer.Size() - 4, 0);

		SendSocketData((unsigned char*) newbuffer.GetCharPtr(), (int)newbuffer.Size());

		p_sockets->Log("Handshake Sent!!");

//TRACE("HANDSHAKE SENT\n");
		m_spoofed = true;
		
		//m_peer_connecting--;
		//char* tempstring = new char[10];
		//sprintf(tempstring, "%d", m_peer_connecting);
		//p_sockets->setPeerConnecting(tempstring);
	
	//	m_peer_connections++;
		//tempstring = new char[10];
		//sprintf(tempstring, "%d", m_peer_connections);
		//tempstring.PutInt(m_peer_connections);
	//	CString s;
	//	s.AppendFormat("%d", m_peer_connections);
	//	char* tempstring = s;
		//p_sockets->PeerStatus(tempstring);


		int index = 0, i =0, j = 0, compsize = 0;
		//spoof tempspoof;
		char zipped[4096];
		UINT zippedsize = 4096;
				if (i < 0)
					{
						int h = 0;
					}
		int a = (int)p_sockets->m_spoofs.size();
		for  (i = 0; i < (int)p_sockets->m_spoofs.size(); i++)
		{
			if (i < 0)
					{
						int h = 0;
					}
			//tempspoof = p_sockets->m_spoofs[i];
			if(tasks[0]->m_artist == p_sockets->m_spoofs[i].m_artist)//tempspoof.m_artist)
			{
				if (j < 0)
					{
						int h = 0;
					}
			//	int s = (int)tempspoof.m_spoof.size();
				for (j = 0; j < 1; j++)//(int)tempspoof.m_spoof.size(); j++)
				{
					if (j < 0)
					{
						int h = 0;
					}
				//	TRACE("J = %d \n", j);
					j = rand() % 10;//j+tasks[0]->m_state;
				//	TRACE("J = %d and state = %d\n", j, tasks[0]->m_state);

					index = 0;
					index += 4 + p_sockets->m_spoofs[i].m_spoof[j]->GetInt(0);//tempspoof.m_spoof[j]->GetInt(0);
				//	TRACE("ADDING TOKEN!\n");
					p_sockets->m_spoofs[i].m_spoof[j]->SetInt(tasks[0]->m_token, index);
				//	TRACE("FINISHED ADDING TOKEN!\n");

					strcpy(zipped, "");
					zippedsize = 4096;
					//TRACE("COMPRESSING!\n");
					compsize = compress((Bytef*)zipped, (uLong *)&zippedsize, (Bytef*)p_sockets->m_spoofs[i].m_spoof[j]->GetCharPtr(), (uLong)p_sockets->m_spoofs[i].m_spoof[j]->Size());

				//	TRACE("DONE COMPRESSING\n");
					if (compsize != Z_OK)
					{
						::MessageBox(NULL, "ERROR COMPRESSING THE SPOOFER", "COMPRESSION ERROR!", MB_OK);
					}

					//	TRACE("NEW BUFFER!\n");			
					buffer sendbuffer;
					sendbuffer.PutInt(0);
					sendbuffer.PutInt(9);
					sendbuffer.PutCharPtr(zipped, zippedsize);
					//delete buf;
					sendbuffer.SetInt((int)sendbuffer.Size() - 4, 0);


				//	TRACE("DONE NEW BUFFER!\n");
 					SendSocketData((unsigned char*)sendbuffer.GetCharPtr(), (int)sendbuffer.Size());
				//	TRACE("SENT NEW BUFFER!\n");
				//	TRACE("J = %d\n", j);
					
				}
				break;
			}
		}
//TRACE("AFTER COMPRESSION\n");
		
	//number 2



/*		buffer newbuffer2;
		newbuffer2.PutInt(0);
		newbuffer2.PutByte(1);
		newbuffer2.PutInt(11);//(int)strlen(m_user_name));
		newbuffer2.PutCharPtr("POLYPRISSYPANTS", 11);//m_user_name, (int)strlen(m_user_name));
		newbuffer2.PutInt(1);
		newbuffer2.PutInt(80);
		newbuffer2.PutByte(0);
		newbuffer2.SetInt((int)newbuffer.Size() - 4, 0);

		SendSocketData((unsigned char*) newbuffer2.GetCharPtr(), (int)newbuffer2.Size());

	

		for  (i = 0; i < (int)p_sockets->m_spoofs.size(); i++)
		{
			
			tempspoof = p_sockets->m_spoofs[i];
			if(tasks[0]->m_artist == tempspoof.m_artist)
			{
				for (j = 0; j < (int)tempspoof.m_spoof.size(); j++)
				{
					index = 0;
					index += 4 + tempspoof.m_spoof[j]->GetInt(0);
					tempspoof.m_spoof[j]->SetInt(tasks[0]->m_token, index);

					strcpy(zipped, "");
					zippedsize = 1024;
					compsize = compress((Bytef*)zipped, (uLong *)&zippedsize, (Bytef*)tempspoof.m_spoof[j]->GetCharPtr(), (uLong)tempspoof.m_spoof[j]->Size());

					if (compsize != Z_OK)
					{
						::MessageBox(NULL, "ERROR COMPRESSING THE SPOOFER", "COMPRESSION ERROR!", MB_OK);
					}

									
					buffer sendbuffer;
					sendbuffer.PutInt(0);
					sendbuffer.PutInt(9);
					sendbuffer.PutCharPtr(zipped, zippedsize);
					//delete buf;
					sendbuffer.SetInt((int)sendbuffer.Size() - 4, 0);


 					SendSocketData((unsigned char*)sendbuffer.GetCharPtr(), (int)sendbuffer.Size());
				}
			}
		}
*/

		// end number 2

		
		p_sockets->Log("Spoofs Sent!!!");
		

		if (tasks.size() > 0)
		{
			delete tasks[0];
			tasks.clear();
		}
	
		
		Close();
	//	TRACE("SENT DONE\n");
		return;

	}
	/*	SLSKtask *temp;
    	vector<SLSKtask*>::iterator iter = CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.begin();
	
		for (int i = 0; i < (int)CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.size(); i++, iter++)
		{
			temp = CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer[i];

			if (strcmp(connection_ip, temp->getIP()) == 0)
			{
				SendSocketData(temp->getBuf().GetMutableUcharPtr(), (unsigned int)temp->getBuf().Size());
				delete CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer[i];
				CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.erase(iter);

				if (CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.size() == 0)
					CSLSKSpooferDlg::GetInstance()->todo.ctp = false;

			//	Close();
				break;
			}
		}
		
			return;
	}

	if(error_code!=0)
	{
		if (p_sockets->state == 3)
		{
			SLSKtask *temp;
    		vector<SLSKtask*>::iterator iter = CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.begin();
	
			for (int i = 0; i < (int)CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.size(); i++, iter++)
			{
				temp = CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer[i];

				if (strcmp(connection_ip, temp->getIP()) == 0)
				{
					CSLSKSpooferDlg::GetInstance()->todo.connectingtome.push_back(temp);
					delete CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer[i];
					CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.erase(iter);

					CSLSKSpooferDlg::GetInstance()->todo.ctm = true;

					if (CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.size() == 0)
						CSLSKSpooferDlg::GetInstance()->todo.ctp = false;
				}

			}
		}
		Close();
		return;*/
	
	//	Close();
		

	if (p_sockets->state == 2)
	{
		m_parent_connecting--;
		char *tempstring;
		tempstring = new char[10];
		sprintf(tempstring, "%d", m_parent_connecting);
		p_sockets->setParentConnecting(tempstring);

		m_parent_connections++;
		tempstring = new char[10];
		sprintf(tempstring, "%d", m_parent_connections);
		p_sockets->ParentStatus(tempstring);

		m_state_connecting = 2;
	    buffer newbuf;
		newbuf.PutInt(0);
		newbuf.PutByte(1);
 		newbuf.PutInt((int)strlen("John3854"));
		newbuf.PutCharPtr("John3854", (int)strlen("John3854"));
		newbuf.PutInt(1);
		newbuf.PutChar('D');
		newbuf.PutInt(0);

		newbuf.SetInt((int)newbuf.Size() - 4, 0);
		m_time_sent_handshake = CTime::GetCurrentTime();
		SendSocketData((unsigned char*)newbuf.GetCharPtr(), (unsigned int)newbuf.Size());
	}

		return;
	}


//
//
//

void SLSKSocket::SomeSocketDataReceived(char* data, unsigned int data_len, unsigned int new_len, unsigned int max_len)
{
	buffer newbuffer;

	if (buffer_total_size == 4)
	{
		newbuffer.PutCharPtr(data, new_len);
		buffer_total_size = newbuffer.GetInt(0);
		if (buffer_total_size > 100,000)
			Close();
		return;
	}

	newbuffer.PutCharPtr(data, new_len);
	reply(newbuffer);
	buffer_total_size = 4;
}

void SLSKSocket::SocketDataSent(unsigned int len)
{
}

void SLSKSocket::SocketDataReceived(char* data, unsigned int len)
{
	buffer newbuffer;

	if (len > 500000 || len == 0)
	{
		Close();
		return;
	}

	if (buffer_total_size == 4)
	{
		newbuffer.PutCharPtr(data, len);
		buffer_total_size = newbuffer.GetInt(0);
		return;
	}

	newbuffer.PutCharPtr(data, len);
	if (len == 1)
	{
		int i = 2;
	}
	reply(newbuffer);
	buffer_total_size = 4;
}

void SLSKSocket::OnReceive(int error_code)
{
	if (error_code != 0)
	{
		Close();
		return;
	}

	if (m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	if (buffer_total_size == 4)
	{
		ReceiveSocketData(buffer_total_size);
		return;
	}

	if (buffer_total_size != 4)
	{
		if (buffer_total_size > 500000 || buffer_total_size < 0)
		{
			Close();
			return;
		}

		ReceiveSocketData(buffer_total_size);
	}

}


void SLSKSocket::reply(buffer &reply_buffer)
{
	buffer buf;
	buf.PutCharPtr((const char*)reply_buffer.GetCharPtr(), reply_buffer.Size());

	int index = 0;
	CString s;


	int reply;
	//m_buf.DropFront(4);
	if (buf.Size()>= 4)
	   reply = buf.GetInt(index);
	else 
		return;

	//CString s;
	//buffer newbuffer;
//int a = 0;
	switch(reply)
	{
	case 1:
		{
		//CString temp;
		index += 4;
		if (buf.GetByte(index) == 1)
		{
	
		//	m_server_connections++;
			//char *tempstring = new char[10];
			//sprintf(tempstring, "%d", m_server_connections);
		//	CString stemp;
		//	stemp.AppendFormat("%d", m_server_connections);
		//	char* tempstring = stemp;
			//p_sockets->ServerStatus(tempstring);
            p_sockets->Log("Logged into main server successfully!");
			m_logged_in = true;

			buffer newbuffer1, newbuffer2;
			newbuffer1.PutInt(0);
			newbuffer1.PutInt(36);
			newbuffer1.PutInt((int)strlen(m_user_name));
			newbuffer1.PutCharPtr(m_user_name, (int)strlen(m_user_name));
			newbuffer1.SetInt((int)newbuffer1.Size() - 4, 0);
			SendSocketData((unsigned char*)newbuffer1.GetCharPtr(), (unsigned int)newbuffer1.Size());
			
			//buffer newbuffer2;
			newbuffer2.PutInt(8);
			newbuffer2.PutInt(2);
			newbuffer2.PutInt(ListeningPort);
			newbuffer2.PutInt(12);
			newbuffer2.PutInt(35);
			newbuffer2.PutInt(0);
			newbuffer2.PutInt(0);
			newbuffer2.PutInt(5);
			newbuffer2.PutInt(71);
			newbuffer2.PutByte(1);
			m_time_last_parent_sent = CTime::GetCurrentTime();
			SendSocketData((unsigned char*)newbuffer2.GetCharPtr(), (unsigned int)newbuffer2.Size());


		}
		else
		{
			p_sockets->Log("Log in failed!!!");
	
			if (m_server_connections < 1)
			{
				m_user_name = p_sockets->getUserName();
				p_sockets->SetUserName(m_user_name);
				m_password = p_sockets->getPassword();
			}

			buffer newbuffer;
			newbuffer.ResizeBuffer(0);
			newbuffer.PutInt(0);
		    newbuffer.PutInt(1);
			newbuffer.PutInt((int)strlen(m_user_name));
			newbuffer.PutCharPtr(m_user_name, (int)strlen(m_user_name));
			newbuffer.PutInt((int)strlen(m_password));
			newbuffer.PutCharPtr(m_password, (int)strlen(m_password));
			newbuffer.PutInt(m_version);
			newbuffer.SetInt((int)buf.Size() - 4, 0 );

			SendSocketData((unsigned char*)newbuffer.GetCharPtr(), (unsigned int)newbuffer.Size());

			
			
		}

		buf.ResizeBuffer(0);
		break;
		}
	case 3:
		{

			p_sockets->Log("Got IP of User!!");
			index += 4;
			int name_size = buf.GetInt(index);
			index += 4;
			char *name = new char[name_size + 1];
			memcpy(name, buf.GetCharPtr(index), name_size );
			name[name_size] = '\0';
						
			index += name_size;
			char * temp_user_ip = new char[16];
			CString user_ip;
			GetIPStringFromIPInt(buf.GetInt(index), temp_user_ip);
			user_ip = temp_user_ip;
			delete [] temp_user_ip;
			index += 4;
			int user_port = buf.GetInt(index);
			index += 4;

			vector<SLSKtask *>::iterator iter = tasks.begin();

			for (int i = 0; i < (int)tasks.size(); i++, iter++)
			{
				if (name == tasks[i]->m_user_name)
				{
					tasks[i]->m_ip = user_ip;
					tasks[i]->m_port = user_port;

					if (abs(tasks[i]->m_port > 65535) || tasks[i]->m_port == 0)
					{
						int a = 3;
					}
					
					tasks[i]->m_state = 1;

				/*	SLSKtask *newtask1 = new SLSKtask();
					newtask1->m_artist = tasks[i]->m_artist;
					newtask1->m_album = tasks[i]->m_album;
					newtask1->m_ip = tasks[i]->m_ip;
					newtask1->m_port = tasks[i]->m_port;
					newtask1->m_token = tasks[i]->m_token;
					newtask1->m_user_name = tasks[i]->m_user_name;
					newtask1->m_state = 2;
					SLSKtask *newtask2 = new SLSKtask();
					newtask2->m_artist = tasks[i]->m_artist;
					newtask2->m_album = tasks[i]->m_album;
					newtask2->m_ip = tasks[i]->m_ip;
					newtask2->m_port = tasks[i]->m_port;
					newtask2->m_token = tasks[i]->m_token;
					newtask2->m_user_name = tasks[i]->m_user_name;
					newtask2->m_state = 3;
					SLSKtask *newtask3 = new SLSKtask();
					newtask3->m_artist = tasks[i]->m_artist;
					newtask3->m_album = tasks[i]->m_album;
					newtask3->m_ip = tasks[i]->m_ip;
					newtask3->m_port = tasks[i]->m_port;
					newtask3->m_token = tasks[i]->m_token;
					newtask3->m_user_name = tasks[i]->m_user_name;
					newtask3->m_state = 4;
					SLSKtask *newtask4 = new SLSKtask();
					newtask4->m_artist = tasks[i]->m_artist;
					newtask4->m_album = tasks[i]->m_album;
					newtask4->m_ip = tasks[i]->m_ip;
					newtask4->m_port = tasks[i]->m_port;
					newtask4->m_token = tasks[i]->m_token;
					newtask4->m_user_name = tasks[i]->m_user_name;
					newtask4->m_state = 5;*/

					p_sockets->SendTask(tasks[i]);
					//Sleep(1000);
					/*p_sockets->SendTask(newtask1);
					//Sleep(1000);
					p_sockets->SendTask(newtask2);
					//Sleep(1000);
					p_sockets->SendTask(newtask3);
					//Sleep(1000);
					p_sockets->SendTask(newtask4);*/
				//	delete tasks[i];
					tasks.erase(iter);
					break;
				}
			}
			
			delete [] name;
			buf.ResizeBuffer(0);
			break;
		}
	case 64:
	{
		buf.ResizeBuffer(0);
		break;
	}
	case 83:
		{
	//	buf.DropFront(4);
		buf.ResizeBuffer(0);

		break;
		}
	case 84:
		{
	//	buf.DropFront(4);
		buf.ResizeBuffer(0);
		break;
		}
	case 86:
		{
		//	buf.DropFront(4);
		buf.ResizeBuffer(0);
		break;
		}
	case 87:
		{
		//	buf.DropFront(4);
		buf.ResizeBuffer(0);
		break;
		}
	case 88:
		{
		//	buf.DropFront(4);
		buf.ResizeBuffer(0);
		break;
		}
	case 90:
		{
		//	buf.DropFront(4);
		buf.ResizeBuffer(0);
		break;
		}
	case 102:
		{
		
	   // CString parentname;
		buffer newbuffer;
		index += 4;
		
		p_sockets->Log("Obtaining Parent Ips from main server...");

		if (send73 == true)
		{
			newbuffer.PutInt(8);
			newbuffer.PutInt(73);
			newbuffer.PutInt(0);
			SendSocketData((unsigned char*)newbuffer.GetCharPtr(), (unsigned int)newbuffer.Size());
			send73 = false;
		}

		int limit = buf.GetInt(index); 
		char *parent_name, parent_ip[16];
		int parent_port, parent_name_size, temp_parent_ip;
		
	//	parent_name = new char[100];
		index += 4;
	//	db myDB;
	//	myDB.dbConnection("38.119.66.38", "root", "sumyungguy37", "slsk");



		for (int i = 0; i < limit; i++)
		{
			parent_name_size = buf.GetInt(index);
			index += 4;
			parent_name = new char[parent_name_size + 1];
			memcpy( parent_name, buf.GetCharPtr(index), parent_name_size );
			parent_name[parent_name_size] = '\0';
			
			index += parent_name_size;
			temp_parent_ip = buf.GetInt(index);
			GetIPStringFromIPInt(temp_parent_ip, parent_ip);
            index += 4;
			parent_port = buf.GetInt(index);
			index += 4;
		//	CString query = "Insert into parent_ips values('";
			//query.Append(s);
			//query.Append("', '");
		//	query.Append(parent_ip);
		//	query.Append("', '");
		//	query.AppendFormat("%d", parent_port);
		//	query.Append("');");
		//	myDB.dbOQuery((char*)(LPCTSTR)query);
		//	p_sockets->Log((const char*)(LPCTSTR)query);
			SupernodeHost nh;
			nh.SetIP(parent_ip);
			nh.setport(parent_port);
			p_sockets->AddParent(nh);
			delete [] parent_name;
			
		}
	//	myDB.dbClose();
		p_sockets->Log("Completed getting parent ips from main server!");
		//m_last_parent_gotten = CTime::GetCurrentTime();
		sendparent = true;
		//m_time_last_parent_sent = CTime::GetCurrentTime();
		buf.ResizeBuffer(0);
		break;
		}
	case 104:
		{
		buf.ResizeBuffer(0);
		
		break;
		}
	default:
		{
			
		if (buf.GetByte(index) != 3)
			break;
		///else 
		
		
		char *tempstring;

		if (m_first_query == true)
		{
			//db myDB;
			//myDB.dbConnection("localhost", "onsystems", "sumyungguy37", "slsk");
		//	if (m_parent_connecting > 0)
		//	m_parent_connecting--;
			//tempstring = new char[10];
			//sprintf(tempstring, "%d", m_parent_connecting);
			//p_sockets->setParentConnecting(tempstring);
		//	m_parent_connections++;
			//tempstring = new char[10];
			//sprintf(tempstring, "%d", m_parent_connections);
		//	tempstring.PutInt(m_parent_connections);
		//	CString stemp;
		//	stemp.AppendFormat("%d", m_parent_connections);
		//	char* tempstring = stemp;
			//p_sockets->ParentStatus(tempstring);
		//	CString q = "Insert into parent_ips values('";
		//	q.Append(connection_ip);
		//	q.Append("', '");
		//	q.AppendFormat("%d", port);
		//	q.Append("');");
		//	myDB.dbOQuery((char*)(LPCTSTR)q);
		//	myDB.dbClose();
		//	m_state_connecting = 2;
		    m_first_query = false;
			//myDB.dbClose();
		}

		
		index += 1;

	//	break;  //breaks out
		//db myDB;
	//	myDB.dbConnection("38.119.66.38", "root", "sumyungguy37", "slsk");


		int integer = buf.GetInt(index);
		index += 4;
		int peer_name_size = buf.GetInt(index);
		if (peer_name_size > 256)
		{
			int a = 0;
		}
		index += 4;
		
		char *peer_name = new char[peer_name_size + 1];
		memcpy( peer_name, buf.GetCharPtr(index), peer_name_size );
		peer_name[peer_name_size] = '\0';
		
		//delete [] peer_name;
		//return;

		index += peer_name_size;
		int token = buf.GetInt(index);
		index += 4;
		int filename_size = buf.GetInt(index);
		index += 4;
		char *filename = new char[filename_size + 1];
		memcpy( filename, buf.GetCharPtr(index), filename_size );
		filename[filename_size] = '\0';
		index += filename_size;
		char *filenameholder = new char[filename_size + 1];
		strcpy(filenameholder, filename);


	
		if (strcmp(filename, "50 cent") == 0)
		{
			int alsfjlad = 0;
		}
        
		int weight = 0;
		char seps[] = " _-,.";
		char* tok;
		char* searchname;
		char* tokenname;
		ProjectKeywords *keywords;

		SLSKtask *task;


	/*	if ((int)p_sockets->weighted_keywords.size() == 0)
		{
			//task = new SLSKtask();
			//delete task;
			return;
		}*/
//break;
	/*	if (strcmp(filename, "salt water enema") == 0)
		{
			task = new SLSKtask();
			task->m_artist = "The game";
			task->m_album = "THE DOCUMENTARY";
		}
		else
		{
			delete [] peer_name;
			delete [] filename;
			return;
		}*/

 		for (int i = 0; i < (int)p_sockets->m_projects->v_projects.size(); i++)
		{
			keywords = &p_sockets->m_projects->v_projects[i];
//lower in vector and remove here
			for (int j = 0; j < (int)keywords->m_query_keywords.v_exact_keywords.size(); j++)
			{
				strcpy(filename, filenameholder);
				searchname = _strlwr( (char*)keywords->m_query_keywords.v_exact_keywords[j].keyword.c_str()  );
				tokenname = _strlwr(  filename );
			
				tok = strtok((char*)(LPCTSTR)tokenname, seps);

				while( tok != NULL)
				{
					if (strcmp(searchname, tok) == 0)
					{
						weight += keywords->m_query_keywords.v_exact_keywords[j].weight;

						if (weight >= 100)
						{
							//if(keywords->m_soulseek_demand_enabled)
							//{
							//	IncrementDemandCounter((char*)keywords->m_artist_name.c_str());

							//	if (keywords->m_soulseek_spoofing_enabled == false)
							//	{
							///		delete [] peer_name;
							//		delete [] filename;
									//	task = new SLSKtask();
									//	delete task;
							//		return;
							//	}
							//}
						
							task = new SLSKtask();
						
							task->m_artist = keywords->m_artist_name.c_str();
							task->m_album = keywords->m_album_name.c_str();
						//	task->m_project_id = keywords->;
							//delete task;
							//break;
						}
						break;
					}
		
					tok = strtok( NULL, seps );
				}
		
				if (weight >= 100)
				break;
			}
		}
        
		if (weight < 100)
		{
			delete [] peer_name;
			delete [] filename;
			delete [] filenameholder;
		//	task = new SLSKtask();
		//	delete task;
			return;
		}

		/*TrackInformation *ti;
	//	delete task;
    	//return;

		for (i = 0; i < (int)p_sockets->m_track_info->size(); i++)
		{
			ti = p_sockets->m_track_info[i];

			if (task->m_artist == ti->)
			{
				task->m_album = p_sockets->projects[i].m_album;
				task->m_artist = p_sockets->projects[i].m_artist;

				for (int j = 0; j < (int)p_sockets->projects[i].tracks.size(); j++)
				{
			//		if (strstr((const char*)(LPCTSTR)filename, (const char*)(LPCTSTR)p_sockets->projects[i].tracks[j].m_track_name) != NULL)
			//		{
						track t;
						t.m_track_name = p_sockets->projects[i].tracks[j].m_track_name;
						t.m_track_number = p_sockets->projects[i].tracks[j].m_track_number;

						task->tracks.push_back(t);
				}
			//	}

				break;
			}
		}*/

	//	if (strcmp(filename, "selt weter eneme") == 0)
	//	{
			p_sockets->Log("Got A Hit!!!");
	//		SLSKtask *task = new SLSKtask();
			task->m_user_name = peer_name;
	//		task->m_project_id = 999;
			task->m_token = token;
		//	delete task;

			delete [] peer_name;
			delete [] filename;
			delete [] filenameholder;
		
			//delete task;

			p_sockets->SendTask(task);
			
//			CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText("got filename!\n");
				//make tasks list
			//CString query = "Insert into raw_data(username, file) values('";
			//query.Append(peer_name);
			//query.Append("', '");
			//query.Append(filename);
			//	query.Append("', '");
			//	query.Append(filename);// supposed to be a time stamp
		//	query.Append("');");
		//	myDB.dbOQuery((char*)(LPCTSTR)query);

			

	//		CString ns = zipped;
	//		ns = ns.Left(zippedsize);
               
			
			//newtask->setUN(peer_name);
//			newtask->setBuf(sendbuffer);
		//	CSLSKSpooferDlg::GetInstance()->todo.connectingtopeer.push_back(newtask);
//
	//		CSLSKSpooferDlg::GetInstance()->todo.peerInfo.push_back(peer_name);
			//	CSLSKSpooferDlg::GetInstance()->todo.ctp = true;
		//	CSLSKSpooferDlg::GetInstance()->todo.tasksforserver = true;
                
	//	}
	//	else
	//	{
	//		CString query = "Insert into spoof_names values('" + peer_name + "')";
	//		myDB.dbOQuery((char*)(LPCTSTR)query);
	//	}

		
		break;
	}

		
	}
}

//
//
//





void SLSKSocket::OnClose(int error_code)
{


/*	if (p_sockets->state == 2)
	{
		SupernodeHost *nh = new SupernodeHost();
		nh->SetIP(connection_ip);
		nh->setport(port);
		p_sockets->DropCachedParent(nh);
	//	if (m_connected_parent)
	//	{
			
		//}
	}*/

	if (p_sockets->state == 3)
	{
		/*m_peer_connections--;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_peer_connections);
	//	tempstring.PutInt(m_peer_connections);
	//	CString temp;
	//	temp.AppendFormat("%d", m_peer_connections);
	//	char* tempstring = temp;
		p_sockets->PeerStatus(tempstring);*/

		for (int i = 0; i < (int)spoofs.size(); i++)
		{
			SendSocketData((unsigned char*)spoofs[i]->GetCharPtr(), (int)spoofs[i]->Size());
			delete spoofs[i];
		}
	}
//	CString s;
//	CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
//	s.Append("Connection closed for this ip ");
//	s.Append(connection_ip);
	//s.Append("\n");
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);
	Close(error_code);
	/*
	char log[128];
	sprintf(log,"Socket closed: %d",error_code);
	p_sockets->Log(log);
	*/
//	m_state = SessDisconnected;
/*	if (CSLSKSpooferDlg::GetInstance()->todo.connections.size() > 0)
		{
			vector<CString>::iterator iter = CSLSKSpooferDlg::GetInstance()->todo.connections.begin();

			for (int i = 0; i < CSLSKSpooferDlg::GetInstance()->todo.connections.size();i++, iter++)
			{
				if (strcmp(connection_ip, CSLSKSpooferDlg::GetInstance()->todo.connections[i]) == 0)
				{
					//numofconnections--;
					CSLSKSpooferDlg::GetInstance()->todo.connections.erase(iter);
				}
			}
		}*/

//		CString d;
//		d.AppendFormat("%d", numofconnections);
//		CSLSKSpooferDlg::GetInstance()->noc.SetWindowText(d);
	TEventSocket::OnClose(error_code);
}

//
//
//
/*
void SLSKSocket::SocketDataSent(unsigned int len)
{
	//TRACE("SocketDataSent\n");
	/*
	// Check to see if we've got some OnReceive(s) cached.
	if(m_on_receive_cached)
	{
		// Check to see if we have sent out enough buffers
		if(v_send_data_buffers.size()<MAX_SEND_BUFFERS_ALLOWED)
		{
			OnReceive(0);
			return;
		}
	}
	/
}
*/
//
//
//
/*void SLSKSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	//TRACE("SomeSocketDataReceived - data_len: %d\n",data_len);

	if(m_state == SessWaitingNetName)
	{
		GetRemoteServerName((unsigned char*)data, data_len);
		if(m_state == SessEstablished)
		{
			SessionGreetSupernode();
			return;
		}
	}
	if(m_state == SessEstablished && !m_received_header)
	{
		PacketBuffer buf;
		buf.PutStr(data,data_len);
		//decrypt msg
		fst_cipher_crypt(p_in_cipher, (unsigned char*)buf.p_data,buf.Size());

		if(data_len <= 5) //msg header
		{
			//decode msg
			byte type = buf.GetByte();

			switch(type)
			{
				/* we got ping /
				case 0x50:
				{
					//p_sockets->Log("Got Ping");
					p_sockets->m_status_data.m_ping_count++;
					SendPong();
					return;
				}
				/* we got pong /
				case 0x52:
				{
					p_sockets->m_status_data.m_pong_count++;
					//p_sockets->Log("Got Pong");
					return;
				}
				/* we got a message /
				case 0x4B:
				{
					//p_sockets->Log("Got Message");
					int xtype = m_in_xinu % 3;
					unsigned short msg_type = 0;
					unsigned short msg_len = 0;

					if(data_len < 5)
					{
						//p_sockets->Log("Didn't get the message type and length, closing socket");
						Close();
						return;
					}
					
					switch (xtype)
					{
						case 0:
						{
							msg_type = buf.GetByte();
							msg_type |= buf.GetByte() << 8;
							msg_len = buf.GetByte() << 8;
							msg_len |= buf.GetByte();
							break;
						}
						case 1:
						{
							msg_type = buf.GetByte() << 8; 
							msg_len = buf.GetByte() << 8;
							msg_type |= buf.GetByte();
							msg_len |= buf.GetByte();
							break;
						}
						case 2:
						{
							msg_type = buf.GetByte() << 8;
							msg_len = buf.GetByte();
							msg_len |= buf.GetByte() << 8;
							msg_type |= buf.GetByte();
							break;
						}
					}
					/*
					char log[128];
					sprintf(log,"got message type = %02x, length = %d byte",msg_type, msg_len);
					p_sockets->Log(log);
					/
					m_message_type = msg_type;
					m_message_length = msg_len;
					/* modify xinu state /
					m_in_xinu ^= ~(msg_type + msg_len);
					if(m_message_length > 0)
					{
						m_received_header = true;
						if(m_message_length <= MAX_MSG_LENGTH)
							ReceiveSocketData(m_message_length);
						else
						{
							char msg[128];
							sprintf(msg, "Message length is too big. Message type: %02x, length = %d byte",m_message_type,m_message_length);
							p_sockets->Log(msg);
							Close();
						}
					}
					else
						m_received_header = false;

					/*
					if (data_len - ptr_count < msg_len)
					{
						p_sockets->Log("Didn't get the whole message, waiting for more data");
						// get more data
						ContinueToReceiveSocketData();
						return;
					}
					

					// prepare packet for callback
					packet = fst_packet_create_copy (session->in_packet, msg_len);
					fst_packet_truncate (session->in_packet);
			
					// raise callback
					if (!session->callback (session, msg_type, packet))
					{
						// session was probably already freed by the handler.
						// so don't access it
						//
						fst_packet_free (packet);
						return;
					}
					
					ptr_buf += msg_len;
					ptr_count += msg_len;
					/
					break;
				}
			}
		}
		else
		{
			char log[128];
			sprintf(log,"Received unknown message length. Expected 5 bytes, received %d bytes", data_len);
			p_sockets->Log(log);
			return;
		}
		return;
	}
}

//
//
//
void SLSKSocket::SocketDataReceived(char *data,unsigned int len)
{
	//TRACE("SocketDataReceived - data length: %d\n",len);

	if(m_state == SessHandshaking)
	{
		GetIncomingSeedAndEncryptionType((unsigned char*)data, len);
		return;
	}

	if(m_state == SessEstablished && m_received_header)
	{
		if( len == m_message_length) //received the whole message data
		{
			PacketBuffer buf;
			buf.PutStr(data,len);
			//decrypt msg
			fst_cipher_crypt(p_in_cipher, buf.p_data,buf.Size());

			/* doing something with the message data here /
			switch(m_message_type)
			{
				/*
				char msg[128];
				sprintf(msg,"Received message data. Length - %d bytes",len);
				p_sockets->Log(msg);
				/
				case SessMsgNodeList: //supernode list <-
				{
					vector<SupernodeHost> hosts;
					while(buf.Remaining() >= 8)
					{
						SupernodeHost host;
						host.m_ip = buf.GetUInt();
						host.m_port = ntohs(buf.GetUShort());
						byte last_seen = buf.GetByte();
						byte load = buf.GetByte();
						hosts.push_back(host);

						// Store the ip for when we need to send spoofs...so that we can randomize the last two digits
						//p_sockets->SpoofIP(host.m_ip);
					}
					p_sockets->ReportHosts(hosts);
					break;
				}
				case SessMsgUserInfo: //user info ->
				{
					break;
				}
				case SessMsgUnshareFile: //unshare file ->
				{
					break;
				}
				case SessMsgQuery: //search query ->
				{
					break;
				}
				case SessMsgQueryReply: //search reply <-
				case SessMsgQueryEnd: //search finished <-
				{
					ReceivedQueryReply(buf,(FSTSessionMsg)m_message_type);
					p_sockets->m_status_data.m_query_hit_count++;
					break;
				}
				case SessMsgNetworkStats: //network status <-
				{
					unsigned int mantissa, exponent;
					UINT users, files, size;

					if (len < 12)
					{
						TRACE("Didn't received 12 bytes of network status data. Received %d bytes only\n",len);
						break;
					}
					
					users = ntohl(buf.GetUInt());
					files = ntohl(buf.GetUInt());

					mantissa = ntohs(buf.GetUShort());	/* mantissa of size /
					exponent = ntohs(buf.GetUShort());	/* exponent of size /

    				if (exponent >= 30)
						size = mantissa << (exponent - 30);
    				else
						size = mantissa >> (30 - exponent);

					/* what follows in the packet is the number of files and their size
					* per media type (6 times).
					* Then optionally the different network names and the number of
					* their users.
					* we do not currently care for those
					*/
					
					/*
					char msg[128];
					sprintf(msg, "Received network stats: %d users, %d files, %d GB",users,files,size);
					p_sockets->Log(msg);
				/
					
					
					// if we connected to a splitted network segment move on
					if (users < MIN_USERS_ON_CONNECT)
					{
						/*
						char msg[256];
						sprintf(msg,"Disconnecting from a splitted network segment which only has %u users.",users);
						p_sockets->Log(msg);
						/
						Close();
					}
					
					break;
				}
				case SessMsgNetworkName: //Network name <-,->
				{
					char* remote_server_name = buf.GetStr(len);
					/*
					char msg[128];
					sprintf(msg, "Remote network name is %s", remote_server_name);
					p_sockets->Log(msg);
					/
					if(remote_server_name != NULL)
					{
						m_status_data.m_remote_vendor = remote_server_name;
						delete [] remote_server_name;
					}
					else
					{
						p_sockets->Log("Remote server name is bigger than 1024 bytes, closing socket");
						Close();
						break;
					}
					
					PacketBuffer out_buf;
					out_buf.PutStr(FST_NETWORK_NAME,strlen(FST_NETWORK_NAME));
					SendSessionMessage(out_buf,SessMsgNetworkName);
					break;
				}
				case SessMsgProtocolVersion: // protocol version ->,<-
				{
					UINT version;
					version = ntohl(buf.GetUInt());
					/*
					char msg[128];
					sprintf(msg,"Received protocol version: 0x%02X", version);
					p_sockets->Log(msg);
					/
					buf.Rewind();									
					SendSessionMessage(buf,SessMsgProtocolVersion);
					m_ready_to_send_shares = true;
					m_status_data.m_up_time=CTime::GetCurrentTime();	// we are now "up"
					break;
				}
				case SessMsgPushRequest:
				{
					p_sockets->m_status_data.m_push_count++;
					break;
				}
				case SessMsgExternalIp:
				{
					break;
				}
			}

			m_received_header = false;

		}
		else //message length mismatch
		{
			char msg[128];
			sprintf(msg,"Message length mismatch. Expected %u bytes, received %u bytes", m_message_length, len);
			p_sockets->Log(msg);
			Close();
			return;
		}
	}
}

//
//
//
unsigned int SLSKSocket::GetEncryptionType(unsigned int seed, unsigned int enc_type)
{
	unsigned int key_80[20];
	int i;

	for (i = 0; i < 20; i++)
	{
		seed = 0x10dcd * seed + 0x4271;
		key_80[i] = seed;
	}

	seed = 0x10dcd * seed + 0x4271;
	EncryptionType2::enc_type_2(key_80, seed);

	return enc_type ^ key_80[7];
}

//
//
//
bool SLSKSocket::GetIncomingSeedAndEncryptionType(unsigned char* data, int len)
{
	int encoded_encryption_type = 0;
	if(len >= 8)
	{
		if(!DoHandshake(&data[0]))
		{
			p_sockets->Log("Handshake fail, closing connection");
			Close();
			return false;
		}
		//fst_cipher_crypt(p_in_cipher, &data[0],8);
		m_state = SessWaitingNetName;
		return true;
	}
	else
	{
		p_sockets->Log("Received insufficient data for calculating key, waiting for more...");
		//ReceiveSomeSocketData(4096);
		return false;
	}

}

//
//
//
int SLSKSocket::DoHandshake (unsigned char * data)
{
	unsigned int seed, enc_type;

	/* get seed and enc_type /
	seed = ntohl( (*(unsigned int*)data) );
	enc_type = ntohl( (*(unsigned int*)&data[4]) );
	enc_type = fst_cipher_mangle_enc_type(seed, enc_type);

	/* generate send key /
	p_out_cipher->seed ^= seed; /* xor send cipher with received seed */

	/* the correct behaviour here is to use the enc_type the supernode sent
	 * us for out_cipher too.
	 * thanks to HEx and weinholt for figuring this out.
	 /
	p_out_cipher->enc_type = enc_type;

	if(!fst_cipher_init(p_out_cipher, p_out_cipher->seed,
						 p_out_cipher->enc_type))
	{
		char msg[128];
		sprintf(msg,"Unsupported encryption: 0x%02X",p_out_cipher->enc_type);
		p_sockets->Log(msg);
		return FALSE;
	}

	/* generate recv key /
	if(!fst_cipher_init (p_in_cipher, seed, enc_type))
	{
		char msg[128];
		sprintf(msg,"Unsupported encryption: 0x%02X",enc_type);
		p_sockets->Log(msg);
		return FALSE;
	}

	//TRACE("outgoing enc_type: 0x%02X, incoming enc_type: 0x%02X\n",p_out_cipher->enc_type, enc_type);


	// send network name
	//SendMyNetworkName();


	return TRUE;
}

//
//
//
void SLSKSocket::GetRemoteServerName(unsigned char* data, UINT data_len)
{
	fst_cipher_crypt(p_in_cipher,data,data_len);
	m_state = SessEstablished;
	/*
	char msg[128];
	sprintf(msg, "Session established. Remote network name is %s",(char*)data);
	p_sockets->Log(msg);
	/
	SendMyNetworkName();
}

//
//
//
void SLSKSocket::SendMyNetworkName(void)
{
	/*
	char msg[128];
	sprintf(msg,"Send my network name: %s",FST_NETWORK_NAME);
	p_sockets->Log(msg);
	/

	UINT buf_len = (UINT)strlen(FST_NETWORK_NAME)+1;
	char* buf = new char[buf_len];
	strcpy(buf,FST_NETWORK_NAME);
	fst_cipher_crypt(p_out_cipher, (unsigned char*)buf, buf_len);
	
	SendSocketData((unsigned char*)buf,buf_len);
	delete [] buf;
}

//
//
//
void SLSKSocket::SessionGreetSupernode()
{
	//p_sockets->Log("Sending ip, bandwidth and user name to supernode");

	/* Send our ip address and port.
	 * Should we send our outside ip if NATed? We don't have it at this
	 * point from the supernode.
	 /

	PacketBuffer buf;

	UINT myIP; //
//	short port; //

	sockaddr_in addr;
	ZeroMemory(&addr,sizeof(sockaddr_in));
	int addr_size = sizeof(addr);
	getsockname(m_hSocket,(sockaddr*)&addr,&addr_size);

	myIP = addr.sin_addr.S_un.S_addr;

	buf.PutUInt(myIP);
	//buf.PutUInt(p_sockets->SpoofIP());
	//buf.PutUShort(addr.sin_port);
#ifdef _DEBUG
	buf.PutUShort(htons(1214));
#else
	buf.PutUShort(htons(2597));
#endif

	// This next byte represents the user's advertised bandwidth, on
 	// a logarithmic scale.  0xd1 represents "infinity" (actually,
	// 1680 kbps).  The value is approximately 14*log_2(x)+59, where
	// x is the bandwidth in kbps.
	byte rand_bandwidth = (rand()%50) + 160;
	buf.PutByte(rand_bandwidth);


	buf.PutByte(0x00); //unknown

	// user name, no trailing '\0'
	//buf.PutStr("poopy55",strlen("poopy55"));
	

	CString username;
	p_sockets->GetRandomUserName(username);
	buf.PutStr(username,username.GetLength());

	SendSessionMessage(buf,SessMsgUserInfo); //send user info
	SendPing();
}

//
//
//*/
unsigned int SLSKSocket::GetIntIPFromStringIP(const char* ip)
{
	unsigned int ip_int = 0;
	if(strlen(ip)==0)
		return ip_int;
	
	int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	return ip_int;
}

//
//
//
void SLSKSocket::GetIPStringFromIPInt(int ip_int, char* ip_str)
{
//	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>24)&0xFF,(ip_int>>16)&0xFF,(ip_int>>8)&0xFF,(ip_int>>0)&0xFF);
}

//
//
//
/*
void SLSKSocket::SendSessionMessage(PacketBuffer& buf, FSTSessionMsg msg_type)
{
	if (m_state != SessEstablished)
		return;

	PacketBuffer new_buf;
	//unsigned char* out_buf = new unsigned char[buf.m_used + 5];
    
	unsigned char hi_len, lo_len;
    unsigned char hi_type, lo_type;
    int xtype;

	//Assert(msg_type < 0xFF);
	//Assert(msg_data != NULL);

	/*
	char log[128];
	sprintf(log,"sending msg with msg_type: 0x%02X", msg_type);
	p_sockets->Log(log);
	/

	lo_len = buf.Size() & 0xff;
	hi_len = buf.Size() >> 8;
	
	new_buf.PutByte(0x4B); //packet type 'K'

	lo_type = msg_type & 0xFF;
	hi_type = msg_type >> 8;

    xtype = m_out_xinu % 3;

    switch(xtype) {
	case 0:
		new_buf.PutByte(lo_type);
		new_buf.PutByte(hi_type);
		new_buf.PutByte(hi_len);
		new_buf.PutByte(lo_len);
	    break;
	case 1:
		new_buf.PutByte(hi_type);
		new_buf.PutByte(hi_len);
		new_buf.PutByte(lo_type);
		new_buf.PutByte(lo_len);
	    break;
	case 2:
		new_buf.PutByte(hi_type);
		new_buf.PutByte(lo_len);
		new_buf.PutByte(hi_len);
		new_buf.PutByte(lo_type);
	    break;
    }

	/* update xinu state /
	m_out_xinu ^= ~(buf.Size() + msg_type);
	new_buf.Append(buf);
	fst_cipher_crypt(p_out_cipher,new_buf.p_data,new_buf.Size());
	SendSocketData(new_buf.p_data,new_buf.Size());
}

//
//
//
/* send out pong response /
void SLSKSocket::SendPong()
{
	unsigned char buf = 0x52;
	fst_cipher_crypt(p_out_cipher,&buf,1);
	SendSocketData(&buf,1);
	//p_sockets->Log("Pong sent");
}

//
//
//
/* send out ping request /
void SLSKSocket::SendPing()
{
	m_time_last_ping_sent=CTime::GetCurrentTime();
	unsigned char buf = 0x50;
	fst_cipher_crypt(p_out_cipher,&buf,1);	
	SendSocketData(&buf,1);
	//p_sockets->Log("Ping sent");
}

//
//
//
void SLSKSocket::SendShare(void)
{
/*	char* filename="mdgifttest.txt";
	char* keyword="mdgifttest";
	PacketBuffer buf;

	buf.PutByte(0x00); //unknown
	buf.PutByte(MEDIA_TYPE_DOCUMENT); //media type
	buf.PutStr("\x00\x00",2);
	buf.PutStr("K23J7DH28FO49SNVU3HS",20); //hash
	
	buf.PutDynInt(GetHashChecksum("K23J7DH28FO49SNVU3HS")); //file_id
	buf.PutDynInt(3425); //file size
	buf.PutDynInt(2);	//number of tag
	
	buf.PutDynInt(FILE_TAG_FILENAME);	//tag type
	buf.PutDynInt((UINT)strlen(filename));	//tag length
	buf.PutStr(filename,strlen(filename)); //tag content
	
	buf.PutDynInt(FILE_TAG_KEYWORDS);	//tag type
	buf.PutDynInt((UINT)strlen(keyword));	//tag length
	buf.PutStr(keyword,strlen(keyword)); //tag content
/

#ifdef SHARING_ENABLED
	/*
	vector<PacketBuffer*>::iterator iter = p_sockets->v_shared_files_buffers.begin();
	while(iter != p_sockets->v_shared_files_buffers.end())
	{
		SendSessionMessage(*(*iter),SessMsgShareFile);
		//delete *iter;
		//v_shared_files_buffers.erase(iter);
	}
	
	for(UINT i=0; i<p_sockets->v_shared_files_buffers.size(); i++)
	{
		SendSessionMessage(*(p_sockets->v_shared_files_buffers[i]),SessMsgShareFile);
	}
	/
	PacketBuffer* buf = p_sockets->p_file_sharing_manager->GetShare();
	if(buf != NULL)
	{
		SendSessionMessage(*buf,SessMsgShareFile);
		delete buf;
		m_num_shares_sent++;
	}
#endif
	
}

//
//
//
void SLSKSocket::Search(ProjectSupplyQuery& psq)
{
	if(m_state == SessEstablished)
	{
		PacketBuffer buf;
		
		buf.PutUStr((unsigned char*)"\x00\x01",2); //unknown
		buf.PutUShort(htons(FST_MAX_SEARCH_RESULTS)); //max search result
		buf.PutUShort(htons(psq.m_search_id)); //search id
		buf.PutByte(0x01); //unknown
		switch(psq.m_search_type)
		{
			case ProjectKeywords::search_type::audio:
			{
				buf.PutByte(QUERY_REALM_AUDIO); //search realm
				break;
			}
			case ProjectKeywords::search_type::video:
			{
				buf.PutByte(QUERY_REALM_VIDEO); //search realm
				break;
			}
			case ProjectKeywords::search_type::software:
			{
				buf.PutByte(QUERY_REALM_SOFTWARE); //search realm
				break;
			}
			default:
			{
				buf.PutByte(QUERY_REALM_EVERYTHING); //search realm
				break;
			}

		}
		buf.PutByte(0x01); //number of search terms

		//for each search term
		buf.PutByte(QUERY_CMP_SUBSTRING); //comparison type
		buf.PutByte(FILE_TAG_ANY);		 //comparison field tag type
		buf.PutDynInt((UINT)psq.m_search_string.length()); //query length
		buf.PutStr(psq.m_search_string.c_str(),(UINT)psq.m_search_string.length());
		SendSessionMessage(buf,SessMsgQuery);
	}
}

//
//
//
void SLSKSocket::ReceivedQueryReply(PacketBuffer& buf, FSTSessionMsg msg_type)
{
	if (msg_type == SessMsgQueryEnd)
	{
		unsigned short search_id = ntohs( buf.GetUShort());
		/*
		char msg[256];
		sprintf(msg,"Received end of search for search id = %d",search_id);//, %d replies, %d firewalled, %d banned",fst_id, search->replies, search->fw_replies, search->banlist_replies);
		p_sockets->Log(msg);
		/
		return;
	}
	else if (msg_type == SessMsgQueryReply)
	{
		/* supernode ip an port /
		unsigned int sip = buf.GetUInt();
		unsigned short sport = ntohs(buf.GetUShort());

		/* get query id and look up search /
		unsigned short search_id = ntohs(buf.GetUShort());

		/* get number of results /
		unsigned short nresults = ntohs(buf.GetUShort());
		int i,ntags;

		// Find the project supply query that is related to this search ID
		//ProjectSupplyQuery *psq=NULL;
		ProjectSupplyQuery find_psq;
		find_psq.m_search_id=search_id;
		hash_set<ProjectSupplyQuery>::iterator psq=NULL;
		psq=p_sockets->hs_project_supply_queries.find(find_psq);
		/*
		for(i=0;i<(int)p_sockets->v_project_supply_queries.size();i++)
		{
			if(p_sockets->v_project_supply_queries[i].m_search_id == search_id)
			{
				psq=&p_sockets->v_project_supply_queries[i];
				break;
			}
		}
		/

		// If it is still equal to NULL, then return
		if(psq== p_sockets->hs_project_supply_queries.end() || psq==NULL)
		{
			TRACE("Couldn't find search ID for this query result\n");
			return;
		}

		// Find the host connection status data for this project
		ProjectStatus *status=&p_sockets->m_status_data.v_project_status[psq->m_project_status_index];
		vector<QueryHit> *query_hits=&p_sockets->m_status_data.v_project_status[psq->m_project_status_index].v_query_hits;


		for(;nresults && buf.Remaining() >= 32; nresults--)
		{
			unsigned int ip;
			unsigned short port;
			CString username;
			CString netname;
			CString filename;
			CString title;
			//CString description;
			CString artist;
			unsigned int filesize;
			unsigned int file_id;
			unsigned char hash[FST_FTHASH_LEN];
			byte bandwidth;

			ip = buf.GetUInt();
			port = ntohs(buf.GetUShort());
			bandwidth = buf.GetByte();

			/* user and network name /
			if(*(byte*)(buf.p_read_ptr) == 0x02)
			{
				/* compressed, look up names based on ip and port /
				buf.p_read_ptr++;

				/*
				// start with results->next because results is us 
				for (item=results->next; item; item = item->next)
				{
					FSTSearchResult *res = (FSTSearchResult*) item->data;
					if (res->ip == result->ip && res->port == result->port)
					{
						result->username = strdup (res->username);
						result->netname = strdup (res->netname);
						FST_HEAVY_DBG_2 ("decompressed %s@%s",
										result->username, result->netname);
						break;
					}
				}

				if (!result->username)
					result->username = strdup("<unknown>");
				if (!result->netname)
					result->netname = strdup("<unknown>");
				/
			}
			else
			{
				/* user name /
				
				if((i = buf.Strlen(0x01)) < 0)
				{
					return;
				}
				
				char* pUser = buf.GetStr(i+1);
				if(pUser != NULL)
				{
					pUser[i] = 0;
					username = pUser;
					delete [] pUser;
				}
				else
				{
					p_sockets->Log("Username length is bigger than 1024 bytes");
					return;
				}


				/* network name /
				if((i = buf.Strlen(0x00)) < 0)
				{
					return;
				}
				char* pNetname = buf.GetStr(i+1);
				if(pNetname != NULL)
				{
					pNetname[i] = 0;
					netname = pNetname;
					delete [] pNetname;
				}
				else
				{
					p_sockets->Log("pNetname length is bigger than 1024 bytes");
					return;
				}
			}

			if (buf.Remaining() >= FST_FTHASH_LEN)
			{
				unsigned char* pHash = buf.GetUStr(FST_FTHASH_LEN);
				if(pHash != NULL)
				{
					memcpy(hash,pHash,FST_FTHASH_LEN);
					delete [] pHash;
				}
				else //packet screwed up
				{
					p_sockets->Log("Hash size is bigger then 1024 bytes");
					return;
				}
			}


			file_id = buf.GetDynInt();
			filesize = buf.GetDynInt();
			ntags = buf.GetDynInt();
			/*
			char log[1024];
			char ip_str[16];
			GetIPStringFromIPInt(ip,ip_str);
			sprintf(log,"Result %d: %s  address: %s:%d  name: %s@%s  filesize: %d, ntags: %d", nresults, HashEncode64(hash),ip_str,port,username,netname,filesize,ntags);
			p_sockets->Log(log);
			/

			
			//* read tags 
			for(;ntags && buf.Remaining() >= 2; ntags--)
			{
				int	tag = buf.GetDynInt();
				int taglen = buf.GetDynInt();

				/* Large tags are a sure sign of broken packets.
				* These packets have random junk inserted or miss some bytes while
				* the surrounding packet data is ok. For example there was an
				* artist tag with a correct length of 0x08 but the following
				* string was "Cldplay" shifting the entire packet by one byte.
				* This is _not_ a problem on our end! Looks like a memory
				* corruption issue on the sender side. /
				
		
				if (tag > 0x40)
				{
					//TRACE("Bad result detected (tag > 0x40)\n");
					return;
				}
				switch(tag)
				{
					case FILE_TAG_RESOLUTION:
					{
						int resol1 = buf.GetDynInt();
						int resol2 = buf.GetDynInt();
						break;
					}
					case FILE_TAG_HASH:
					{
						if(taglen > MAX_STRING_SIZE)
						{
							/*
							char msg[256];
							sprintf(msg,"Tag length is bigger than %d bytes. Tag: 0x%02x, len: %02d",MAX_STRING_SIZE,tag,taglen);
							p_sockets->Log(msg);
							/
							return;
						}
		
						byte* hash = buf.GetUStr(taglen);
						if( hash != NULL)
						{
							delete [] hash;
						}
						else
						{
							/*
							char msg[256];
							sprintf(msg,"Remaining packet length is smaller than tag length. Remains: %u bytes, Tag: 0x%02x, Tag length: %d bytes",buf.Remaining(),tag,taglen);
							p_sockets->Log(msg);
							/
						}
						break;
					}
					case FILE_TAG_YEAR:
					case FILE_TAG_TIME:
					case FILE_TAG_BITDEPTH:
					case FILE_TAG_QUALITY:
					case FILE_TAG_UNKNOWN1:
					case FILE_TAG_RATING:
					case FILE_TAG_SIZE:
					{
						int integer = buf.GetDynInt();
						break;
					}
					case FILE_TAG_FILENAME:
					{
						if(taglen > MAX_STRING_SIZE)
						{
							/*
							char msg[256];
							sprintf(msg,"Tag length is bigger than %d bytes. Tag: 0x%02x, len: %02d",MAX_STRING_SIZE,tag,taglen);
							p_sockets->Log(msg);
							/
							return;
						}
						char* string_data = buf.GetStr(taglen);
						if( string_data != NULL)
						{
							filename = string_data;
							delete [] string_data;
						}
						else
						{
							/*
							char msg[256];
							sprintf(msg,"Remaining packet length is smaller than tag length. Remains: %u bytes, Tag: 0x%02x, Tag length: %d bytes",buf.Remaining(),tag,taglen);
							p_sockets->Log(msg);
							/
							return;
						}						
						break;
					}
					case FILE_TAG_TITLE:
					{
						if(taglen > MAX_STRING_SIZE)
						{
							/*
							char msg[256];
							sprintf(msg,"Tag length is bigger than %d bytes. Tag: 0x%02x, len: %02d",MAX_STRING_SIZE,tag,taglen);
							p_sockets->Log(msg);
							/
							return;
						}
						char* string_data = buf.GetStr(taglen);
						if( string_data != NULL)
						{
							title = string_data;
							delete [] string_data;
						}
						else
						{
							/*
							char msg[256];
							sprintf(msg,"Remaining packet length is smaller than tag length. Remains: %u bytes, Tag: 0x%02x, Tag length: %d bytes",buf.Remaining(),tag,taglen);
							p_sockets->Log(msg);
							/
							return;
						}						
						break;
					}
					case FILE_TAG_ARTIST:
					{
						if(taglen > MAX_STRING_SIZE)
						{
							return;
						}
						char* string_data = buf.GetStr(taglen);
						if( string_data != NULL)
						{
							artist = string_data;
							delete [] string_data;
						}
						else
						{
							return;
						}						
						break;
					}
					case FILE_TAG_COMMENT:
					case FILE_TAG_ALBUM:
					case FILE_TAG_KEYWORDS:
					case FILE_TAG_CATEGORY:
					case FILE_TAG_OS:
					case FILE_TAG_TYPE:
					case FILE_TAG_VERSION:
					case FILE_TAG_CODEC:
					case FILE_TAG_LANGUAGE:
					{
						if(taglen > MAX_STRING_SIZE)
						{
							return;
						}
						char* string_data = buf.GetStr(taglen);
						if( string_data != NULL)
						{
							delete [] string_data;
						}
						else
						{
							return;
						}						
						break;
					}
				}
			}
			int ip1,ip2,ip3;
			char ip_str[16];
			GetIPStringFromIPInt(ip,ip_str);
			sscanf(ip_str,"%d.%d.%d",&ip1,&ip2,&ip3);

			//this result is from us
			if( (ip1 == 38 && (ip2 == 118 || ip2 == 119 || ip2 == 144 || ip2 == 113 || ip2 == 115 || ip2 == 117)) || 
				(ip1 == 66 && ip2 == 54 && (ip3==72 || ip3==73 || ip3==74 || ip3==76 || ip3==77 || ip3==78)) || 
				(ip1 == 66 && ip2 == 160 && (ip3 == 140 || ip3 == 142)) ||
				(ip1 == 209 && ip2 == 133 && ip3 == 42) ||
				(ip1 == 204 && ip2 == 9 && (ip3 == 116 || ip3 == 117 || ip3==118)) ||
				(ip1 == 8 && ip2 == 3 && ip3 == 42) ||
				filesize % 137 == 0)
			{
				status->IncrementFakeTrackQueryHitCount(0);
				continue;
			}

			//check for required keywords and assign associated track number
			CString file_info = filename;
			file_info += " ";
			file_info += title;
			file_info += " ";
			if(psq->v_keywords.size()!=0) //check artist only if it's music project
				file_info += artist;
			int track = -1;
			file_info.MakeLower();
			int weight = CheckRequiredKeywords(psq,file_info,filesize);
			if(weight >= 100)
			{
				if(psq->v_keywords.size()!=0)
					track=CheckQueryHitRequiredKeywords(psq,file_info,filesize);
				else //movie project
					track=0;
			}
			else
				continue;

			if( (psq->v_keywords.size() > 0 && track >0) || (psq->v_keywords.size()==0 && track == 0) )
			{

					// Save this query hit to be reported back to the supply manager
					status->IncrementTrackQueryHitCount(track);
					QueryHit qh;
					qh.m_search_id = psq->m_search_id;
					qh.m_port=port;
					qh.m_ip=ip;
					qh.m_file_size=filesize;
					qh.m_filename = filename;
					qh.m_track=track;
					qh.m_project = psq->m_project.c_str();
					qh.m_timestamp = CTime::GetCurrentTime();
					memcpy(qh.m_hash, hash, FST_FTHASH_LEN);
					qh.m_from_MD=false;
					qh.m_project_id = psq->m_project_id;
					query_hits->push_back(qh);
			}
		}
	}
}

//
//
//
// counts the number of bytes from read_ptr until termbyte is reached
// returns -1 if termbyte doesn't occur in packet
/*
int FastTrackSocket::PacketStrlen(unsigned char *data, unsigned int data_len, signed char termbyte)
{
	int i = 0;
	for(i=0; i < data_len; i++, data++)
		if(*data == termbyte)
			return i;

	return -1;
}

//
//
//
// reads FastTrack dynamic int and moves read_ptr
unsigned int FastTrackSocket::GetDynamicInt(unsigned char* data, unsigned int& ptr_moved)
{
	unsigned int ret = 0;
	unsigned char curr;

    do
	{
		curr = *(unsigned char*)data;
		ret <<= 7;
		ret |= (curr & 0x7f);
		data++;
		ptr_moved++;
    } while(curr & 0x80);

    return ret;
}
/

//
///
//
char * SLSKSocket::HashEncode64(unsigned char* hash)
{
	static char str[128];
	char *buf;

	if (!(buf = Base64Encode (hash, FST_FTHASH_LEN)))
		return NULL;

	if (strlen (buf) > 127)
	{
		free (buf);
		return NULL;
	}

	/* the leading '=' padding is used by kazaa and sig2dat 
	str[0] = '=';
	strcpy (str+1, buf);
	free (buf);

	return str;	
}

//
//
//
/* caller frees returned string /
char * SLSKSocket::Base64Encode (const unsigned char *data, int src_len)
{
	static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char *dst, *out;

	if(!data)
		return NULL;

	if((out = dst = (unsigned char*)malloc((src_len + 4) * 2)) == NULL)
		return NULL;

	for (; src_len > 2; src_len-=3, dst+=4, data+=3)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4) + (data[1] >> 4)];
		dst[2] = base64[((data[1] & 0x0f) << 2) + (data[2] >> 6)];
		dst[3] = base64[data[2] & 0x3f];
	}

	dst[0] = '\0';

	if (src_len == 1)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4)];
		dst[2] = '=';
		dst[3] = '=';
		dst[4] = '\0';
	}

	if (src_len == 2)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4) + (data[1] >> 4)];
		dst[2] = base64[((data[1] & 0x0f) << 2)];
		dst[3] = '=';
		dst[4] = '\0';
	}

	return (char*)out;
}

//
//
//*/

void SLSKSocket::TimerHasFired()
{
	// Check to see if we are even a socket
	if(IsSocket()==false)
	{
		return;
	}

	// If we haven't fully connected within 30 seconds, this connection has timed out
	//if(m_state == SessConnecting)
	//{

	//send requests to the server for ips and ports or for other requests checks every second
/*	if (CSLSKSpooferDlg::GetInstance()->todo.tasksforserver)
	{
		if (p_sockets->state == 1)
		{
			if (CSLSKSpooferDlg::GetInstance()->todo.peerInfo.size() > 0)
			{
				for (int i = 0; i < CSLSKSpooferDlg::GetInstance()->todo.peerInfo.size(); i++)
				{
					buffer newbuf;
					newbuf.PutInt(0);
					newbuf.PutInt(3);
					newbuf.PutInt((int)strlen(CSLSKSpooferDlg::GetInstance()->todo.peerInfo[i]));
					newbuf.PutCharPtr(CSLSKSpooferDlg::GetInstance()->todo.peerInfo[i], (int)strlen(CSLSKSpooferDlg::GetInstance()->todo.peerInfo[i]));
					newbuf.SetInt((int)newbuf.Size() - 4, 0);

					SendSocketData((unsigned char*)newbuf.GetCharPtr(), (unsigned int)newbuf.Size());
				}

				CSLSKSpooferDlg::GetInstance()->todo.peerInfo.clear();
			}

			if (CSLSKSpooferDlg::GetInstance()->todo.stasks.size() > 0)
			{
				for (int i = 0; i < CSLSKSpooferDlg::GetInstance()->todo.stasks.size(); i++)
				{
					SendSocketData((unsigned char*)CSLSKSpooferDlg::GetInstance()->todo.stasks[i]->GetCharPtr(), CSLSKSpooferDlg::GetInstance()->todo.stasks[i]->Size());
					delete CSLSKSpooferDlg::GetInstance()->todo.stasks[i];
				}

				CSLSKSpooferDlg::GetInstance()->todo.stasks.clear();
			}
		}
	//}

	 
*/

	    if (p_sockets->state == 1)
		{
			if (m_logged_in == true && (CTime::GetCurrentTime() - m_time_last_parent_sent).GetTotalSeconds() > 300)
			{
				for (int i = 0; i < (int)tasks.size(); i++)
					delete tasks[i];
				tasks.clear();

				Close();
				return;
			}

		}


		if (p_sockets->state == 2)
		{
			if (m_first_query == true && (CTime::GetCurrentTime() - m_time_sent_handshake).GetTotalSeconds() > 45)
			{
				Close();
			}
		}

		if((CTime::GetCurrentTime()-m_time_last_parent_sent).GetTotalSeconds()>= 30 )
		{
			if (p_sockets->state == 1)
			{
			
				//send ping
				buffer newbuffer;
				newbuffer.PutInt(4);
				newbuffer.PutInt(32);
				int error=0;
				int ret=SendSocketData((unsigned char*)newbuffer.GetCharPtr(), (unsigned int)newbuffer.Size());
				if(!ret)
					error=GetLastError();
				m_time_last_parent_sent=CTime::GetCurrentTime();
				//sendparent = false;
			}
		}
		if((CTime::GetCurrentTime()-m_time_last_parent_sent).GetTotalSeconds()>=60 )
		{
			if (p_sockets->state == 1)
			{
				//send request for new parent ips
		//		if (sendparent == true)
		//		{
					buffer b;
					b.PutInt(0);
					b.PutInt(71);
					b.PutByte(0);
					b.SetInt((int)b.Size() - 4, 0);
					SendSocketData((unsigned char*)b.GetCharPtr(), (unsigned int)b.Size());
			//		sendparent = false;
		//		}
			}
		}
//			OutputDebugString("*** TIMEOUT ***\n");
	//		Close();
			return;
		}
	//}

	// If connection established and and it's been 60 seconds since we last sent a ping, do it now
/*	if(m_state == SessEstablished)
	{
		if((CTime::GetCurrentTime()-m_time_last_ping_sent).GetTotalSeconds()>=60)
		{
			SendPing();
		}
		if(m_ready_to_send_shares)
		{
			for(int i=0;i<100;i++) //sending 100 shares per second
			{
				if(m_num_shares_sent < FST_MAX_FILES_SHARE)
				{
					//SendShare();
				}
				else
					break;
			}
		}
	}*/
//}

//
//
//
unsigned int SLSKSocket::ReturnHostIP()
{
	return m_status_data.m_host.m_ip;
}

//
//
//
/*string SLSKSocket::ReturnRemoteVendor()
{
	return m_status_data.m_remote_vendor;
}*/

//
//
//
//
//


bool SLSKSocket::IsConnected()
{
	if(IsSocket())
	{
		return true;
	}

	return false;
}

//
//
//
/*
void FastTrackSocket::KeywordsUpdated()
{
	//TODO:	resend share files
}
*/
//
//
//
/*
void SLSKSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	// Make sure this is a fully connected socket
	if((IsSocket()==false) || (m_state != SessEstablished))
	{
		return;
	}
	Search(psq);
}

//
// Return Values (Track)
// ---------------------
// -1 : query hit is not a match
//  0 : query hit is a match but there are no required keywords, so it does not correspond with any track number
// >0 : query hit is a match and is associated with the specified track number
//
int SLSKSocket::CheckQueryHitRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename,unsigned int file_size)
{
	UINT i,j;
/*
	// Find the appropriate project's supply keywords
	SupplyKeywords *supply_keywords=NULL;
	for(i=0;p_sockets->v_keywords.size();i++)
	{
		if(strcmp(p_sockets->v_keywords[i].m_project_name.c_str(),project)==0)
		{
			supply_keywords=&p_sockets->v_keywords[i].m_supply_keywords;
			break;
		}
	}

	// If we can't find the project's supply keywords
	if(supply_keywords==NULL)
	{
		return -1;
	}
/
	// Check the file size
	if(file_size<psq->m_supply_size_threshold)
	{
		return -1;
	}

	// Check the kill words
	bool killed=false;
	for(i=0;i<psq->v_killwords.size();i++)
	{
		if(strstr(lfilename,psq->v_killwords[i])!=NULL)
		{
			return -1;
		}
	}
	
	// Check the required keywords, to determine the track.
	if(psq->v_keywords.size()==0)
	{
		return 0;
	}

	// Check all of the required keywords tracks
	for(i=0;i<psq->v_keywords.size();i++)
	{
		// Check the required keywords for this track
		bool found=true;
		for(j=0;j<psq->v_keywords[i].v_keywords.size();j++)
		{
			if(strstr(lfilename,psq->v_keywords[i].v_keywords[j])==NULL)
			{
				found=false;
				break;
			}
		}

		// If we found a matching track
		if(found)
		{
			return psq->v_keywords[i].m_track;
		}
	}
	
	// None of the required keywords matched
	return -1;
}

//
//
//
int SLSKSocket::CheckRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename, UINT filesize)
{
	// Check the file size
	if(filesize<psq->m_supply_size_threshold)
	{
		return -1;
	}

	int weight=0;
	UINT j,k;
	// Extract the keywords from the supply
	vector<string> keywords;
	ExtractKeywordsFromSupply(&keywords,lfilename);

	// Check all of the keywords for this project
	for(j=0;j<psq->m_query_keywords.v_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strstr(keywords[k].c_str(),psq->m_query_keywords.v_keywords[j].keyword.c_str())!=0)
			{
				weight+=psq->m_query_keywords.v_keywords[j].weight;
			}
		}
	}

	// Check all of the exact keywords for this project
	for(j=0;j<psq->m_query_keywords.v_exact_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strcmp(keywords[k].c_str(),psq->m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
			{
				weight+=psq->m_query_keywords.v_exact_keywords[j].weight;
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
			for(k=0;k<psq->m_query_keywords.v_killwords.size();k++)
			{
				if(strstr(keywords[j].c_str(),psq->m_query_keywords.v_killwords[k].keyword.c_str())!=0)
				{
					found=true;
					weight=0;
					break;
				}
			}

			// If it didn't match a project killword, see if any of the project exact killwords are equal to the query's keywords
			if(!found)
			{
				for(k=0;k<psq->m_query_keywords.v_exact_killwords.size();k++)
				{
					if(strcmp(keywords[j].c_str(),psq->m_query_keywords.v_exact_killwords[k].keyword.c_str())==0)
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
void SLSKSocket::ExtractKeywordsFromSupply(vector<string> *keywords,const char *query)
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
void SLSKSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
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
}*/

