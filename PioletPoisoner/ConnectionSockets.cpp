// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
//#include "VendorCount.h"
#include "FrameHeader.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
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
	
	p_noise_data_buf=NULL;
	InitNoiseData();
}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
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
	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
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
/*
ConnectionModuleStatusData ConnectionSockets::ReportStatus()
{
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
			m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	ConnectionModuleStatusData ret=m_status_data;
	
	m_status_data.ClearCounters();
	
	return ret;
}
*/
//
//
//
/*
void ConnectionSockets::ConnectToHost(char* ip, int port, UINT file_length, const char* md5)
{
	UINT j;

	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		if(m_sockets[j].IsSocket()==false)
		{
			m_sockets[j].CreateSocket(ip,port, file_length, md5);
			//m_sockets[j].Connect(ip,port, file_length, md5);
			break;
		}
	}
}
*/
//
//
//
void ConnectionSockets::AcceptConnection(SOCKET hSocket,char* ip, int port, UINT file_length, const char* md5)
{
	UINT j;

	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		if(m_sockets[j].IsSocket()==false)
		{
			//m_sockets[j].CreateSocket(ip,port, file_length, md5);
			m_sockets[j].AcceptConnection(hSocket,ip,port, file_length, md5);
			//m_sockets[j].Connect(ip,port, file_length, md5);
			break;
		}
	}
}
//
//
//
/*
void ConnectionSockets::ReportHosts(vector<GnutellaHost> &hosts)
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
*/

//
//
//
int ConnectionSockets::GetNumIdleSockets()
{
	int idles =0;
	UINT j;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		if(m_sockets[j].IsSocket()==false)
		{
			idles++;
		}
	}
	return idles;
}

//
//
//
void ConnectionSockets::CloseIdleSockets()
{
	UINT j;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		m_sockets[j].CloseIdle();
	}
}



//
//
//
void ConnectionSockets::InitNoiseData()
{
	// This is the time we last initialized the noise data
	m_time_last_initialized_noise_data=CTime::GetCurrentTime();

	// Try to init the noise data from a file
	if(InitNoiseDataFromFile("test.mp3"))
	{
		return;
	}

	// Else use random noise

	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
	}

	UINT i;

	// Seed the rand() with 12 a.m. today
	CTime now=CTime::GetCurrentTime();
	CTime today(now.GetYear(),now.GetMonth(),now.GetDay(),0,0,0);	// 12 a.m. today
	srand((unsigned int)today.GetTime());

	p_noise_data_buf=new unsigned char[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if((NOISE_DATA_LEN-offset)<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&p_noise_data_buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if((NOISE_DATA_LEN-offset)<frame_len)
		{
			break;
		}
			
		for(i=0;i<frame_len-sizeof(FrameHeader);i++)
		{
//			p_noise_data_buf[offset+i]=(i+rand()%2)%256;	// noisy hell
//			p_noise_data_buf[offset+i]=rand()%256;			// less frequent noise (on winamp, but not on mediaplayer)
			
			p_noise_data_buf[offset+i]=rand()%12;			// quiet noise
		}

		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<NOISE_DATA_LEN;i++)
	{
		p_noise_data_buf[i]=rand()%256;
	}

	// Copy out the buffer padding
	memcpy(&p_noise_data_buf[NOISE_DATA_LEN],&p_noise_data_buf[0],NOISE_BUFFER_LEN);
}

//
//
//
bool ConnectionSockets::InitNoiseDataFromFile(char *filename)
{
	int i;

	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
	}

	// Seed the rand() with 12 a.m. today
	CTime now=CTime::GetCurrentTime();
	CTime today(now.GetYear(),now.GetMonth(),now.GetDay(),0,0,0);	// 12 a.m. today
	srand((unsigned int)today.GetTime());

	p_noise_data_buf=new unsigned char[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
	CFile file;
	if(file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)==NULL)
	{
		return false;
	}

	// Seek past the zeros to find the first frame
	unsigned int first_frame_index=0;
	while(1)
	{
		char c;
		file.Read(&c,1);
		if(c==0)
		{
			first_frame_index++;
		}
		else
		{
			break;
		}
	}

	// Position the file pointer at the first frame
	file.Seek(first_frame_index,CFile::begin);

	FrameHeader hdr;

	unsigned int offset=0;
	unsigned int num_read=0;

	while(1)
	{
		// Check to see if a frame header will go beyond the buffer
		if((NOISE_DATA_LEN-offset)<sizeof(FrameHeader))
		{
			break;
		}

		num_read=file.Read(&hdr,sizeof(FrameHeader));

		// if EOF, reset file
		if(num_read!=sizeof(FrameHeader))
		{
			file.Seek(first_frame_index,CFile::begin);
			file.Read(&hdr,sizeof(FrameHeader));
		}

		if(hdr.IsValid()==false)
		{
			return false;
		}

		unsigned int frame_len=hdr.ReturnFrameSize();

		// Check to see if this frame will go beyond the buffer
		if((NOISE_DATA_LEN-offset)<frame_len)
		{
			break;
		}

		memcpy(&p_noise_data_buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		num_read=file.Read(&p_noise_data_buf[offset],frame_len-sizeof(FrameHeader));
		
		// Make sure that all of the frame data is there
		if(num_read!=(frame_len-sizeof(FrameHeader)))
		{
			return false;
		}

		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<NOISE_DATA_LEN;i++)
	{
		p_noise_data_buf[i]=rand()%256;
	}

	// Copy out the buffer padding
	memcpy(&p_noise_data_buf[NOISE_DATA_LEN],&p_noise_data_buf[0],NOISE_BUFFER_LEN);

	return true;
}