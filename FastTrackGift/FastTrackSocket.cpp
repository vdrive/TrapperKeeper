#include "StdAfx.h"
#include "fasttracksocket.h"
#include "ConnectionSockets.h"
#include "EncryptionType2.h"
//#include "cryptlib.h"	// for CryptoPP
#include "FileSharingManager.h"


FastTrackSocket::FastTrackSocket(void)
{
	srand((unsigned)time( NULL ));
	p_in_cipher = p_out_cipher = NULL;
	ResetData();
}

//
//
//
FastTrackSocket::~FastTrackSocket(void)
{
	ResetData();
	if(p_in_cipher)
		delete p_in_cipher;
	if(p_out_cipher)
		delete p_out_cipher;

}

//
//
//
//
//
//
void FastTrackSocket::InitParent(ConnectionSockets* parent)
{
	p_sockets = parent;

}

//
//
//
void FastTrackSocket::ResetData()
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
	m_last_last_shared_file_sent=CTime::GetCurrentTime();
}

//
//
//
int FastTrackSocket::Connect(SupernodeHost host)
{
	Close();

	m_status_data.m_host=host;

	m_status_data.m_connect_time=CTime::GetCurrentTime();
	m_state = SessConnecting;
	Create();
	char ip[16];
	memset(&ip,0,16);
	host.GetIP(ip);


	return TEventSocket::Connect(ip,host.m_port);
}

//
//
//
int FastTrackSocket::Close()
{
	ResetData();
	//p_sockets->Log("Socket closed");
	//TRACE("FastTrack Socket Closed\n");
	return TEventSocket::Close();
}

//
//
//
void FastTrackSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		//char msg[128];
		//sprintf(msg,"FastTrackSocket::OnConnect() - Error: %d\n",error_code);
		//TRACE("FastTrackSocket::OnConnect() - Error: %d\n",error_code);
		//p_sockets->Log(msg);
		return;
	}
	//p_sockets->Log("Connected. Sending Init Handshake");
	m_state = SessHandshaking;
	byte init_handshake[12];

	unsigned int* unknown = (unsigned int*)&init_handshake[0];
	unsigned int* seed = (unsigned int*)&init_handshake[4];
	unsigned int* enc_type = (unsigned int*)&init_handshake[8];
	
	*unknown = htonl(0xFA00B62B);

	*seed = htonl(p_out_cipher->seed);
	*enc_type = htonl(GetEncryptionType(p_out_cipher->seed, p_out_cipher->enc_type));

	SendSocketData((unsigned char *)init_handshake,(UINT)sizeof(init_handshake));
}

//
//
//
void FastTrackSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	// If we haven't received the connection string yet
	if(m_state == SessHandshaking)
	{
		ReceiveSocketData(8);
		return;
	}
	if(m_state == SessWaitingNetName)
	{
		ReceiveSomeSocketData(4096);
		return;
	}
	if(m_state == SessEstablished && !m_received_header)
	{
		ReceiveSomeSocketData(5);
		return;
	}
	if(m_state == SessEstablished && m_received_header)
	{
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
}

//
//
//
void FastTrackSocket::OnClose(int error_code)
{
	/*
	char log[128];
	sprintf(log,"Socket closed: %d",error_code);
	p_sockets->Log(log);
	*/
	m_state = SessDisconnected;
	TEventSocket::OnClose(error_code);
}

//
//
//
void FastTrackSocket::SocketDataSent(unsigned int len)
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
	*/
}

