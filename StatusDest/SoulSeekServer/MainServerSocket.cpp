#include "StdAfx.h"
#include ".\MainServerSocket.h"
#include "SoulSeekServerDll.h"
#include "PacketIO.h"
#include "ParentCacheIO.h"
#include "md5.h"
//
//
//
MainServerSocket::MainServerSocket(void)
{
	p_parent=NULL;
	m_bGotHeader = false;
}

//
//
//
MainServerSocket::~MainServerSocket(void)
{
	while(!lookupUsers.empty())
	{
		SearchRequest* sr = lookupUsers[lookupUsers.size() - 1];
		lookupUsers.pop_back();
		if(sr != NULL)
		{
			delete sr;
			sr = NULL;
		}
	}
}

//
//
//
void MainServerSocket::InitParent(SoulSeekServerDll *parent)
{
	p_parent=parent;
}

//
//
//
void MainServerSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}

	p_parent->m_dlg.SetConnectionInfo(1);

	// You are connected and can call SendSocketData() now or wait for an OnReceive()
	p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,"Sending LOGIN info");
	PacketWriter loginPacket(45+4+32);  // Create a packet with body size 45
	loginPacket.AppendInt(1,false); // login code
	loginPacket.AppendSizeAndString(USERNAME, false);  //username
	loginPacket.AppendSizeAndString(PASSWORD, false); //password
	loginPacket.AppendInt(156,false); //program version number
	//loginPacket.AppendSizeAndString("8d396d4dcd42e9595b4c5760aa47cd4d", false); //password
	
	unsigned char hash[16];
	ZeroMemory(hash, 16);
	md5_context md5;
	md5_starts(&md5);
	char* encrypt = new char [strlen(USERNAME)+strlen(PASSWORD)+1];
	memcpy(encrypt,USERNAME,strlen(USERNAME));
	memcpy(encrypt+strlen(USERNAME),PASSWORD,strlen(PASSWORD)+1);
	md5_update(&md5, (unsigned char *)encrypt, (unsigned long)strlen(encrypt) );
	md5_finish( &md5, (unsigned char *)hash );	// first 16 bytes of that hash
	char a32Hash[32+1];

	sprintf( a32Hash, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], 
		hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], 
		hash[14], hash[15] );

//p_parent->m_dlg.WriteToLog(a32Hash);

	loginPacket.AppendSizeAndString(a32Hash, false);

	SendSocketData( loginPacket.GetPacket(), loginPacket.GetPacketSize() );  //Send packet specifying packet string and size
	delete encrypt;
}

//
//
//
bool MainServerSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return false;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return false;
	}

	// ---

	// Your ReceiveSocketData() or ReceiveSomeSocketData() code goes here

	// For this example, we will attempt to receive the string "HELLO" without the NULL
	//p_parent->m_dlg.WriteToLog("ReceiveSocketData");
	ReceiveSocketData(4);

	// ---

	// Check for more data
	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			return true;	// continute to call OnReceive()
		}
	}

	return false;
}

//
//
//
void MainServerSocket::OnClose(int error_code)
{
	// Do any additional cleanup here
	Close();
	TAsyncSocket::OnClose(error_code);	// calls Close()
	p_parent->m_dlg.IncrementReconnect();
	p_parent->m_dlg.WriteToLog(2,MAIN_SERVER_NUM,"Connection Closed");
}

int MainServerSocket::Close()
{
	p_parent->m_dlg.SetConnectionInfo(0);
	return TAsyncSocket::Close();
}

