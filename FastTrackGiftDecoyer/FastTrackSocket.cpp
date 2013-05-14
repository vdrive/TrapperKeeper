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
					//ReceivedQueryReply(buf,(FSTSessionMsg)m_message_type);
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
	PacketBuffer* buf = p_sockets->p_file_sharing_manager->GetShare();
	if(buf != NULL)
	{
		SendSessionMessage(*buf,SessMsgShareFile);
		delete buf;
		m_num_shares_sent++;
	}
}

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
		if(m_ready_to_send_shares)
		{
			for(int i=0;i<100;i++) //sending 100 shares per second
			{
				if(m_num_shares_sent < FST_MAX_FILES_SHARE)
				{
					SendShare();
				}
				else
					break;
			}
		}
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
