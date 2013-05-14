#include "StdAfx.h"
#include "swarmersource.h"
#include "imagehlp.h"
#include <io.h>
//#include <afxdllx.h>
#include <math.h>
#include "resource.h"
#include "giFThash.h"
#include "giFTmd5.h"

SwarmerSource::SwarmerSource(void)
{
}

SwarmerSource::~SwarmerSource(void)
{
}
void SwarmerSource::DllInitialize()
{
	// Create the dialog
	m_dlg.Create(IDD_KAZAASWARMER_DLG ,CWnd::GetDesktopWindow());
//	m_dlg.Create(IDD_DUMMY_DLG ,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_syncher.InitParent(this);

}
void SwarmerSource::DllUnInitialize()
{

}


void SwarmerSource::DllStart()
{

	//Timer to check downloads
	m_dlg.SetTimer(1,60*60*1000,NULL);
	//m_dlg.SetTimer(1,30*1000,NULL); // Temp for debug short timer

}

void  SwarmerSource::DllShowGUI()
{

	m_dlg.ShowWindow(SW_NORMAL);

	//vector<MusicProject> projects;
	//ReadMusicProjects(&projects);
	//CopyCompletedDownloads();
		//Temp for Testing
	//CloseandRelaunchKazaa();
	//GetFileList();

}

void SwarmerSource::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void SwarmerSource::CopyCompletedDownloads()
{
	char downloaddir[256+1];
	char datastring[256+1];
	char displaydata[1024+1];

	//Will check registry timer here

	DWORD cbData=(DWORD)sizeof(downloaddir);
	//Get shared Dir from registry
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey)==ERROR_SUCCESS)
	{
		RegQueryValueEx(hkey,"DownloadDir",NULL,NULL,(unsigned char *)downloaddir,&cbData);

	}



	RegCloseKey(hkey);
	char filename[256+1];
	_finddata_t data;


	strcpy(datastring,downloaddir);
	strcat(datastring,"\\*.*");

	//Dialog Message
	sprintf(displaydata,"Shared Directory Is: %s",datastring);
	m_dlg.DisplayInfo(displaydata);

	intptr_t handle=_findfirst(datastring,&data);
	//intptr_t handle=_findfirst("C:\\KaZaALite\\My Shared Folder\\*.*",&data);

	// the first occurance will be .
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
			//strcpy(filename,"C:\\KaZaALite\\My Shared Folder\\");
			strcpy(filename,downloaddir);
			strcat(filename,"\\");
			strcat(filename,data.name);

			// diaolg message
			sprintf(displaydata,"Copying: %s",filename);
			m_dlg.DisplayInfo(displaydata);

			MakeSureDirectoryPathExists(filename);
			DeflateFile(filename);
			remove(filename);  // don't remove it when debugging
		}
	}
	_findclose(handle);

//	m_syncher.RescanSource("Swarmer");
	m_syncher.RescanSource("Swarmer.new");
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
	if (urlreturn <=(HINSTANCE) 32)
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
	strcpy(outputfilename,"C:\\syncher\\src\\Swarmer.new\\deflated_");
	strcat(outputfilename,filename);

	char * ptr = strstr(outputfilename,"_");
	ptr++;

	// If the filename has a number at the front "(122)" get rid of it
	if (*ptr == '(')
	{
		ptr = strstr(filename,")");
		ptr++;
		strcpy(outputfilename,"C:\\syncher\\src\\Swarmer.new\\deflated_");
		strcat(outputfilename,ptr);
	}

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
		strcat(outputfilename," (1)");
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
	return true;
}

bool SwarmerSource::GetFileList()
{

	bool good_data = false;
	/*
	vector<MusicProject> projects;
	ReadMusicProjects(&projects);
	*/
	v_file_list.clear();
	good_data = m_db.GetData(&v_file_list,1500,500);
	if (good_data == false)
	{
		return false;
	}	
	int numrows = (int)v_file_list.size();

	ReadMapsFile();
	v_makefile_list.clear();
	for (int i = 0; i < numrows; i ++)
	{
		//Compare here and either save and leave or save and make sig2dat;
		if ((CompareHashWithMaps(v_file_list[i])) == true)
		{
			v_file_list[i].Save();
		}
		else
		{
			// Try and catch every popup
			//ClosePopups();
			v_file_list[i].Save();
			v_makefile_list.push_back(v_file_list[i]);
			//MakeSig2Dat(v_file_list[i].m_filename,v_file_list[i].m_size,v_file_list[i].m_hash);
			//Sleep(1*1000);
			//ClosePopups();
		}
	}

	m_dlg.SetTimer(2,1000,NULL);

	return true;
}

void SwarmerSource::MakeOnTimer()
{
	ClosePopups();
	FileEntry fentry;
	if (v_makefile_list.size() != 0)
	{
		 fentry = v_makefile_list.back();
	}
	else
	{
		FinishMaking();
		m_dlg.KillTimer(2);
		return;
	}
	MakeSig2Dat(fentry.m_filename,fentry.m_size,fentry.m_hash);
	ClosePopups();
	v_makefile_list.pop_back();
	if (v_makefile_list.size() == 0)
	{
		FinishMaking();
		m_dlg.KillTimer(2);
	}
}

