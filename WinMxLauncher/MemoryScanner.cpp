#include "StdAfx.h"
#include "memoryscanner.h"
#include <Psapi.h>
#define STARTADDRESS 280300
#define MEMORYSCANSIZE 280330-STARTADDRESS


MemoryScanner::MemoryScanner(void)
{
}

MemoryScanner::~MemoryScanner(void)
{
}

UINT MemoryScanner::Run()
{
	TRACE("MemoryScanner::Run()");
/*	time_t curTime;
	time(&curTime);
	srand((unsigned int)curTime);  //seed the random number generater with the current time. (so we generate different random strings for the mutexname each time the program is run)
*/	vector <DWORD> checkedIds;  //a vector of IDs we've already looked at.
	byte *buf=new byte[MEMORYSCANSIZE+1];
	DWORD ids[1000];
	HMODULE modules[1000];
	while(!b_killThread)
	{
		try
		{ //a big ass try statement to handle any exceptions (like the one that occurs if another program crashes and we enumerate its modules
			DWORD count;
			EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until winmx shows up
			count/=(sizeof(DWORD));
			for(int i=0;i<(int)count && !b_killThread;i++)
			{ //for each of the processes
				DWORD nmod;
				
				int b_skip=0;
				for(unsigned int k=0;k<checkedIds.size();k++)
				{  //see if we've checked this id before
					if(checkedIds[k]==ids[i])
					{
						b_skip=1;	
						break;
					}
					else
					{
						//TRACE("instance=%d,saved=%d\n",handle,ids[i]);
					}
				}
				if(b_skip)
				{  //we've already examined this winmx process, so lets not try to change it again
					//TRACE("Skipping winmx Analysis\n");
					continue;
				}
				HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
				EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
				nmod/=(sizeof(HMODULE));

				if(nmod>0)
				{
					char name[100];
					GetModuleBaseName(handle,modules[0],name,99);
					DWORD modaddressx=(DWORD)modules[0];
					
					if(stricmp("WinMx.exe",name)==0)
					{  //if process is named WinMx try to rename its mutex
						//TRACE("Found a new WinMx\n");
						HINSTANCE winmxInstance=(HINSTANCE)handle;
						int b_found=0;
						int lcount=0;
						while(!b_found && lcount++<500 && !b_killThread)
						{  //lcount is just a guard variable
							//TRACE("Scanning WinMx for the %d time\n",lcount);
							DWORD num=0;
							int error=-1;
							
							DWORD stat=0;
							
							DWORD start=STARTADDRESS;
							stat=ReadProcessMemory(winmxInstance,(void*)(modaddressx+start),(void*)buf,MEMORYSCANSIZE,&num);
							if(stat==0)
							{//256+32+11=299=12b  768+224+6=998=3e6 
								error=GetLastError();
								TRACE("ReadProcessMemory Failed!  Error Code = %d\n",error);
							}
							TRACE("num bytes read via ReadProcessMemory = %d\n",num);
			
							byte *ptr=(byte*)buf;
							int b_write=0;
							DWORD memOffset=0;
							
							for(memOffset=0;memOffset<num-10;memOffset++)
							{ //go through memory searching hard and fast.  Memoffset should start as high as possible so it finds it right away.
								if(*ptr==0x0F && *(ptr+1)==0x84 && *(ptr+2)==0x9e && *(ptr+3)==0x00 && *(ptr+4)==0x00 && *(ptr+5)==0x00 && *(ptr+6)==0x8b && *(ptr+7)== 0x50)
								{
									b_found=1;
									b_write=1;
									memOffset=memOffset;
									break;
								}
								ptr++;
							}
							/*
							CFile file;
							byte tempbuf[MEMORYSCANSIZE];
							file.Open("winmx_dump.dat",CFile::modeRead|CFile::typeBinary);
							file.Seek(STARTADDRESS,CFile::begin);
							file.Read(tempbuf,MEMORYSCANSIZE);
							DWORD tnum=MEMORYSCANSIZE;
							file.Close();
							*/
							
							if(b_found && b_write)
							{
								DWORD tnum=6;
								byte ch[6];
								ch[0]=0xE9;
								ch[1]=0x9F;
								ch[2]=0x00;
								ch[3]=0x00;
								ch[4]=0x00;
								ch[5]=0x50;

								TRACE("Writing to winmx's process memory at location %d\n",STARTADDRESS+memOffset);
								WriteProcessMemory(winmxInstance,(void*)(modaddressx+STARTADDRESS+memOffset),ch,6,&tnum);
								FlushInstructionCache(winmxInstance,(void*)(modaddressx+STARTADDRESS+memOffset),6);

								//save this process id, since we've modified it already
								checkedIds.push_back(ids[i]);
							}
							
							/*
							WriteProcessMemory(winmxInstance,(void*)(modaddressx+STARTADDRESS),tempbuf,MEMORYSCANSIZE,&tnum);
							FlushInstructionCache(winmxInstance,(void*)(modaddressx+STARTADDRESS),MEMORYSCANSIZE);
							*/

								//save this process id, since we've modified it already
								checkedIds.push_back(ids[i]);
							
							Sleep(5);
						}
					}
					else
					{
						checkedIds.push_back(ids[i]);//this process isn't named winmx, let us never look at it again.
					}
				}
				CloseHandle(handle);
			}
		}
		catch( char *str )
		{
			TRACE("Caught some arbitrary exception in the MemoryScanner::Run(): %s\n",str);
		}
		Sleep(5);
	}
	delete[] buf;
	return 0;
}
