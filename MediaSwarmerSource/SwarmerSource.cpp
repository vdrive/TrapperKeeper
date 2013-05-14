#include "StdAfx.h"
#include "swarmersource.h"
#include "imagehlp.h"
#include <io.h>
#include <afxdllx.h>
#include <math.h>
#include "resource.h"


SwarmerSource::SwarmerSource(void)
{
}

SwarmerSource::~SwarmerSource(void)
{
}
void SwarmerSource::DllInitialize()
{
	// Create the dialog
//	m_dlg.Create(IDD_MEDIASWARMER_DLG ,CWnd::GetDesktopWindow());
	m_dlg.Create(IDD_DUMMY_DLG ,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);

}
void SwarmerSource::DllUnInitialize()
{

}


void SwarmerSource::DllStart()
{

	//Timer to check downloads
//	m_dlg.SetTimer(1,60*60*1000,NULL);
	m_dlg.SetTimer(1,5*1000,NULL); // Temp for debug short timer

}

void  SwarmerSource::DllShowGUI()
{

	m_dlg.ShowWindow(SW_NORMAL);
		//Temp for Testing
	CloseandRelaunchKazaa();
	//GetFileList();

}

void SwarmerSource::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void SwarmerSource::CopyCompletedDownloads()
{

	char filename[256+1];
	_finddata_t data;

	// the first occurance will be .
	intptr_t handle=_findfirst("C:\\FastTrack Shared\\*.*",&data);

	if(handle==-1)
	{
		return;
	}
/*
	if((strstr(data.name,".dat")== NULL) && !(data.attrib & _A_SUBDIR))
	{
		strcpy(filename,"C:\\Syncher\\Src\\Swarmer\\");
		strcat(filename,data.name);
		MakeSureDirectoryPathExists(filename);
		DeflateFile(filename);
		remove(data.name);
	}
*/
	//pull out .. as well 
	_findnext(handle,&data);

	while(_findnext(handle,&data)==0)
	{
		// it's not a datfile and not a directory so it is a target
		if((strstr(data.name,".dat")== NULL)&& !(data.attrib & _A_SUBDIR))
		{
			strcpy(filename,"C:\\FastTrack Shared\\");
			strcat(filename,data.name);
			MakeSureDirectoryPathExists(filename);
			DeflateFile(filename);
			//remove(filename);  // don't remove it when debugging
		}
	}
	_findclose(handle);
}

bool SwarmerSource::MakeSig2Dat(char * filename, int bytes, char hash[40])
{
// sig2dat://|File: (smr)blade2-ts-(1of2).avi|Length: 206441472 Bytes, 201603KB|UUHash: =iDTwSiS9VmcrX41U85KPBqK0KlU=|"} (smr)blade2-ts-(1of2).avi

	HINSTANCE urlreturn;
	int kbytes;
	kbytes = bytes/1024;
	char url[1024];


	// Convert the ascii hash to binary
	char ascii_hash[40+1];
	unsigned char bin_hash[20];
	memset(bin_hash,0,sizeof(bin_hash));
	strcpy(ascii_hash,hash);
	for(int k=0;k<20;k++)
	{
		char tmp[3];
		tmp[0]=ascii_hash[2*k];
		tmp[1]=ascii_hash[2*k+1];
		tmp[2]='\0';

		unsigned int c;
		sscanf(tmp,"%x",&c);
		bin_hash[k]=(unsigned char)c;
	}

	// Init lookup table
	char lookup[64];
	memset(lookup,0,sizeof(lookup));
	for(k=0;k<26;k++)
	{
		lookup[k]='A'+k;
		lookup[k+26]='a'+k;
	}
	for(k=0;k<10;k++)
	{
		lookup[k+26+26]='0'+k;
	}
	lookup[26+26+10]='+';
	lookup[26+26+10+1]='/';

	// Convert the binary hash to the uuhash
	CString uuhash="=";
	for(k=0;k<sizeof(bin_hash);k+=3)
	{
		uuhash+=lookup[(bin_hash[k]>>2)&0x3F];
		uuhash+=lookup[((bin_hash[k+1]>>4)&0x0F)+((bin_hash[k]<<4)&0x30)];
		
		if(k<sizeof(bin_hash)-3)
		{
			uuhash+=lookup[((bin_hash[k+1]<<2)&0x3C)+((bin_hash[k+2]>>6)&0x03)];
			uuhash+=lookup[bin_hash[k+2]&0x3F];
		}
		else
		{
			uuhash+=lookup[((bin_hash[k+1]<<2)&0x3C)];
			uuhash+='=';
		}
	}


	sprintf(url, "sig2dat:///|File: %s|Length: %d Bytes, %dKB|UUHash: %s|\" %s",filename, bytes, kbytes, uuhash, filename);


	urlreturn = ShellExecute(NULL,"open",url,NULL,NULL,SW_SHOWNORMAL);
	if ((int)urlreturn <= 32)
	{	
		return false;
	}

	return true;
}