//
//
//
void MainServerSocket::SocketDataReceived(char *data,unsigned int len)
{
	if( !m_bGotHeader )
	{
		//p_parent->m_dlg.WriteToLog("Processing message header");
		if(*(unsigned int *)(data) != 0)
		{
			m_bGotHeader = true;
			ReceiveSocketData( *(unsigned int *)(data) );
		}
		else p_parent->m_dlg.WriteToLog(2,MAIN_SERVER_NUM,"Body data of length 0");
	}
	else
	{
		m_bGotHeader = false;
		// process message
		PacketReader pr(data);
		int cmd;
		cmd = pr.GetNextInt();
		char cmdString[10+1];
		itoa(cmd,cmdString, 10);

		switch(cmd)
		{
			//RECEIVED LOGIN PACKET
			case 1:  //Login
				{
					if(pr.GetNextByte() != 1)
					{
						p_parent->m_dlg.WriteToLog(2,MAIN_SERVER_NUM,"Could not login");
						p_parent->m_dlg.SetConnectionInfo(0);
						break;
					}
					p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,"Login Successful");
					p_parent->m_dlg.SetConnectionInfo(2);

					PacketWriter *packet;

					p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,"Set wait port 6881");
					packet = new PacketWriter(8);  // Create a packet with body size 8
					packet->AppendInt(2,false); // wait port cmd code
					packet->AppendInt(LISTEN_PORT,false); //port to wait on
					SendSocketData( packet->GetPacket(), packet->GetPacketSize() );  //Send packet specifying packet string and size
					delete packet;

					p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,"Lookup my stats, send files and folders shared, and request parent list");
					//packet = new PacketWriter(21+12+12+13+5+4+4);  // Create a packet with body size 21
					
					//21
					packet = new PacketWriter(4+4+(int)strlen(USERNAME));
					packet->AppendInt(36,false); // CMD code
					packet->AppendSizeAndString(USERNAME,false); // Number of folders sharing
					SendSocketData( packet->GetPacket(), packet->GetPacketSize() );
					delete packet;
					//16
					packet = new PacketWriter(12);
					packet->AppendInt(35,false); // CMD code
					packet->AppendInt(3,false); // Number of folders sharing
					packet->AppendInt(68,false); // Number of files sharing
					SendSocketData( packet->GetPacket(), packet->GetPacketSize() );
					delete packet;					
					//9
					packet = new PacketWriter(5);
					packet->AppendInt(71,false);
					packet->AppendByte(1,false); // Give me parents
					SendSocketData( packet->GetPacket(), packet->GetPacketSize() );
					delete packet;		
					
					//24
					packet = new PacketWriter(4+4+(int)strlen(USERNAME)+4);
					packet->AppendInt(124,false); 
					packet->AppendInt(1,false);
					packet->AppendSizeAndString(USERNAME,false);
					SendSocketData( packet->GetPacket(), packet->GetPacketSize() );
					delete packet;

					ConnectToCachedParents();

					p_parent->m_dlg.StartTimer();  //Start timer to request new parent ip's every 5 minutes
				}
				break;

			//RECEIVE REQUESTED USER IP AND PORT
			case 3:
				{
					//get packet data
					char ip[16];
					char* username = pr.GetNextString();
					pr.GetNextIP(ip);
					int port = pr.GetNextInt();

					//find related search request
					SearchRequest* sr;
					sr = NULL;

					for(size_t i = 0; i < lookupUsers.size();i++)
					{
						if(strcmp(lookupUsers[i]->GetUsername(),username) == 0)
						{
							sr = lookupUsers[i];
							lookupUsers.erase(lookupUsers.begin()+i);
							if(strcmp(ip, "0.0.0.0") == 0 && port == 0)
							{
								delete sr;
								break;
							}
							//TODO
							//Set searchresult IP and port
							sr->SetConnection(ip,port);

							char s[256];
							sprintf(s,"Received user information: %s, %s:%i", sr->GetUsername(), sr->GetIP(), sr->GetPort());
							p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,s);

							//Pass sr along to connection manager
							delete sr;
							break;
						}
					}
					delete username;
				}
				break;

			//RECIEVED LIST OF SOME PARENTS
			case 102:
				{
					PacketWriter *packet;

					//p_parent->m_dlg.WriteToLog("Send Type 73 with value 0"); // Not sure what this does, but we will behave like a normal client
					//packet = new PacketWriter(8); // Create a packet with body size 8
					//packet->AppendInt(73,false);  // wait port cmd code
					//packet->AppendInt(1391415368,false);   // Send 0 info
					//SendSocketData( packet->GetPacket(), packet->GetPacketSize() );  //Send packet specifying packet string and size
					//delete packet;

					int numOfParentsSent = pr.GetNextInt();
					for(int i = 0; i < numOfParentsSent; i++)
					{
		
						char* username = pr.GetNextString(); 
						char ip[16];
						pr.GetNextIP(ip);
						p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,ip);
						int port = pr.GetNextInt();
						
						unsigned int token =
							p_parent->parents.AddParent(username, ip, port,1);  // 1 since we are connecting

						packet = new PacketWriter((int)(17+strlen(username)));  // Create a packet with body size 21
						packet->AppendInt(18,false); // CMD code
						packet->AppendInt(token,false);  // Token
						packet->AppendSizeAndString(username,false); // username
						packet->AppendInt(1,false);
						packet->AppendChar('D',false);
						SendSocketData( packet->GetPacket(), packet->GetPacketSize() );
						delete packet;

						delete[] username;
					}
				}
				break;
			// PIER CONNECTION ERROR
			case 1001:
				{
					//integer 1001 (Message Code) 
					//integer token 
					unsigned int token = pr.GetNextInt();
					p_parent->parents.FailedConnection(token);

				}
				break;

		}
	}
}

//
//
//
void MainServerSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	// Some data has been received

	// If you have not received enough data (HTTP header example)
	/*if(strstr(data,"\r\n\r\n")==NULL)
	{
		ReceiveSomeMoreSocketData(data,data_len,new_len,max_len);
		return;
	}*/
	
	// You have received enough data
}

void MainServerSocket::RequestMoreParents()
{
	PacketWriter packet(5);
	packet.AppendInt(71,false); // wait port cmd code
	packet.AppendByte(1,false); // Give me parents
	SendSocketData(packet.GetPacket(), packet.GetPacketSize());
}

void MainServerSocket::ConnectToCachedParents()
{
	p_parent->parents.ConnectToParents();
}

void MainServerSocket::GetClientAddress(SearchRequest* sr)
{

	lookupUsers.push_back(sr);
	
	//TODO BREAKS AT THE LINE bellow...sr->GetUsername == null?
	PacketWriter packet(4+4+(int)strlen(sr->GetUsername()));
	packet.AppendInt(3,false); // wait port cmd code
	packet.AppendSizeAndString(sr->GetUsername(),false); // Give me parents
	SendSocketData(packet.GetPacket(), packet.GetPacketSize());
}
