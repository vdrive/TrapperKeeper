// NoiseSockets.cpp

#include "stdafx.h"
#include "NoiseSockets.h"
#include "fstream"
#include "ProjectKeywords.h"
#include "db.h"
//#include "FrameHeader.h"

//
//
//
void NoiseSockets::CreateSpoofs()
{
	
//	delete task;
//	return;
	//TRACE("Creating SPOOFS!\n");

	int i, j, k, l, bitrate;
	int songlength;
	long songsize;
	CString clean_path, path, spoofname;
	buffer *buf;
//	ofstream fout;
//	fout.open("C:\\spoofscreated.txt", ios::out);
	//char zipped[1024];
	//UINT zippedsize = 1024;
	//TrackInformation *track_info;

	SupplyKeywords *sk;
	int a = (int)m_projects->v_projects.size();
//	spoofs.clear();
//	for (int i = 0; i < task->tracks.size(); i++)
	//{
//	fout << "number of projects = " << a << endl;
	for (i = 0; i < (int)m_projects->v_projects.size(); i++)
	{
		//TRACE("IN FOR I FOR LOOP OF CREATING SPOOOFS!\n");
		spoof s;
		ProjectKeywords *ptr_keyword = &m_projects->v_projects[i];

		
		if (ptr_keyword->m_soulseek_spoofing_enabled == false)
			continue;

	//	fout << "project title = " << ptr_keyword->m_project_name << endl;

		s.m_artist = ptr_keyword->m_artist_name.c_str();
	//	Log("Generating Spoofs...");
//		bool file_found;
		if (ptr_keyword->m_search_type == ProjectKeywords::search_type::video)
			{
				//TRACE("CREATING SPOOF IS A VIDEO!\n");
				for (j = 0; j < 10; j++)
				{
					//TRACE("IN j loop of creating spoof!\n");
					buf = new buffer();
					spoofname = getSpoofName();
					buf->PutInt((int)strlen((char*)(LPCTSTR)spoofname));//(int)strlen((const char*)(LPCTSTR)spoofname));
					buf->PutCharPtr((char*)(LPCTSTR)spoofname, (int)strlen((char*)(LPCTSTR)spoofname));//(const char*)(LPCTSTR)spoofname, (int)strlen((const char*)(LPCTSTR)spoofname));
					buf->PutInt(0); // token spot

			
					sk  = &(ptr_keyword->m_supply_keywords);
					buf->PutInt(100);
					for (l = 0; l < 100; l++)
					{
						//TRACE("IN l loop of creating spoof!\n");
						clean_path = GetFilePath(ptr_keyword->m_artist_name.c_str(), " ");


						path = clean_path;
						buf->PutByte(1);
						path.Append(ptr_keyword->m_artist_name.c_str());
						path.Append(".mpg");
						buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
						buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));


						buf->PutLong(689789787);
						buf->PutInt(0);
						buf->PutInt(3);
						buf->PutCharPtr("mpg", 3);
						buf->PutInt(0);


						//TRACE("END OF L LOOP FOR VIDEO CREATING SPOOF!\n");
					}

					if ( j == 2 || j == 6)
					{
						buf->PutByte(0);
						buf->PutInt(random(1000, 80000));
						buf->PutInt(rand() % 10);
					}
					else
					{
					buf->PutByte(1);
					buf->PutInt(random(1000, 80000));
					buf->PutInt(0);	
					}
				//	fout << buf->GetCharPtr() << endl;
					


				
					//fout.write((const unsigned char*)buf->GetCharPtr(), buf->Size());
					s.m_spoof.push_back(buf);
					//TRACE("END OF J LOOP FOR VIDEO CREATING SPOOF!\n");
				}

				m_spoofs.push_back(s);
				//TRACE("END OF VIDEO CREATING SPOOF!\n");
			}
			else
				if (ptr_keyword->m_search_type == ProjectKeywords::search_type::audio)
			{
				//TRACE("AUDIO CREATING SPOOF!\n");
			
		for (j = 0; j < 10; j++)
		{
			//TRACE("IN J LOOP OF AUDIO CREATING SPOOF!\n");
			buf = new buffer();
			spoofname = getSpoofName();
			buf->PutInt((int)strlen((char*)(LPCTSTR)spoofname));//(int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutCharPtr((char*)(LPCTSTR)spoofname, (int)strlen((char*)(LPCTSTR)spoofname));//(const char*)(LPCTSTR)spoofname, (int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutInt(0); // token spot

		//	fout << "spoof name = " << spoofname.GetString() << endl;
			
			sk  = &(ptr_keyword->m_supply_keywords);
			buf->PutInt((int)sk->v_keywords.size() * 10);

		//	fout << "number of tracks = " << sk->v_keywords.size() << endl;
			for (l = 0; l < 10; l++)
			{

				//TRACE("IN L LOOP OF AUDIO CREATING SPOOF!\n");
			clean_path = GetFilePath(ptr_keyword->m_artist_name.c_str(), ptr_keyword->m_album_name.c_str());

			if (random(0, 1) == 0)
				bitrate = 192;
			else
				bitrate = 128;

			for (k = 0; k < (int)sk->v_keywords.size(); k++)
			{
				//if (k == 15) 
				//	continue;
			    //TRACE("IN K LOOP OF AUDIO CREATING SPOOF!\n");
			 //   sk  = &(ptr_keyword->m_supply_keywords);
			    

				//track_info = new TrackInformation(*(*m_track_info)[k]);
					//counter = 0; 
				

                 
				path = clean_path;
				buf->PutByte(1);
			//	fout << "a bit i forgot for what " << endl;
				path.Append(sk->v_keywords[k].m_track_name.c_str());
				path.Append(".mp3");
			//	fout << "length of path = " << strlen((const char*) path) << endl;
				buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
			//	fout << "The path = " << path.GetString() << endl;
				buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));
			
				//		file_found = false;
				//TRACE ("IN MIDDLE OF K LOOP OF AUDIO CREATING SPOOF!\n");

	//			file_found = true;
				/*if (k == 0)
					buf->PutLong(7354288);
				if (k == 1)
					buf->PutLong(2000230);
				if (k == 2)
					buf->PutLong(2746287);
				if (k == 3)
					buf->PutLong(6104799);
				if (k == 4)
					buf->PutLong(6080348);
				if (k == 5)
					buf->PutLong(5166898);
				if (k == 6)
					buf->PutLong(5783806);
				if (k == 7)
					buf->PutLong(4180096);
				if (k == 8)
					buf->PutLong(6899130);
				if (k == 9)
					buf->PutLong(10472681);
				if (k == 10)
					buf->PutLong(5385700);
				if (k == 11)
					buf->PutLong(4324242);
				if (k == 12)
					buf->PutLong(9532273);
				if (k == 13)
					buf->PutLong(7492841);
				if (k > 13)
				{*/
					songsize = random(2000000, 10000000);
				    songlength = (songsize / 1000000) * 69;
					buf->PutLong(songsize);
			/*	}*/


				buf->PutInt(0);
				buf->PutInt(3);
				buf->PutCharPtr("mp3", 3);
				buf->PutInt(3);
				buf->PutInt(0);
				buf->PutInt(bitrate);
				buf->PutInt(1);

			//	fout << "file size\n0\n3\nmp3\n3\n0\n" << bitrate << "\n1\nlength\n";
				/*if (k == 0)
					buf->PutInt(506);
				if (k == 1)
					buf->PutInt(123);
				if (k == 2)
					buf->PutInt(154);
				if (k == 3)
					buf->PutInt(414);
				if (k == 4)
					buf->PutInt(413);
				if (k == 5)
					buf->PutInt(335);
				if (k == 6)
					buf->PutInt(400);
				if (k == 7)
					buf->PutInt(254);
				if (k == 8)
					buf->PutInt(447);
				if (k == 9)
					buf->PutInt(716);
				if (k == 10)
					buf->PutInt(344);
				if (k == 11)
					buf->PutInt(300);
				if (k == 12)
					buf->PutInt(637);
				if (k == 13)
					buf->PutInt(512);
				if (k > 13)*/
					buf->PutInt(songlength);

				buf->PutInt(2);
				buf->PutInt(0);
			//	fout << "2\n0\n";
			//	break;
			//TRACE("END OF K LOOP\n");
					
			}
			//TRACE("END OF L LOOP\n");
			}
			if ( j == 2 || j == 6)
					{
						buf->PutByte(0);
						buf->PutInt(random(1000, 80000));
						buf->PutInt(rand() % 10);
				//		fout << "no aviable slots\nupload speed\nnum of people in que\n";
					}
					else
					{
					buf->PutByte(1);
					buf->PutInt(random(1000, 80000));
					buf->PutInt(0);	
			//		fout << "yes available slots\nupload speed\nno one in que\n";
					}

			
	//		fout << endl;
		//	fout << buf->GetCharPtr() << endl;
					/*int index = 0;
					int sl = buf->GetInt(index);
					index += 4;
					fout << "String length = " << sl << endl;
					char* st = new char[sl + 1];
					index += sl;
					strncpy(st, buf->GetCharPtr(index), sl);
					st[sl] = '/0';
					fout << "Username = " << st << endl;
					delete [] st;
					int token = buf->GetInt(index);
					fout << "token = " << token << endl;
					index += 4;
					int nof = buf->GetInt(index);
					fout << "number of files = " << nof << endl;
					index += 4;
					for (int v = 0; v < nof; v++)
					{
						fout << "code = " << buf->GetByte(index) << endl;
						index += 1;
						int sl = buf->GetInt(index);
						index += 4;
						fout << "String length = " << sl << endl;
						st = new char[sl + 1];
						strncpy(st, buf->GetCharPtr(index), sl);
						st[sl] = '\0';
						fout << "string = " << st << endl;
						index += sl;
						delete [] st;
						fout << "size1 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "size2 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "string length = " <<buf->GetInt(index) << endl;
						index += 4;
						st = new char[4];
						strncpy(st, buf->GetCharPtr(index), 3);
						st[3] = '\0';
						fout << "ext = " << st << endl;
						delete [] st;
						index += 3;
                        fout << "Num of attrib = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c0 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c1 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c2 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
					}
					fout << "available = " << buf->GetByte(index) << endl;
					index += 1;
					fout << "upload = " << buf->GetInt(index) << endl;
					index += 4;
					fout << "queue = " << buf->GetInt(index) << endl;*/
			s.m_spoof.push_back(buf);
			//TRACE("END OF J LOOP\n");

		}

		//TRACE("END OF AUDIO CREATING SPOOF\n");
			}

			//TRACE("FINISHED CREATING SPOOFS NOW ADDING TO m_spoofs!\n");
			TRACE("Artist = %s and num of spoofs = %d\n", s.m_artist, s.m_spoof.size());
		//	fout << s.m_spoof
		m_spoofs.push_back(s);
	//	fout << "\n\n";
			
	}

	TRACE("DONE CREATING SPOOFS!\n");
	m_spoofs_created = true;
		
}



