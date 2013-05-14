#include "StdAfx.h"
#include "SwarmerDownloader.h"
#include <io.h>
#include "imagehlp.h"
#include <math.h>
#include "../KazaaSwarmerDistributedSource/SwarmerComHeader.h"
#include "FileEntry.h"
#include "gifthash.h"
#include "giftmd5.h"

//
//
//
SwarmerDownloader::SwarmerDownloader(void)
{
}

//
//
//
SwarmerDownloader::~SwarmerDownloader(void)
{
}

//
//
//
void SwarmerDownloader::DllInitialize()
{

	srand((unsigned int)time(NULL));
	m_dlg.Create(IDD_DOWNLOADER_DLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);

	m_kazaa_manager.InitParent(this);
	m_datfile_manager.InitParent(this);
	strcpy(m_sourceip,"\0");
	
}

//
//
//
void SwarmerDownloader::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}

}

//
//
//
void SwarmerDownloader::DllStart()
{

	

	m_datfile_manager.ReadCurrentDatfile();
	m_datfile_manager.ReadWaitingDatfile();

	m_dlg.SetTimer(1,30*60*1000,NULL);
	// Debug
	//m_dlg.SetTimer(1,10*1000,NULL);

	p_com = new SwarmerDownloaderComInterface();

	p_com->InitParent(this);

	p_com->Register(this,45);
	
	char dlgdata[5000];

	strcpy(dlgdata,"Dll Started Successfully");
	DisplayInfo(dlgdata);

}

//
//
//
void SwarmerDownloader::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	/*
	m_datfile_manager.CreateNewDatfile("puddle of mud - she hates me (1).mp3 ","a70b51611a765bc2677452a20abf8f3acc9a4a4b",3465845);
	m_datfile_manager.CreateNewDatfile("blues traveler - run around (1).mp3","c7fd7ee8d445c79c37843092ab493889d09525c8",4479139);
	m_datfile_manager.CreateNewDatfile("wallflowers - one headlight.mp3","8d94490ff7dfe5035fa70c55b6d5ea10d66a772c",5002553);
	m_datfile_manager.CreateNewDatfile("bee gees - night fever.mp3","bae9161c21c90ef80dbf44bf6388a307c7511cd4",3413472);
	m_datfile_manager.CreateNewDatfile("chumbawamba - tubthumping.mp3","04ca6c1323163b4c86ab500838178a744753526a",3397718);
	m_datfile_manager.CreateNewDatfile("daniel beningfield-if your not the one.mp3","a14463e2b338cb1396219efa6a9830809838ab0d",3122638);
	m_datfile_manager.CreateNewDatfile("puddle of mud 05. blurry.mp3","3d925dd327f2eb74508698dc165ea04bb5aa0392",4863501);
	m_datfile_manager.CreateNewDatfile("melissa etheridge - come to my window.mp3","96494b8de4a845ba74dc4c41ef02731eca85ae36",3800502);
	m_datfile_manager.CreateNewDatfile("bedingfield daniel-if you are not the one.mp3","f308ffed233e53f8a6e659cbfaa34bed38baf2da",3905827);
	m_datfile_manager.CreateNewDatfile("elton john - tiny dancer (3).mp3","f9f93b6b48d0c1d47f5945c891c796b87de118e0",6002857);
	m_datfile_manager.CreateNewDatfile("01- supertramp - give a little bit_mp3 (1).mp3","63193ebe1827147c7152d0add3c027987240e100",3005544);
	DoneReceiving();
	*/

//	Testing!
//	m_datfile_manager.RefreshDownloadFiles();

	char dlgdata[5000];

	strcpy(dlgdata,"GUI display started");
	DisplayInfo(dlgdata);

}

//
//
//
void SwarmerDownloader::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	SwarmerComHeader* header = (SwarmerComHeader*)data;
	byte * data_received =(byte *)data;
	data_received += sizeof(SwarmerComHeader);
	char dlgdata[5000];

	strcpy(dlgdata,"Got Data!");
	strcpy(m_sourceip,source_name);
	RecordSourceIp(source_name);
	DisplayInfo(dlgdata);
	
	switch(header->op)
	{
		case(SwarmerComHeader::Project_Record):
			{
				FileEntry *entry;
				entry = (FileEntry *)data_received;

				sprintf(dlgdata,"Received Project Record %s, %i",entry->m_filename,entry->m_size);
				DisplayInfo(dlgdata);
				m_datfile_manager.CreateNewDatfile(entry->m_filename,entry->m_hash,entry->m_size);
				break;
			}

		case(SwarmerComHeader::Records_Complete):
			{
				strcpy(dlgdata,"Received Records Complete");
				DisplayInfo(dlgdata);

				DoneReceiving();
				
			}

		default:
			{
				DisplayInfo("Received Unknown Data");


				break;
		
			}

	}

}