void SwarmerSource::FinishMaking()
{
	DeleteLeftoverFiles();

	ClosePopups();
	CloseandRelaunchKazaa();
}


void SwarmerSource::DeleteLeftoverFiles()
{

	char displaydata[1024+1];

	strcpy(displaydata,"Checking for file Deletion");
	m_dlg.DisplayInfo(displaydata);


	vector<FileEntry> oldfiles;
	FileEntry cur_entry;
	_finddata_t data;

	char filename[256];
	// first two found will be . and ..
	intptr_t handle = _findfirst("C:\\syncher\\src\\Swarmer.New\\*.*",&data);
	_findnext(handle,&data);

	while(_findnext(handle,&data)==0)
	{
		strcpy(filename,"C:\\syncher\\src\\Swarmer.New\\");
		strcat(filename,data.name);
	
		sprintf(displaydata,"Checking: %s",filename);
		m_dlg.DisplayInfo(displaydata);

		strcpy(cur_entry.m_filename,data.name);
		CalculateHash(filename,cur_entry.m_hash);
		cur_entry.m_size = data.size;

		oldfiles.push_back(cur_entry);
	}
	_findclose(handle);


	for (int i = 0; i < (int)v_file_list.size(); i++)
	{

		sprintf(displaydata,"Comparing: %s",v_file_list[i].m_filename);
		m_dlg.DisplayInfo(displaydata);

		vector<FileEntry>::iterator file_iter = oldfiles.begin();
		while (file_iter != oldfiles.end())
		{
			//Debugging only check
			char hash1[64];
			char hash2[64];
			strcpy(hash1,v_file_list[i].m_hash);
			strcpy(hash2,file_iter->m_hash);
			if ((strcmp(v_file_list[i].m_hash,file_iter->m_hash)) == 0)
			{
				oldfiles.erase(file_iter);
			}
			file_iter++;
		}
	}
	char deletefilename[256+1];

	for (int j = 0; j < (int)oldfiles.size(); j++)
	{
		strcpy(deletefilename,"C:\\syncher\\src\\Swarmer.new\\");
		strcat(deletefilename,oldfiles[j].m_filename);
		remove(deletefilename);
		sprintf(displaydata,"Removing: %s",deletefilename);
		m_dlg.DisplayInfo(displaydata);

	}
/*
	char deletefilename[256+1];

	for (int i = 0; i < (int)old_file_list.size(); i++)
	{
		strcpy(deletefilename,"C:\\syncher\\src\\Swarmer.new\\deflated_");
		strcat(deletefilename,old_file_list[i].m_filename);
		remove(deletefilename);
	}

*/
}


bool SwarmerSource::CompareHashWithMaps(FileEntry entry)
{
	vector<FileEntry>::iterator file_iter = old_file_list.begin();
	while (file_iter != old_file_list.end())
	{
		if ((strcmp(entry.m_hash,file_iter->m_hash)) == 0)
		{
			old_file_list.erase(file_iter);
			return true;
		}
		file_iter++;
	}
	return false;
}

void SwarmerSource::ReadMapsFile()
{
	CFile list;
	FileEntry fa;

	old_file_list.clear();

	//  read each line of the file associations and place it onto the  vector.
	if( list.Open("swarmer_source_maps.dat", CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) == TRUE )
	{
		while(list.Read(&fa, sizeof(fa)) != 0)
		{
			old_file_list.push_back(fa);

		}
		list.Close();
	}

	remove("swarmer_source_maps.dat");
}

