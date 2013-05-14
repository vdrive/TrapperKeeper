// NoiseSockets.cpp

#include "stdafx.h"
#include "NoiseSockets.h"
#include "FrameHeader.h"
#include "FileSharingManager.h"
//
//
//
NoiseSockets::NoiseSockets(HWND hwnd,CCriticalSection *connection_data_critical_section,vector<ConnectionData> *connection_data,FileSharingManager* fsm)
{
	m_sharing_enabled = true;
	p_fs_manager = fsm;
	m_hwnd=hwnd;
	p_connection_data_critical_section=connection_data_critical_section;
	p_connection_data=connection_data;

	m_num_bogus_connections=0;
	m_num_good_connections=0;

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

	//p_noise_data_buf=NULL;
	
	//InitNoiseData();
}

//
//
//
NoiseSockets::~NoiseSockets()
{
	/*
	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
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
}


//
//
//
DWORD NoiseSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int NoiseSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

//
//
//
int NoiseSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//
void NoiseSockets::AcceptConnection(SOCKET hSocket)
{
	int i;

	// Find an idle socket
	for(i=0;i<NumberOfSocketEvents;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_sockets[i].Attach(hSocket);
			break;
		}
	}
}

//
// Timer has fired - every second
//
vector<NoiseModuleThreadStatusData> NoiseSockets::ReportStatus()
{
	// Check to see if it a new day...to see if we need to init the noise data again
	/*
	CTime now=CTime::GetCurrentTime();
	if(m_time_last_initialized_noise_data.GetDay()!=now.GetDay())
	{
		InitNoiseData();
	}
	*/
	int i;

	vector<NoiseModuleThreadStatusData> status;

	for(i=0;i<NumberOfSocketEvents;i++)
	{
		NoiseModuleThreadStatusData data=m_sockets[i].ReportStatus();
		data.m_socket=i;
		data.m_num_bogus_connections=0;		// just in case
		data.m_num_good_connections=0;		// just in case
		status.push_back(data);
	}

	status[0].m_num_bogus_connections=m_num_bogus_connections;
	status[0].m_num_good_connections=m_num_good_connections;
	m_num_bogus_connections=0;
	m_num_good_connections=0;

	return status;
}

//
//
//
/*
void NoiseSockets::InitNoiseData()
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
bool NoiseSockets::InitNoiseDataFromFile(char *filename)
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
*/

//
//
//
void NoiseSockets::LogToDB(UploadInfo& info)
{
	p_fs_manager->StoreUploadLog(info);
}