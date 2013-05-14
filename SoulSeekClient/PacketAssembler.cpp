// PacketAssembler.cpp

#include "stdafx.h"
#include "PacketAssembler.h"
#include "UserNames.h"

#include "SoulSeekClientDll.h"

#include <afxmt.h>	// for CCriticalSection

#include <vector>
#include <time.h>
#include "ZLib.h"
using namespace std;

#include "logger.h"

//
//
//
UINT abcThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	PacketAssemblerThreadData thread_data;

	::PostMessage(hwnd,WM_INIT_THREAD_DATA, (WPARAM)&thread_data, (LPARAM)&critical_section );

	// Wait for events
	DWORD num_events=PACKET_ASSEMBLER_WAIT_EVENTS;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired

	srand ( (unsigned int)time(NULL) );  //seed thread

	Logger log;

	UserNames userInfo;
	userInfo.PopulateDirectoryList();
	userInfo.PopulateUserList();

	//ZLIB Buffer
	unsigned char* compressedText = new unsigned char[1000000];


	while(1)
	{
		event= WaitForMultipleObjects(num_events,&thread_data.m_reserved_events[0],wait_all,timeout);
		
		if(event==WAIT_FAILED)
		{
			::MessageBox(NULL,"WaitForMultipleObjects() Failed","Error",MB_OK);
			break;
		}

		// Check to see if it the triggered event is any of the 4 reserved events

		// Event 0 - Check to see if this is the kill thread event (reserved event 0)
		if(event== 0)
		{
			break;
		}

		// Event 1 - Process Search Results to see if we are watching the project
		//           If we are watching the project, return SearchRequest item with search results to send back
		if(event==1)
		{
            queue<unsigned char *> *pqPackets = NULL;
			{
				CSingleLock singleLock(&critical_section, true);
				pqPackets = thread_data.p_preprocessedPackets;
				thread_data.p_preprocessedPackets = NULL;
				
				ResetEvent(thread_data.m_reserved_events[event]);
			}
			if(pqPackets != NULL)
			{
				while(!pqPackets->empty())
				{
					unsigned char* packet = pqPackets->front();
					pqPackets->pop();

					if( packet == NULL )
						continue;

					bool doDecoy = false;
					if((*(BYTE *) (packet)) == 1)
						doDecoy = true;

					
					int sizeOfIP = *(unsigned int *) (packet+1);
					char* p_ip = new char[sizeOfIP+1];
					memcpy( p_ip, packet+4+1, sizeOfIP);
					p_ip[sizeOfIP] = '\0';

					unsigned int port = *(unsigned int *) (packet+4+sizeOfIP+1);

					//Token
					unsigned int token = *(unsigned int *) (packet+8+sizeOfIP+1);

					//Number of Files
					unsigned int numOfFiles = *(unsigned int *) (packet+12+sizeOfIP+1);

					//File Vector
					unsigned int counter = 0;
					vector<string> mediaNames;
					for(unsigned int i = 0; i < numOfFiles; i++)
					{
						unsigned int mediaSize = *(unsigned int *) (packet+16+sizeOfIP+1+counter);
						if(mediaSize < 128)
						{
							char temp[128];
							memcpy( temp, packet+20+sizeOfIP+1+counter, mediaSize);
							temp[mediaSize] = '\0';
							mediaNames.push_back(string(temp));
						}
						counter += mediaSize+4;
					}

					delete[] packet;

					int numOfReturnUsers = (int)rand()%4+6;
					if(doDecoy) numOfReturnUsers = 1;

					if(mediaNames.size() > 0)
					{
						for(int numSent=0; numSent < numOfReturnUsers;numSent++)
						{
							int numOfFiles;

							if(mediaNames.size() < 4)
								numOfFiles = (int)((rand()%mediaNames.size())+1);
							else
								numOfFiles = (int)((rand()%4)+1);

							unsigned int whatUser = rand();
							unsigned int SongNumberToChoose = (int)(rand() % mediaNames.size());
							unsigned int tempSong = SongNumberToChoose;

							unsigned int filenameSize = 0;

							for(int i = 0; i < numOfFiles; i++)
							{
								filenameSize += (unsigned int)strlen(mediaNames[tempSong % mediaNames.size()].c_str());
								tempSong++;
							}

							unsigned char zlibPacket[50000];
							unsigned int zlibPacketSize = 0;
							unsigned int counter = 0;
							char* userName = new char[128];

							if(doDecoy) {
								_snprintf(userName,128,"%s","jbutter2222");
								//zlibPacketSize = 21+(int)strlen("jbutter2222")+ (int)filenameSize + (numOfFiles * (44+5+userInfo.GetDirectoryList()->at(whatUser % userInfo.GetDirectoryList()->size()).length()));
								//*(unsigned int *)(zlibPacketSize) = (unsigned int)strlen("jbutter2222");
								//memcpy(zlibPacketSize+4,"jbutter2222", strlen("jbutter2222"));  //4 + strlen
							} else {
								_snprintf(userName,128,"%s",userInfo.GetRandomUserName().c_str());
								//zlibPacketSize = 21+(int)userNames->at(whatUser).size()+ (int)filenameSize + (numOfFiles * (44+5+userInfo.GetDirectoryList()->at(whatUser % userInfo.GetDirectoryList()->size()).length()));
								//*(unsigned int *)(zlibPacketSize) = (unsigned int)strlen(userNames->at(whatUser).c_str());
								//memcpy(zlibPacketSize+4,userNames->at(whatUser).c_str(), strlen(userNames->at(whatUser).c_str()));
							}

							zlibPacketSize = 21+(int)strlen(userName)+ (int)filenameSize + (numOfFiles * (44+5+userInfo.GetDirectoryList()->at(whatUser % userInfo.GetDirectoryList()->size()).length()));
							*(unsigned int *)(zlibPacket) = (unsigned int)strlen(userName);
							memcpy(zlibPacket+4,userName, strlen(userName));

							counter = strlen(userName)+4;

							*(unsigned int *)(zlibPacket+counter) = token;
							*(unsigned int *)(zlibPacket+counter+4) = numOfFiles;
							counter += 8;
							//zPacket->AppendInt(sr->GetToken(), false); //8
							//zPacket->AppendInt(numOfFiles, false); //12

							for(int i = 0; i < numOfFiles; i++)
							{
							
								//zPacket->AppendByte(1, false); //1
								*(bool *)(zlibPacket+counter) = (bool)(1);

								char fileLocation[MAX_PATH+1];
								sprintf(fileLocation, "%s\\%s\0", userInfo.GetDirectoryList()->at(whatUser % userInfo.GetDirectoryList()->size()).c_str(),mediaNames[SongNumberToChoose % mediaNames.size()].c_str());
				
								*(unsigned int *)(zlibPacket+counter+1) = (unsigned int)strlen(fileLocation);
								memcpy(zlibPacket+counter+5,fileLocation, strlen(userInfo.GetDirectoryList()->at(whatUser % userInfo.GetDirectoryList()->size()).c_str()) + 1 + strlen(mediaNames[SongNumberToChoose % mediaNames.size()].c_str()));

								counter += 5+strlen(fileLocation);

								unsigned int fileSize = (unsigned int)((rand()/(float)RAND_MAX)*5000000);
								fileSize += 3000000;
								int mod137 = fileSize % 137;
								fileSize = fileSize - mod137;
								if(!doDecoy) 
									*(unsigned int *)(zlibPacket+counter) = (unsigned int)fileSize;//zPacket->AppendInt((int)fileSize,false); //10
								else *(unsigned int *)(zlibPacket+counter) = 6521277; //zPacket->AppendInt(6521277,false);
								//zPacket->AppendInt(0,false);  //14
								*(unsigned int *)(zlibPacket+counter+4)=0;
								//zPacket->AppendSizeAndString("mp3",false); //21
								*(unsigned int *)(zlibPacket+counter+8) = (unsigned int)strlen("mp3");
								memcpy(zlibPacket+counter+12,"mp3", strlen("mp3"));
								counter += 12+strlen("mp3");

								//zPacket->AppendInt(3,false); //25
								*(unsigned int *)(zlibPacket+counter) = 3;
								//zPacket->AppendInt(0,false); //29
								*(unsigned int *)(zlibPacket+counter+4) = 0;

								int bitrate = rand()%10;

								if(bitrate < 7)
									*(unsigned int *)(zlibPacket+counter+8) = 192;//zPacket->AppendInt(192,false); //33
								else if(bitrate == 7)
									*(unsigned int *)(zlibPacket+counter+8) = 164;//zPacket->AppendInt(164,false);
								else
									*(unsigned int *)(zlibPacket+counter+8) = (unsigned int)(rand()%64 + 148);//zPacket->AppendInt(rand()%64 + 148,false);
							
								//zPacket->AppendInt(1,false); //37
								*(unsigned int *)(zlibPacket+counter+12) = 1;
								//zPacket->AppendInt(rand()%120 + 180,false); //41  track length in seconds
								*(unsigned int *)(zlibPacket+counter+16) = (unsigned int)(rand()%120 + 180);
								
								//zPacket->AppendInt(2,false); //45
								*(unsigned int *)(zlibPacket+counter+20) = (unsigned int)2;
								if(bitrate <= 7)
									*(unsigned int *)(zlibPacket+counter+24) = (unsigned int)0;//zPacket->AppendInt(0,false); //49
								else
									*(unsigned int *)(zlibPacket+counter+24) = (unsigned int)1;//zPacket->AppendInt(1,false);

								counter += 28;

								SongNumberToChoose++;
							}  //End files
							
							unsigned int isQueue = rand() % 10;
							
							if(isQueue < 2) *(byte *)(zlibPacket+counter) = (byte)0;
							else *(byte *)(zlibPacket+counter) = (byte)1;
							*(unsigned int *)(zlibPacket+counter+1) = (unsigned int)(rand()%2400+10000);

							unsigned int numInQueue = 0;
							if(isQueue < 2) numInQueue = (rand() % 16) + 1;
							*(unsigned int *)(zlibPacket+counter+5) = (unsigned int) numInQueue; //0

							counter += 9;

							UINT compressedSize = 1000000;

								int result = compress( (Bytef *)compressedText , (uLongf *)&compressedSize,
									(const Bytef *)(zlibPacket), (uLong)(zlibPacketSize) );															
								if( result != Z_OK )
								{
									TRACE( "Compress: Compression error(%i)\n", result);
								}
								else
								{
									PacketWriter* p_packet;
									p_packet = new PacketWriter(4+compressedSize);
									p_packet->p_ip = dupstring(p_ip);
									p_packet->m_port = port;
									p_packet->p_username = userName;
									p_packet->AppendInt(9, false);
									p_packet->AppendData((char *)compressedText,compressedSize);
									
									::PostMessage(hwnd, WM_PA_SEND_RESULTS, (WPARAM)p_packet, NULL);
								}
						}
						delete[] p_ip;
					}
				}
				delete pqPackets;
				pqPackets = NULL;
				
			}
		}
		if(event== 2)
		{
			ResetEvent(thread_data.m_reserved_events[event]);
		}
		if(event== 3)
		{
			ResetEvent(thread_data.m_reserved_events[event]);
		}
		if(event== 4)
		{	
			ResetEvent(thread_data.m_reserved_events[event]);
		}
	}
	if(thread_data.p_preprocessedPackets != NULL)
	{
		while(!thread_data.p_preprocessedPackets->empty())
		{
			delete thread_data.p_preprocessedPackets->front();
			thread_data.p_preprocessedPackets->pop();
		}
		delete thread_data.p_preprocessedPackets;
		thread_data.p_preprocessedPackets = NULL;
	}
	return 0;	// exit the thread
}