CString NoiseSockets::GetFilePath(CString artist, CString album)
{
	CString paths[10] = {"C:\\downloads\\", "C:\\mydownloads\\", "C:\\music\\", "C:\\Documents and Settings\\All Users\\Documents\\My Music\\", "C:\\", "D:\\my music\\", "D:\\downloads\\", "E:\\downloads\\", "D:\\musics\\", "C:\\downloaded\\"};

	CString filepath = paths[rand() % 10];
	filepath.Append(artist);
	filepath.Append("\\");
	if (album.TrimLeft().IsEmpty())
		return filepath;
	
	filepath.Append(album);
	filepath.Append("\\");
//	filepath.Append(track);
//	filepath.Append("mp3");

	return filepath;

}

CString NoiseSockets::getSpoofName(void)
{
	CString username;
//	int temp;

	fillName();
	
	username = names[rand() % 100];
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10); 
	username.AppendFormat("%d", (username.GetLength() % 10));//rand() % 10);
	//username.AppendFormat("%d", rand() % 10);
	//username.AppendFormat("%d", 5);

	names.clear();
	return username;
}

void NoiseSockets::fillName(void)
{
	names.push_back("Jacob");
	names.push_back("Emily");
	names.push_back("Michael");
	names.push_back("Emma");
	names.push_back("Joshua");
	names.push_back("Madison");
	names.push_back("Matthew");
	names.push_back("Hannah");
	names.push_back("Andrew");
	names.push_back("Olivia");
	names.push_back("Joseph");
	names.push_back("Abigail");
	names.push_back("Ethan");
	names.push_back("Alexis");
	names.push_back("Daniel");
	names.push_back("Ashley");
	names.push_back("Christopher");
	names.push_back("Elizabeth");
	names.push_back("Anthony");
	names.push_back("Samantha");
	names.push_back("William");
	names.push_back("Isabella");
	names.push_back("Ryan");
	names.push_back("Sarah");
	names.push_back("Nicholas");
	names.push_back("Grace");
	names.push_back("David");
	names.push_back("Alyssa");
	names.push_back("David");
	names.push_back("Tyler");
	names.push_back("Lauren");
	names.push_back("Alexander");
	names.push_back("Kayla");
	names.push_back("John");
	names.push_back("Brianna");
	names.push_back("James");
	names.push_back("Jessica");
	names.push_back("Dylan");
	names.push_back("Taylor");
	names.push_back("Zachary");
	names.push_back("Sophia");
	names.push_back("Brandon");
	names.push_back("Anna");
	names.push_back("Jonathan");
	names.push_back("Victoria");
	names.push_back("Samuel");
	names.push_back("Natalie");
	names.push_back("Christian");
	names.push_back("Chloe");
	names.push_back("Benjamin");
	names.push_back("Sydney");
	names.push_back("Justin");
	names.push_back("Hailey");
	names.push_back("Nathan");
	names.push_back("Jasmine");
	names.push_back("Jose");
	names.push_back("Rachel");
	names.push_back("Logan");
	names.push_back("Morgan");
	names.push_back("Gabriel");
	names.push_back("Megan");
	names.push_back("Kevin");
	names.push_back("Jennifer");
	names.push_back("Noah");
	names.push_back("Kaitlyn");
	names.push_back("Austin");
	names.push_back("Julia");
	names.push_back("Caleb");
	names.push_back("Haley");
	names.push_back("Robert");
	names.push_back("Mia");
	names.push_back("Thomas");
	names.push_back("Katherine");
	names.push_back("Elijah");
	names.push_back("Destiny");
	names.push_back("Jordan");
	names.push_back("Alexandra");
	names.push_back("Aidan");
	names.push_back("Nicole");
	names.push_back("Cameron");
	names.push_back("Maria");
	names.push_back("Hunter");
	names.push_back("Ava");
	names.push_back("Jason");
	names.push_back("Savannah");
	names.push_back("Angel");
	names.push_back("Brooke");
	names.push_back("Connor");
	names.push_back("Ella");
	names.push_back("Evan");
	names.push_back("Allison");
	names.push_back("Jack");
	names.push_back("MacKenzie");
	names.push_back("Luke");
	names.push_back("Paige");
	names.push_back("Isaac");
	names.push_back("Stephanie");
	names.push_back("Aaron");
	names.push_back("Jordan");
	names.push_back("Isaiah");
	names.push_back("Kylie");
}

int NoiseSockets::random(int lowest, int highest)
{
	double range = (highest - lowest) + 1.0;
    return lowest + (int) (range * rand() / RAND_MAX + 1.0);
}

NoiseSockets::NoiseSockets(HWND hwnd,CCriticalSection *connection_data_critical_section)//,vector<ConnectionData> *connection_data,FileSharingManager* fsm)
{
//	p_fs_manager = fsm;
	m_hwnd=hwnd;
	p_connection_data_critical_section=connection_data_critical_section;
//	p_connection_data=connection_data;

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

	p_noise_data_buf=NULL;
	
//	InitNoiseData();
}

//
//
//
NoiseSockets::~NoiseSockets()
{
	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
	}

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
	CTime now=CTime::GetCurrentTime();
	//if(m_time_last_initialized_noise_data.GetDay()!=now.GetDay())
	//{
//		InitNoiseData();
	//}

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
	//CTime today(now.GetYear(),now.GetMonth(),now.GetDay(),0,0,0);	// 12 a.m. today
	//srand((unsigned int)today.GetTime());

	p_noise_data_buf=new unsigned char[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
//	FrameHeader hdr(32,44.1);
	//unsigned int frame_len=hdr.ReturnFrameSize();

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
}*/