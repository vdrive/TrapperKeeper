// HttpSocket.cpp

#include "stdafx.h"
#include "HttpSocket.h"
#include "SLSKtask.h"
//#include "SLSKToDoList.h"
//#include "FrameHeader.h"
#include "NoiseSockets.h"
#include "SLSKSpooferDlg.h"
//#include "ConnectionData.h"
//#include "FTHash.h"
//#include "FileSharingManager.h"


//
//
//
HttpSocket::HttpSocket()
{
	p_sockets = NULL;
	m_file_len=0;
	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;
	m_rand_offset=0;
	m_connection_keep_alive = true;
	buffer_total_size = 4;
}

//
//
//
void HttpSocket::InitParent(NoiseSockets *sockets)
{
	p_sockets=sockets;
}

//
//
//
bool HttpSocket::Attach(SOCKET hSocket)
{
	m_status.m_start_time=CTime::GetCurrentTime();
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();

	bool ret=TEventSocket::Attach(hSocket);

	if(ret)
	{
		if(EventSelect(FD_ALL_EVENTS)!=0)
		{
			OnClose(301);
			ret=false;
		}
	}
	else
	{
		OnClose(302);
		ret=false;
	}

//#ifdef KAZAA_KILLER_ENABLED
	//crash kazaa!!
	//m_file_len=128*1024; //always send 128 kb data to crash kazaa
	//SendFrames();
//#endif
	return ret;
}

//
//
//
int HttpSocket::Close()
{
	m_file_len=0;
	m_file_num_sent=0;
	m_rand_offset=0;
	m_connection_keep_alive=true;

	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;

	// If we never got any data from this fool, it is bogus
	if(m_status.m_end==0)
	{
		p_sockets->m_num_bogus_connections++;
	}

	m_status.Clear();

	// Inform the parent

	return TEventSocket::Close();
}

//
//
//

void HttpSocket::OnConnect(int error_code)
{
/*	if (CSLSKSpooferDlg::GetInstance()->todo.ctm)
	{
		SLSKtask *t;
		

		vector<SLSKtask*>::iterator iter = CSLSKSpooferDlg::GetInstance()->todo.connectingtome.begin();

		CString temp;
		
		for (int i = 0; i < (int)CSLSKSpooferDlg::GetInstance()->todo.connectingtome.size(); i++, iter++)
		{
			t = CSLSKSpooferDlg::GetInstance()->todo.connectingtome[i];
			temp = ReturnRemoteIPAddressString2();
			if (strcmp(t->getIP(), temp) == 0)
			{
				SendSocketData((unsigned char*)t->getBuf().GetCharPtr(), (unsigned int)t->getBuf().Size());
			    CSLSKSpooferDlg::GetInstance()->todo.connectingtome.erase(iter);
			}

			if (CSLSKSpooferDlg::GetInstance()->todo.connectingtome.size() == 0)
				CSLSKSpooferDlg::GetInstance()->todo.ctm = false;
			
		}
	}*/
}

void HttpSocket::SomeSocketDataReceived(char* data, unsigned int data_len, unsigned int new_len, unsigned int max_len)
{
	buffer newbuffer;

	if (buffer_total_size == 4)
	{
		newbuffer.PutCharPtr(data, new_len);
		buffer_total_size = newbuffer.GetInt(0);
		return;
	}

	newbuffer.PutCharPtr(data, new_len);
	reply(newbuffer);
	buffer_total_size = 4;
}


void HttpSocket::SocketDataReceived(char* data, unsigned int len)
{
	buffer newbuffer;

	if (buffer_total_size == 4)
	{
		newbuffer.PutCharPtr(data, len);
		buffer_total_size = newbuffer.GetInt(0);
		return;
	}

	newbuffer.PutCharPtr(data, len);
	reply(newbuffer);
	buffer_total_size = 4;
}

void HttpSocket::OnReceive(int error_code)
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
		ReceiveSocketData(buffer_total_size);
	}
//
}

//
//
//
void HttpSocket::OnClose(int error_code)
{
	Close();	// just in case

	TEventSocket::OnClose(error_code);	// does nothing...yay!
}