bool SwarmerSource::DeflateFile(char * filepath)
{
	CFile inputfile;
	CFile outputfile;
	CString filename;
	char outputfilename[256+1];
	int inputfilesize;

	// Open the input
	if (inputfile.Open(filepath,CFile::modeRead|CFile::typeBinary|CFile::modeNoTruncate|CFile::shareDenyNone) == 0)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	// Create the output file name from the input file name
	filename = inputfile.GetFileName();	
	strcpy(outputfilename,"C:\\syncher\\src\\Swarmer\\deflated_");
	strcat(outputfilename,filename);

	// Open the file to write to
	MakeSureDirectoryPathExists(outputfilename);
	if (outputfile.Open(outputfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeNoTruncate|CFile::modeCreate|CFile::shareDenyWrite) == 0 )
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	inputfilesize = (int)inputfile.GetLength();

	char buf300k[300*1024];

	// Read the first 300k
	outputfile.Write(&inputfilesize,sizeof(inputfilesize));
	inputfile.Read(buf300k,(300*1024));
	outputfile.Write(buf300k,(300*1024));

	int pos = 0;
	int lastpos = 0 ; // last good position
	int numread = 0;
	int specialcase = 0;

	// save space in the loop to do up to a terabyte 
	// why? because files will never be that large and it will break out of the loop at the end
	for (int i = 0; i <30; i ++)
	{	
		int mult;
		// Try to seek to the 2^ith megabyte
		mult = (int)pow(2,i);
		(pos = (int)inputfile.Seek(mult *1024 *1024,CFile::begin));

		if (pos > inputfilesize)
		{
			break;
		}
		/*
		catch(CFileException *ex)
		{
			ex->Delete();
			break;
		}
		*/
		//i++;  // not sure why this was here maybe messing it up
	 	// save the last good position for calculations later
		lastpos = pos;
		//Write the file

		// if the filesize falls between the range of 2^i +300k and 2^i +600k there will be overlap
		if (inputfile.GetLength() - 300*1024 < lastpos + 300*1024)
		{
			specialcase = 2;
			break;
		}
		numread = inputfile.Read(buf300k,(300*1024));
		if (numread == 300*1024)
		{
			outputfile.Write(buf300k,(300*1024));
		}
		else
		{
			specialcase = 1;
		}
	}


	// Here will be the logic to take care of cases where the last 300 k falls inside a written block

	switch (specialcase)
	{
		// normalcase
		case 0:
		{
			// Seek to the beginning of the last 300k
			pos = (int)inputfile.Seek(-300*1024,CFile::end);
			inputfile.Read(&buf300k,(300*1024));
			outputfile.Write(buf300k,(300*1024));
			break;
		}
		// the filesize was within 2^i + 300k
		// Finish same as case 1 since we decided not to write the partial buffer
		case 1:
		{	// Seek to the beginning of the last 300k
			pos = (int)inputfile.Seek(-300*1024,CFile::end);
			inputfile.Read(&buf300k,(300*1024));
			outputfile.Write(buf300k,(300*1024));
			break;
		}
		// the filesize was between 2^i + 300k and 2^i + 600k
		case 2:
		{
			int numread = 0;
			char bigbuf[2*300*1024];
			
			numread = inputfile.Read(&bigbuf,(2*300*1024));
			outputfile.Write(bigbuf,numread);

//			delete [] bigbuf;
			break;
		}	
	}

//	delete [] buf300k;
	return true;
}

bool SwarmerSource::GetFileList()
{

	m_db.GetData(&file_list,1500);
	int numrows = (int)file_list.size();

	for (int i = 0; i < numrows; i ++)
	{
		MakeSig2Dat(file_list[i].m_filename,file_list[i].m_size,file_list[i].m_hash);
	}
	return true;
}

void SwarmerSource::CloseandRelaunchKazaa()
{

	// Kill the old
	HWND kazaa_hwnd = NULL;
	kazaa_hwnd = ::FindWindow("KaZaA",NULL);
	if(kazaa_hwnd==NULL)
	{
		return;
	}
	::PostMessage(kazaa_hwnd,WM_COMMAND,0x00008066,0);

	// Start the new one
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo,sizeof(processInfo));
	ZeroMemory(&startInfo,sizeof(startInfo));
	startInfo.lpTitle="Kazaa";
	startInfo.wShowWindow=SW_SHOWNORMAL;

	// Rack location
	CString kazaadir = "C:\\KazaaLite\\";
	CString kazaaexe = "C:\\KazaaLite\\kazaa.exe";

	 //Testing string for my computer
	//CString kazaadir = "C:\\Program Files\\KaZaA Lite\\";
	//CString kazaaexe = "C:\\Program Files\\KaZaA Lite\\kazaa.exe";

	int err=CreateProcess(kazaaexe,NULL,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,kazaadir,&startInfo,&processInfo);


}