void SwarmerDownloader::DisplayInfo(char * info)
{
	m_dlg.DisplayInfo(info);

}


void SwarmerDownloader::KillKazaa()
{
	m_kazaa_manager.KillKazaa();
}

void SwarmerDownloader::LaunchKazaa()
{
	m_kazaa_manager.LaunchKazaa();
}

void SwarmerDownloader::DoneReceiving()
{
	m_datfile_manager.WriteCurrentDatfile();
	m_datfile_manager.WriteWaitingDatfile();
	m_kazaa_manager.CloseandRelaunchKazaa();

}

bool SwarmerDownloader::DeflateFile(char * filepath, DatfileRecord record)
{
	CFile inputfile;
	CFile outputfile;
	CString filename;
	char outputfilename[256+1];
	int inputfilesize;
	char dlgdata[5000];

	sprintf(dlgdata,"Deflating %s, with recordname %s from dat %s",filepath,record.m_filename,record.m_datfilename);
	DisplayInfo(dlgdata);

	// Open the input
	if (inputfile.Open(filepath,CFile::modeRead|CFile::typeBinary|CFile::modeNoTruncate|CFile::shareDenyNone) == 0)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	// Create the output file name from the input file name
	filename = inputfile.GetFileName();	
	strcpy(outputfilename,"C:\\syncher\\src\\Swarmer.new\\deflated_");
	strcat(outputfilename,filename);

	char * ptr = strstr(outputfilename,"_");
	ptr++;

	// If the filename has a number at the front "(122)" get rid of it
	/*
	if (*ptr == '(')
	{
		ptr = strstr(filename,")");
		ptr++;
		strcpy(outputfilename,"C:\\syncher\\src\\Swarmer.new\\deflated_");
		strcat(outputfilename,ptr);
	}
*/
	// Open the file to write to
	MakeSureDirectoryPathExists(outputfilename);

	_finddata_t data;
	long handle=(long)_findfirst(outputfilename,&data);

	// If the file is already there add a (1) to the end so it looks like Kazaa did it
	while (handle != -1)
	{
		char endbuf[256+1];
		char * ptr = strrchr(outputfilename,'.');
		strcpy(endbuf,ptr);
		*ptr = '\0';
		char endstr[6];
		sprintf(endstr," (%i)",(rand()%9));
		strcat(outputfilename,endstr);
		strcat(outputfilename,endbuf);
		_findclose(handle);
		handle=(long)_findfirst(outputfilename,&data);
	}

	if (outputfile.Open(outputfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeNoTruncate|CFile::modeCreate|CFile::shareDenyWrite) == 0 )
	{
		DWORD damnerror = ::GetLastError();
		inputfile.Close();
		return false;
	}

	inputfilesize = (int)inputfile.GetLength();

	char buf300k[300*1024];
	// Read the first 300k
	if (inputfile.Read(buf300k,(300*1024))< (300*1024))
	{
		inputfile.Close();
		outputfile.Close();
		remove(filepath);
		remove(outputfilename);
		return false;
	}
	outputfile.Write(&inputfilesize,sizeof(inputfilesize));

	outputfile.Write(&record,sizeof(DatfileRecord));
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

	 	// save the last good position for calculations later
		lastpos = pos;
		//Write the file
		numread = inputfile.Read(buf300k,(300*1024));
		if (numread == 300*1024)
		{
			outputfile.Write(buf300k,(300*1024));
		}
		else
		{
			specialcase = 1;
			break;
		}
		// if the filesize falls between the range of 2^i +300k and 2^i +600k there will be overlap
		if (inputfile.GetLength() - 300*1024 < lastpos + 300*1024)
		{
			specialcase = 2;
			break;
		}

	}


	// Here will be the logic to take care of cases where the last 300 k falls inside a written block

	switch (specialcase)
	{
		// normalcase
		case 0:
		{
			// Seek to the beginning of the last 300k
			try
			{
				pos = (int)inputfile.Seek(-300*1024,CFile::end);
			}
			catch(CFileException *ex)
			{
				ex->Delete();
				break;
			}
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
	inputfile.Close();
	outputfile.Close();


	SendFile(outputfilename);
	// Get rid of the original and the deflated file after it is sent

	remove(filepath);
	remove(outputfilename);
	return true;
}

void SwarmerDownloader::TimerFired()
{
	vector<string>  filenames;
	m_kazaa_manager.KillKazaa();
	m_datfile_manager.ExploreFolder("C:\\FastTrack Shared\\",&filenames);
	char tempname[256+1];
	char hashname[256+1];
	char hash[40+1];
	hash[40] = '\0';
	DatfileRecord temprecord;

	char dlgdata[5000];



	char * ptr;
	for(int i = 0; i < (int)filenames.size(); i++)
	{
		strcpy(tempname,filenames[i].c_str());
		sprintf(dlgdata,"filename : %s",tempname);
		DisplayInfo(dlgdata);
		ptr = strstr(tempname,".dat");
		if (ptr == NULL)
		{
			strcpy(hashname,tempname);
			CalculateHash(tempname,hash);
	
			temprecord = m_datfile_manager.GetRecordFromHash(hash);
			if (temprecord.m_size == 0)
			{
				temprecord = m_datfile_manager.GetRecordFromFilename(hashname);
			}
			// Deflate the File with the record.
			sprintf(dlgdata,"Preparing to deflate : %s",hashname);
			DisplayInfo(dlgdata);
			DeflateFile(hashname,temprecord);

			sprintf(dlgdata,"Deflating hashname %s, with hash %s",hashname,hash);
			DisplayInfo(dlgdata);
			// Send the record
		}
	}

	m_datfile_manager.RefreshDownloadFiles();


}

void SwarmerDownloader::CalculateHash(char * inputfilename, char * hashin)
{


	CFile file_in;
	if(file_in.Open(inputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return;
	}

	unsigned char hash[20];

		// Read in first 300K
	unsigned char md5_buf[300*1024];
	unsigned int num_read=file_in.Read(md5_buf,sizeof(md5_buf));

	MD5Context md5;
	MD5Init(&md5);
	MD5Update(&md5,&md5_buf[0],num_read);
	MD5Final(hash,&md5);	// first 16 bytes of that hash
	
	// Stolen code, begin

		// Calculate the 4-byte small hash.
	unsigned int smallhash = 0xffffffff;
	unsigned int chunk_size=300*1024;
	unsigned char buf[300*1024];

	if(file_in.GetLength() > chunk_size)
	{
		size_t offset = 0x100000;	// 1 MB
		size_t lastpos = chunk_size;	// 300K
		size_t endlen;
		while(offset+2*chunk_size < file_in.GetLength()) 
		{
/*			
				if (fseek(fp, offset, SEEK_SET) < 0 || fread(filebuf, 1, chunk_size, fp) < chunk_size) 
				{
					free(filebuf);	
					return -1;
			    }
*/
			file_in.Seek(offset,CFile::begin);
			file_in.Read(buf,chunk_size);
	    
			smallhash = hashSmallHash(buf, chunk_size, smallhash);
			lastpos = offset+chunk_size;
			offset <<= 1;
		}

		endlen = (int)file_in.GetLength() - lastpos;
		if (endlen > chunk_size) 
		{
			endlen = chunk_size;
		}
/*
			if (fseek(fp, fdp->filesize - endlen, SEEK_SET) < 0 ||	fread(filebuf, 1, endlen, fp) < endlen) 
			{
				free(filebuf);
				return -1;
			}
*/			
		file_in.Seek(file_in.GetLength() - endlen,CFile::begin);
		file_in.Read(buf,(int)endlen);

		smallhash = hashSmallHash(buf, endlen, smallhash);
	}

	smallhash ^= file_in.GetLength();
	hash[16] = smallhash & 0xff;
	hash[17] = (smallhash >> 8) & 0xff;
	hash[18] = (smallhash >> 16) & 0xff;
	hash[19] = (smallhash >> 24) & 0xff;

// Stolen code - end

		// Print the hash out
	memset(buf,0,sizeof(buf));
	for(int i=0;i<sizeof(hash);i++)
	{
		sprintf((char *)&buf[i*2],"%02x",((int)hash[i])&0xFF);
	}

	memcpy(hashin,buf,40);
	file_in.Close();
	

}

bool SwarmerDownloader::SendFile(char * filename)
{

	CFile file;

	if (file.Open(filename,CFile::modeRead|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return FALSE;
	}

	int size = (int)file.GetLength();

	byte* buf;
	UINT length = sizeof(SwarmerComHeader) + size;
	buf = new byte[length]; 
	SwarmerComHeader * header = (SwarmerComHeader*)buf;
	header->op = SwarmerComHeader::File_Sent;
	header->size = length - sizeof(SwarmerComHeader);

	byte * ptr = &buf[sizeof(SwarmerComHeader)];
		
	//memcpy(ptr,&v_makefile_list[0],sizeof(FileEntry));
	// Read in the File here
	file.Read(ptr,size);
	if (m_sourceip[0] == '\0')
	{
		ReadSourceIp(m_sourceip);
	}
	p_com->SendReliableData(m_sourceip,buf,length);
	
	delete[] buf;
	file.Close();

	return TRUE;

}

void SwarmerDownloader::RecordSourceIp(char * sourceip)
{
	CFile datfile;
	if (datfile.Open("sourceip.txt",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}
	datfile.Write(sourceip, (int)strlen(sourceip));


	datfile.Close();
}

void SwarmerDownloader::ReadSourceIp(char * sourceip)
{
	CFile datfile;
	if (datfile.Open("sourceip.txt",CFile::modeRead|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}
	datfile.Read(sourceip, (int)datfile.GetLength());


	datfile.Close();
}