//
//
//
void FastTrackSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
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
				/* we got ping */
				case 0x50:
				{
					//p_sockets->Log("Got Ping");
					p_sockets->m_status_data.m_ping_count++;
					SendPong();
					return;
				}
				/* we got pong */
				case 0x52:
				{
					p_sockets->m_status_data.m_pong_count++;
					//p_sockets->Log("Got Pong");
					return;
				}
				/* we got a message */
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
					*/
					m_message_type = msg_type;
					m_message_length = msg_len;
					/* modify xinu state */
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
					*/
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
void FastTrackSocket::SocketDataReceived(char *data,unsigned int len)
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

			/* doing something with the message data here */
			switch(m_message_type)
			{
				/*
				char msg[128];
				sprintf(msg,"Received message data. Length - %d bytes",len);
				p_sockets->Log(msg);
				*/
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

					mantissa = ntohs(buf.GetUShort());	/* mantissa of size */
					exponent = ntohs(buf.GetUShort());	/* exponent of size */

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
					*/
					
					
					// if we connected to a splitted network segment move on
					if (users < MIN_USERS_ON_CONNECT)
					{
						/*
						char msg[256];
						sprintf(msg,"Disconnecting from a splitted network segment which only has %u users.",users);
						p_sockets->Log(msg);
						*/
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
					*/
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
					*/
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
unsigned int FastTrackSocket::GetEncryptionType(unsigned int seed, unsigned int enc_type)
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
bool FastTrackSocket::GetIncomingSeedAndEncryptionType(unsigned char* data, int len)
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
int FastTrackSocket::DoHandshake (unsigned char * data)
{
	unsigned int seed, enc_type;

	/* get seed and enc_type */
	seed = ntohl( (*(unsigned int*)data) );
	enc_type = ntohl( (*(unsigned int*)&data[4]) );
	enc_type = fst_cipher_mangle_enc_type(seed, enc_type);

	/* generate send key */
	p_out_cipher->seed ^= seed; /* xor send cipher with received seed */

	/* the correct behaviour here is to use the enc_type the supernode sent
	 * us for out_cipher too.
	 * thanks to HEx and weinholt for figuring this out.
	 */
	p_out_cipher->enc_type = enc_type;

	if(!fst_cipher_init(p_out_cipher, p_out_cipher->seed,
						 p_out_cipher->enc_type))
	{
		char msg[128];
		sprintf(msg,"Unsupported encryption: 0x%02X",p_out_cipher->enc_type);
		p_sockets->Log(msg);
		return FALSE;
	}

	/* generate recv key */
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
void FastTrackSocket::GetRemoteServerName(unsigned char* data, UINT data_len)
{
	fst_cipher_crypt(p_in_cipher,data,data_len);
	m_state = SessEstablished;
	/*
	char msg[128];
	sprintf(msg, "Session established. Remote network name is %s",(char*)data);
	p_sockets->Log(msg);
	*/
	SendMyNetworkName();
}

//
//
//
void FastTrackSocket::SendMyNetworkName(void)
{
	/*
	char msg[128];
	sprintf(msg,"Send my network name: %s",FST_NETWORK_NAME);
	p_sockets->Log(msg);
	*/

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
void FastTrackSocket::SessionGreetSupernode()
{
	//p_sockets->Log("Sending ip, bandwidth and user name to supernode");

	/* Send our ip address and port.
	 * Should we send our outside ip if NATed? We don't have it at this
	 * point from the supernode.
	 */

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
	buf.PutUShort(htons(21688));
#else
	buf.PutUShort(htons(3368));
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
//
unsigned int FastTrackSocket::GetIntIPFromStringIP(const char* ip)
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
void FastTrackSocket::GetIPStringFromIPInt(int ip_int, char* ip_str)
{
	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
}

//
//
//
void FastTrackSocket::SendSessionMessage(PacketBuffer& buf, FSTSessionMsg msg_type)
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
	*/

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

	/* update xinu state */
	m_out_xinu ^= ~(buf.Size() + msg_type);
	new_buf.Append(buf);
	fst_cipher_crypt(p_out_cipher,new_buf.p_data,new_buf.Size());
	SendSocketData(new_buf.p_data,new_buf.Size());
}

//
//
//
/* send out pong response */
void FastTrackSocket::SendPong()
{
	unsigned char buf = 0x52;
	fst_cipher_crypt(p_out_cipher,&buf,1);
	SendSocketData(&buf,1);
	//p_sockets->Log("Pong sent");
}

//
//
//
/* send out ping request */
void FastTrackSocket::SendPing()
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
void FastTrackSocket::SendShare(void)
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
*/

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
	*/
	PacketBuffer* buf = p_sockets->p_file_sharing_manager->GetShare();
	if(buf != NULL)
	{
		SendSessionMessage(*buf,SessMsgShareFile);
		delete buf;
		m_num_shares_sent++;
		m_last_last_shared_file_sent=CTime::GetCurrentTime();
	}
#endif
	
}

//
//
//
void FastTrackSocket::Search(ProjectSupplyQuery& psq)
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
void FastTrackSocket::ReceivedQueryReply(PacketBuffer& buf, FSTSessionMsg msg_type)
{
	if (msg_type == SessMsgQueryEnd)
	{
		unsigned short search_id = ntohs( buf.GetUShort());
		/*
		char msg[256];
		sprintf(msg,"Received end of search for search id = %d",search_id);//, %d replies, %d firewalled, %d banned",fst_id, search->replies, search->fw_replies, search->banlist_replies);
		p_sockets->Log(msg);
		*/
		return;
	}
	else if (msg_type == SessMsgQueryReply)
	{
		/* supernode ip an port */
		unsigned int sip = buf.GetUInt();
		unsigned short sport = ntohs(buf.GetUShort());

		/* get query id and look up search */
		unsigned short search_id = ntohs(buf.GetUShort());

		/* get number of results */
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
		*/

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

			/* user and network name */
			if(*(byte*)(buf.p_read_ptr) == 0x02)
			{
				/* compressed, look up names based on ip and port */
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
				*/
			}
			else
			{
				/* user name */
				
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


				/* network name */
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
			*/

			
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
				* corruption issue on the sender side. */
				
		
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
							*/
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
							*/
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
							*/
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
							*/
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
							*/
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
							*/
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
			//this result is from us
			if(IsResultFromUs(ip,filesize))
			{
				status->IncrementFakeTrackQueryHitCount(0);
				continue;
			}
			//check filename
			if(IsFilenameExcluded(filename))
			{
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
int FastTrackSocket::PacketStrlen(unsigned char *data, unsigned int data_len, unsigned char termbyte)
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
*/

//
///
//
char * FastTrackSocket::HashEncode64(unsigned char* hash)
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

	/* the leading '=' padding is used by kazaa and sig2dat */
	str[0] = '=';
	strcpy (str+1, buf);
	free (buf);

	return str;	
}

//
//
//
/* caller frees returned string */
char * FastTrackSocket::Base64Encode (const unsigned char *data, int src_len)
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
//
void FastTrackSocket::TimerHasFired()
{
	// Check to see if we are even a socket
	if(IsSocket()==false)
	{
		return;
	}

	// If we haven't fully connected within 30 seconds, this connection has timed out
	if(m_state == SessConnecting)
	{
		if((CTime::GetCurrentTime()-m_status_data.m_connect_time).GetTotalSeconds()>30)
		{
//			OutputDebugString("*** TIMEOUT ***\n");
			Close();
			return;
		}
	}

	// If connection established and and it's been 60 seconds since we last sent a ping, do it now
	if(m_state == SessEstablished)
	{
		if((CTime::GetCurrentTime()-m_time_last_ping_sent).GetTotalSeconds()>=60)
		{
			SendPing();
		}
#ifdef SHARING_ENABLED
		if(m_ready_to_send_shares)
		{
//#ifndef DC2
			for(int i=0;i<50;i++) //sending 50 shares per second
			{
				if(m_num_shares_sent < FST_MAX_FILES_SHARE)
				{
					SendShare();
				}
				else
				{
					CTimeSpan ts;
					ts=CTime::GetCurrentTime()-m_last_last_shared_file_sent;
					if(ts.GetTotalSeconds() >= FST_RE_UPLOAD_FILES)
					{
						m_num_shares_sent=0;
						//this->Close();
					}
					break;
				}
			}
//#else
			//for(int i=0;i<5;i++) //sending 5 shares per second
			//{
			//	SendShare();
			//}		
//#endif
		}
#endif
	}
}

//
//
//
unsigned int FastTrackSocket::ReturnHostIP()
{
	return m_status_data.m_host.m_ip;
}

//
//
//
string FastTrackSocket::ReturnRemoteVendor()
{
	return m_status_data.m_remote_vendor;
}

//
//
//
//
//
//
bool FastTrackSocket::IsConnected()
{
	if(IsSocket())
	{
		if(m_state == SessEstablished)
		{
			return true;
		}
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
void FastTrackSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
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
int FastTrackSocket::CheckQueryHitRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename,unsigned int file_size)
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
*/
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
int FastTrackSocket::CheckRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename, UINT filesize)
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
void FastTrackSocket::ExtractKeywordsFromSupply(vector<string> *keywords,const char *query)
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
void FastTrackSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
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
bool FastTrackSocket::IsResultFromUs(unsigned int ip, unsigned int filesize)
{
#ifndef NO_PROCESSING
	if(filesize % 137 == 0)
		return true;
#endif
	int ip1,ip2,ip3;
	char ip_str[16];
	GetIPStringFromIPInt(ip,ip_str);
	sscanf(ip_str,"%d.%d.%d",&ip1,&ip2,&ip3);

	if( (ip1==72 && ip2==35 && ip3==224) ||
		(ip1==216 && ip2==151 && (ip3==154 || ip3==155)) ||
		(ip1==154 && ip2==37 && ip3==66) ||
		(ip1==63 && ip2==219 && ip3==21) ||
		(ip1==63 && ip2==216 && ip3==76) ||
		(ip1==208 && ip2==49 && ip3==28) )
	{
		return true;
	}
	return false;
}

//
//
//
bool FastTrackSocket::IsFilenameExcluded(CString& filename)
{
	if(filename.Find("___ARESTRA___")==0)
		return true;
	else
		return false;
}