//
//
//
PacketAssembler::PacketAssembler()
: p_critical_section(NULL), p_thread_data(NULL), p_parent(NULL), p_thread(NULL)
{
}

void PacketAssembler::Run()
{
	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in PacketAssembler()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	// Start the worker thread...passing it the handle to the message window
	HWND hwnd=m_wnd.GetSafeHwnd();
	p_thread = ::AfxBeginThread(abcThreadProc,(LPVOID)hwnd,THREAD_PRIORITY_BELOW_NORMAL);

}
//
//
//
PacketAssembler::~PacketAssembler()
{
	// Kill the thread if we have received initialized the thread data
	if(p_critical_section!=NULL)
	{
		SetEvent(p_thread_data->m_reserved_events[0]);
	}

	if(p_thread != NULL)
	{
		WaitForSingleObject( p_thread->m_hThread, 2000 );
		p_thread = NULL;
	}

	// Destroy Window
	m_wnd.DestroyWindow();
}

//
//
//
void PacketAssembler::InitParent(SoulSeekClientDll *parent)
{
	p_parent=parent;
}

//
//
//
void PacketAssembler::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section = (CCriticalSection *)lparam;
	p_thread_data = (PacketAssemblerThreadData *)wparam;
}

void PacketAssembler::SetPAEvent(unsigned int theEvent)
{
	SetEvent( p_thread_data->m_reserved_events[theEvent] );
}

void PacketAssembler::ReturnResults(PacketWriter* packet)
{
	//p_parent->m_dlg.WriteToLog(0,0,"Routing Packet");
	p_parent->m_connectionManager.RoutePacket(packet);
}

void PacketAssembler::AssemblePacket(unsigned char* packetdata)
{
	if( packetdata == NULL )
		return;

	if( p_critical_section == NULL )
	{
		p_parent->m_dlg.WriteToLog(0,0,"Criticle Section is not created yet.");
		delete[] packetdata;
		return;
	}

	if(p_critical_section!=NULL)
	{
		CSingleLock singleLock(p_critical_section, true);
		if( p_thread_data->p_preprocessedPackets == NULL )
			p_thread_data->p_preprocessedPackets = new queue<unsigned char *>;

		p_thread_data->p_preprocessedPackets->push( packetdata );
		SetEvent( p_thread_data->m_reserved_events[1] );
	}
}

void PacketAssembler::WriteToLog(char* stringToWrite, int warningLevel)
{
	p_parent->m_dlg.WriteToLog(warningLevel,SEARCH_NUM,stringToWrite);
	delete[] stringToWrite;
}
void PacketAssembler::WriteToLogFile(char* s)
{
	p_parent->WriteToTextFile(s);
}