// Obsolete function to read projects, projects now come from the database
void SwarmerSource::ReadMusicProjects(vector<MusicProject> *projects)
{
	
	CStdioFile projectfile;
	if (projectfile.Open("swarmerprojects.dat",CFile::modeRead|CFile::typeText)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}

	char buf[2048+1];

	char * ptr;
	char * curptr;
	char intbuf[10+1];
	MusicProject tempproject;

	while (projectfile.ReadString(buf,sizeof(buf)-1)!=FALSE)
	{
		ptr = strstr(buf,"\t");
		*ptr = '\0';
		strcpy(tempproject.m_projectname,buf);
		ptr++;
		curptr = ptr;
		ptr = strstr(ptr,"\t");
		*ptr = '\0';
		strcpy(intbuf,curptr);
		tempproject.m_track = atoi(intbuf);
		ptr++;
		curptr = ptr;
		ptr = strstr(ptr,"\t");
		*ptr = '\0';
		strcpy(intbuf,curptr);
		tempproject.m_minswarms = atoi(intbuf);
		ptr++;
		curptr = ptr;
		strcpy(intbuf,curptr);
		tempproject.m_maxswarms = atoi(intbuf);
		projects->push_back(tempproject);
	}


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

void SwarmerSource::ClosePopups()
{
	// Message box with title "Kazaa Lite" that has an OK button as the first child
	HWND hwnd=FindWindow("#32770","Kazaa Lite");
	if(hwnd!=NULL)
	{
		HWND child=GetWindow(hwnd,GW_CHILD);
		if(child!=NULL)
		{
			char name[1024];
			GetClassName(child,name,sizeof(name));
			if(strcmp(name,"Button")==0)
			{
				GetWindowText(child,name,sizeof(name));
				if(strcmp(name,"OK")==0)
				{
					// Click OK
					Click(child);
				}
			}
		}
	}
}

void SwarmerSource::Click(HWND hwnd)
{
	// Find the center point of the HWND to click on
	CRect rect;
	::GetWindowRect(hwnd,&rect);
	LPARAM center=MAKELPARAM(rect.Width()/2,rect.Height()/2);

	::PostMessage(hwnd,WM_LBUTTONDOWN,(WPARAM)MK_LBUTTON,(LPARAM)center);
	::PostMessage(hwnd,WM_LBUTTONUP,(WPARAM)0,(LPARAM)center);
}

void SwarmerSource::CalculateHash(char * outputfilename, char * hashin)
{
		CFile outfile;

		unsigned char hash[20];

	


		if(outfile.Open(outputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
		{
			return;
		}
		// Read in first 300K
		unsigned int filesize;
		unsigned char md5_buf[300*1024];
		unsigned int num_read=outfile.Read(&filesize,sizeof(unsigned int));
		num_read=outfile.Read(md5_buf,sizeof(md5_buf));
		MD5Context md5;
		MD5Init(&md5);
		MD5Update(&md5,&md5_buf[0],num_read);
		MD5Final(hash,&md5);	// first 16 bytes of that hash

// Modified Stolen code, modified to accept deflated files.

		// Calculate the 4-byte small hash.
	    unsigned int smallhash = 0xffffffff;
		int chunk_size=300*1024;
		unsigned char md5buf[300*1024];

		if(outfile.GetLength() > chunk_size + 4)
		{/*
			//size_t offset = 0x100000;	// 1 MB
			size_t lastpos = chunk_size;	// 300K
			size_t endlen;
			while(lastpos + chunk_size  < (outfile.GetLength()-4)) 
			{
/*			
				if (fseek(fp, offset, SEEK_SET) < 0 || fread(filebuf, 1, chunk_size, fp) < chunk_size) 
				{
					free(filebuf);	
					return -1;
			    }
* /
				//outfile.Seek(offset,CFile::begin);
				outfile.Read(md5buf,chunk_size);
	*/   
			num_read=0;
			while (num_read=outfile.Read(md5buf,chunk_size))
			{
				if((outfile.GetLength()-outfile.GetPosition())<chunk_size)
				{
					break;
				}
/*
				if((num_read<chunk_size) && (num_read>0))
				{
					outfile.Seek(0 - chunk_size,CFile::end);
					num_read = outfile.Read(md5buf,chunk_size);
					//break;
				}
*/
				smallhash = hashSmallHash(md5buf, num_read, smallhash);
				//lastpos += chunk_size;
				//offset <<= 1;
			}

			// Do the tail chunk
			outfile.Seek(0-chunk_size,CFile::end);
			num_read=outfile.Read(md5buf,chunk_size);
			smallhash = hashSmallHash(md5buf, num_read, smallhash);

/*			// Subtract 4 because we added the integer at the beginning
			endlen = (size_t)outfile.GetLength() -4  - lastpos;
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
* /			
			outfile.Seek( 0 - endlen,CFile::end);
			outfile.Read(md5buf,(UINT)endlen);

			smallhash = hashSmallHash(md5buf, endlen, smallhash);
			*/
		}

		smallhash ^= filesize;
		hash[16] = smallhash & 0xff;
		hash[17] = (smallhash >> 8) & 0xff;
		hash[18] = (smallhash >> 16) & 0xff;
		hash[19] = (smallhash >> 24) & 0xff;
		//Stolen from Zonie end
		char temphash[40+1];
		for (int i = 0; i <20; i++)
		{
			sprintf((char *)&temphash[i*2],"%02x",((int)hash[i])&0xFF);
		}
		temphash[40] = '\0';
		strcpy(hashin,temphash);
		int test = (int)strlen(hashin);


}

void SwarmerSource::SetRegistryTimer()
{
	CHAR reg_sub_key[] = TEXT("Software\\Swarmer\\TimeStamp");
	HKEY hkey;

	//if (RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey)==ERROR_SUCCESS)
	if (RegCreateKeyEx(HKEY_CURRENT_USER,reg_sub_key,0,NULL,REG_OPTION_NON_VOLATILE,KEY_QUERY_VALUE|KEY_SET_VALUE,NULL,&hkey,NULL)==ERROR_SUCCESS)
	{
		//RegQueryValueEx(hkey,"DownloadDir",NULL,NULL,(unsigned char *)downloaddir,&cbData);

	}
}

void SwarmerSource::CheckRegistryTimer()
{

}