//
// called when data has been sent
//
void HttpSocket::SocketDataSent(unsigned int len)
{
	// Check to see if the file length is zero. If so, then exit
	if(m_file_len==0)
	{
		OnClose(0);
		return;
	}
	if(m_file_num_sent > 20971520) //we have sent 20MB already, close the connection
	{
		OnClose(0);
		return;
	}
	//if(m_file_len>m_file_num_sent)
	//{
	//	SendFrames();
	//}
	else if(m_connection_keep_alive)
	{
		m_file_len=0;
		m_file_num_sent=0;
		m_rand_offset=0;
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;
		m_status.Clear();
	}
	else
	{
		OnClose(0);
	}
}

int HttpSocket::random(int lowest, int highest)
{
	double range = (highest - lowest) + 1.0;
    return lowest + (int) (range * rand() / RAND_MAX + 1.0);
}


void HttpSocket::reply(buffer &reply_buffer)
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

	switch(reply)
	{
	default:
		{
			if (buf.GetByte(index) != 0)
				break;

			index += 1;
			int token = buf.GetInt(index);

			int i = 0, randomnumber = 0, compsize = 0;
			char zipped[4096];
			UINT zippedsize = 4096;
			char* peerip = ReturnRemoteIPAddressString2();
			
			for (i = 0; i < (int)p_sockets->tasks.size(); i++)
				if (strcmp((p_sockets->tasks[i])->getIP(), peerip) == 0 && p_sockets->tasks[i]->m_token == token)
					break;

			
			//compare artist and send spoofs
			

			for (int j = 0; j < (int)p_sockets->m_spoofs.size(); j++)
			{
				if (p_sockets->m_spoofs[j].m_artist == p_sockets->tasks[i]->m_artist)
				{
					randomnumber = random(0, 9);
					
					buffer message;
					message.PutInt(0);
					message.PutInt(9);

					zippedsize = 4096;
					strcpy(zipped, "");

					compsize = compress((Bytef*)zipped, (uLong *)&zippedsize, (Bytef*)p_sockets->m_spoofs[j].m_spoof[randomnumber]->GetCharPtr(), (uLong)p_sockets->m_spoofs[j].m_spoof[randomnumber]->Size());

					if (compsize != Z_OK)
					{
						::MessageBox(NULL, "ERROR COMPRESSING THE SPOOFER", "COMPRESSION ERROR!", MB_OK);
					}

					message.PutCharPtr(zipped, zippedsize);

					message.SetInt(message.Size() - 4, 0);

					SendSocketData((unsigned char*)message.GetCharPtr(), (unsigned int)message.Size());

				}
			}

			if (i != p_sockets->tasks.size())
				delete p_sockets->tasks[i];

			break;
		}
	}

}
//
//
//
/*void HttpSocket::SendFrames()
{
/*
	int i;

	unsigned char buf[4*1024];
	memset(buf,0,sizeof(buf));

	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if(sizeof(buf)-offset<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if(sizeof(buf)-offset<frame_len)
		{
			break;
		}
			
		for(i=0;i<frame_len-sizeof(FrameHeader);i++)
		{
//			if(which)
//			{
//				buf[offset+i]=i%256;		// periodic
//			}
//			else
//			{
				buf[offset+i]=(i+rand()%2)%256;	// randon
//			}
		}
		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<sizeof(buf);i++)
	{
		buf[i]=rand()%256;
	}

	// Figure out how much to send
//	unsigned int num_to_send=sizeof(buf);
	unsigned int num_to_send=4096;
	if(num_to_send>(m_file_len-m_file_num_sent))
	{
		num_to_send=m_file_len-m_file_num_sent;
	}


	//not resetting m_file_num_sent so always sends crap to the user upon connection

#ifndef KAZAA_KILLER_ENABLED
	m_file_num_sent+=num_to_send;
#endif

	// Update the last time we sent stuff
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();
	
	// Increment offset in case we call anouther SendFrames() function before returning from this one
	m_rand_offset+=num_to_send;

	unsigned char *data=p_sockets->p_noise_data_buf;
	SendSocketData(&data[(m_rand_offset-num_to_send) % NOISE_DATA_LEN],num_to_send);
}

//
//
//*/
NoiseModuleThreadStatusData HttpSocket::ReportStatus()
{
	// Check to see if we need to whack this socket b/c i haven't sent some stuff for 1 minute
	if(IsSocket())	// if we're running
	{
		if((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalMinutes()>0)
		{
			OnClose(555);
			//TRACE("Closing timeout connection (1 min)\n");
			//p_sockets->Log("Closing timeout connection (1 min)\n");
			return m_status;
		}
	}

	if(IsSocket())
	{
		m_status.m_is_idle=false;
//		m_status.m_remote_ip=ReturnRemoteIPAddressString();
		m_status.m_remote_ip=ReturnRemoteIPAddress();
	}
	else
	{
		m_status.m_is_idle=true;
	}

	m_status.m_num_sent=m_file_num_sent;

	return m_status;
}

//
//
//
unsigned int HttpSocket::ReturnRemoteIPAddress()
{
	sockaddr_in addr;
	int len=sizeof(addr);
	getpeername(m_hSocket,(sockaddr *)&addr,&len);

	unsigned int ret=addr.sin_addr.S_un.S_addr;
	return ret;
}

//
//
//
char* HttpSocket::ReturnRemoteIPAddressString2()
{
	sockaddr_in addr;
	int len=sizeof(addr);
	getpeername(m_hSocket,(sockaddr *)&addr,&len);

	char address[256];
	unsigned long address_len=sizeof(address);
	WSAAddressToString((sockaddr *)&addr,sizeof(addr),NULL,address,&address_len);

	if(strchr(address,':')!=NULL)
	{
		*strchr(address,':')='\0';
	}

	char* ret=address;
	return ret;
}

string HttpSocket::ReturnRemoteIPAddressString()
{
	sockaddr_in addr;
	int len=sizeof(addr);
	getpeername(m_hSocket,(sockaddr *)&addr,&len);

	char address[256];
	unsigned long address_len=sizeof(address);
	WSAAddressToString((sockaddr *)&addr,sizeof(addr),NULL,address,&address_len);

	if(strchr(address,':')!=NULL)
	{
		*strchr(address,':')='\0';
	}

	string ret=address;
	return ret;
}

//
//
//
/*bool HttpSocket::CheckConnectionData(unsigned int ip,string filename)
{
	UINT i;

	// Check to see if this ip and filename already exists 2 times in the connection data vector
	ConnectionData data;
	data.m_ip=ip;
	data.m_filename=filename;

	bool ret=false;

	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		unsigned int count=0;
		for(i=0;i<p_sockets->p_connection_data->size();i++)
		{
			// Check to see how many other sockets are connected with this filename and filesize
			if(data==(*(p_sockets->p_connection_data))[i])
			{
				if((*(p_sockets->p_connection_data))[i].p_socket!=this)
				{
					count++;
				
					if(count>=2)
					{
						ret=true;
						break;
					}
				}
			}
		}

		singleLock.Unlock();
	}

	return ret;
}

//
//
//
void HttpSocket::SetConnectionData(unsigned int ip,string filename)
{
	UINT i;
	
	// Find the entry for this socket, if there is one.
	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		unsigned int count=0;
		for(i=0;i<p_sockets->p_connection_data->size();i++)
		{
			// Look for this socket
			if((*(p_sockets->p_connection_data))[i].p_socket==this)
			{
				(*(p_sockets->p_connection_data))[i].m_ip=ip;
				(*(p_sockets->p_connection_data))[i].m_filename=filename;
				return;
			}
		}

		// We didn't find it, so make a new one
		ConnectionData data;
		data.m_ip=ip;
		data.m_filename=filename;
		data.p_socket=this;
		p_sockets->p_connection_data->push_back(data);

		singleLock.Unlock();
	}
}*/

//
// Scrambles the bits of a 32 bit value, except for the MSB so that the case is unchanged
//
/*unsigned int HttpSocket::BitScramble(unsigned int val)
{
	unsigned int ret=0;

	ret|=((val>> 15 )&0x0001)<<0;
	ret|=((val>> 29 )&0x0001)<<1;
	ret|=((val>> 6  )&0x0001)<<2;
	ret|=((val>> 11 )&0x0001)<<3;
	ret|=((val>> 7  )&0x0001)<<4;
	ret|=((val>> 21 )&0x0001)<<5;
	ret|=((val>> 23 )&0x0001)<<6;
	ret|=((val>> 2  )&0x0001)<<7;
	ret|=((val>> 30 )&0x0001)<<8;
	ret|=((val>> 14 )&0x0001)<<9;
	ret|=((val>> 26 )&0x0001)<<10;
	ret|=((val>> 18 )&0x0001)<<11;
	ret|=((val>> 0  )&0x0001)<<12;
	ret|=((val>> 27 )&0x0001)<<13;
	ret|=((val>> 12 )&0x0001)<<14;
	ret|=((val>> 10 )&0x0001)<<15;
	ret|=((val>> 22 )&0x0001)<<16;
	ret|=((val>> 28 )&0x0001)<<17;
	ret|=((val>> 1  )&0x0001)<<18;
	ret|=((val>> 25 )&0x0001)<<19;
	ret|=((val>> 5  )&0x0001)<<20;
	ret|=((val>> 9  )&0x0001)<<21;
	ret|=((val>> 20 )&0x0001)<<22;
	ret|=((val>> 13 )&0x0001)<<23;
	ret|=((val>> 8  )&0x0001)<<24;
	ret|=((val>> 19 )&0x0001)<<25;
	ret|=((val>> 17 )&0x0001)<<26;
	ret|=((val>> 4  )&0x0001)<<27;
	ret|=((val>> 24 )&0x0001)<<28;
	ret|=((val>> 16 )&0x0001)<<29;
	ret|=((val>> 3  )&0x0001)<<30;

	return ret;
}

//
// UnScrambles the bits of a 32 bits value, except for the MSB so that the case is unchanged
//
unsigned int HttpSocket::BitUnScramble(unsigned int val)
{
	unsigned int ret=0;

	ret|=((val>> 0  )&0x0001)<<15;
	ret|=((val>> 1  )&0x0001)<<29;
	ret|=((val>> 2  )&0x0001)<<6;
	ret|=((val>> 3  )&0x0001)<<11;
	ret|=((val>> 4  )&0x0001)<<7;
	ret|=((val>> 5  )&0x0001)<<21;
	ret|=((val>> 6  )&0x0001)<<23;
	ret|=((val>> 7  )&0x0001)<<2;
	ret|=((val>> 8  )&0x0001)<<30;
	ret|=((val>> 9  )&0x0001)<<14;
	ret|=((val>> 10 )&0x0001)<<26;
	ret|=((val>> 11 )&0x0001)<<18;
	ret|=((val>> 12 )&0x0001)<<0;
	ret|=((val>> 13 )&0x0001)<<27;
	ret|=((val>> 14 )&0x0001)<<12;
	ret|=((val>> 15 )&0x0001)<<10;
	ret|=((val>> 16 )&0x0001)<<22;
	ret|=((val>> 17 )&0x0001)<<28;
	ret|=((val>> 18 )&0x0001)<<1;
	ret|=((val>> 19 )&0x0001)<<25;
	ret|=((val>> 20 )&0x0001)<<5;
	ret|=((val>> 21 )&0x0001)<<9;
	ret|=((val>> 22 )&0x0001)<<20;
	ret|=((val>> 23 )&0x0001)<<13;
	ret|=((val>> 24 )&0x0001)<<8;
	ret|=((val>> 25 )&0x0001)<<19;
	ret|=((val>> 26 )&0x0001)<<17;
	ret|=((val>> 27 )&0x0001)<<4;
	ret|=((val>> 28 )&0x0001)<<24;
	ret|=((val>> 29 )&0x0001)<<16;
	ret|=((val>> 30 )&0x0001)<<3;

	return ret;
}*/
