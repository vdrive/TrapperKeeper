#include "StdAfx.h"
#include "poster.h"

Poster::Poster(void)
{
}

Poster::~Poster(void)
{
}

bool Poster::Post(string filename, string name, char * torrent, int torrent_len) 
{
	
	string boundry = GetBoundry();
	string header = GetHeader(torrent, boundry);
	string start = GetBeginData(filename, name, boundry);
	string end = GetEndData(filename, name, boundry);
	int post_data_len = (int)start.length() + (int)end.length() + torrent_len + 2;
	char * post_data = new char[post_data_len];
	char * ptr = post_data;
	
	memcpy(ptr, start.c_str(), start.length());
	ptr += start.length();
	memcpy(ptr, torrent, torrent_len);
	ptr += torrent_len;
	memcpy(ptr, "\r\n", 2);
	ptr += 2;
	memcpy(ptr, end.c_str(), end.length());
	ptr += end.length();

	//WriteTorrentFile(filename, post_data, post_data_len);
	unsigned short *body = NULL;
	string ref = GetRefererURL();
	int size = PostData(GetPostURL().c_str(), &body, ref, header, post_data, post_data_len);
	if(body != NULL) delete [] body;
	delete [] post_data;
	return false;
}

string Poster::GetHeader(char * torrent, string boundry) 
{
	
	string header = "Content-Type: multipart/form-data; boundary="+boundry;
	return header;
}

int Poster::GetPostData(char * post_data, char * torrent)
{
	return 0;
}

string Poster::GetFormatedPostData(string boundry, string name, string value)
{
	string data;
	string cr = "\r\n";
	data += "--" + boundry + cr;
	data += "Content-Disposition: form-data; name=\""+name+"\"" + cr;
	data += cr;
	data += value + cr;
	return data;

}

string Poster::GetBoundry() 
{
	srand( (unsigned)time( NULL ) );
	int size = rand() % 5 + 11;
	char boundry[50];
	strcpy(boundry, "---------------------------");
	for(int i = 0; i < size; i++) {
		boundry[i+27] = rand() % 10 + 48;
	}
	boundry[size+27]='\0';
	

	return string(boundry);
        
}

string Poster::GetBeginData(string filename, string name, string boundry)
{
	return "begin";
}

string Poster::GetEndData(string filename, string name, string boundry)
{
	return "end";
}

string Poster::GetPostURL()
{
	return "";
}

string Poster::GetRefererURL()
{
	return "";
}

bool Poster::CreateFile(string filename)
{
	CFile t_file;
	CFileStatus status;
	if( CFile::GetStatus( filename.c_str(), status ) ) {
		TRACE("%s already exits\n", filename.c_str());
		return false;
	} else {
		if(t_file.Open(filename.c_str(),CFile::modeCreate|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return false;
		}
		t_file.Close();
		return true;
	}

}

void Poster::OpenFile(string filename, CFile *t_file)
{

	if(t_file->Open(filename.c_str(),CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}
}

void Poster::CloseFile(CFile *file)
{
	file->Flush();
	file->Close();
}

void Poster::WriteTorrentFile(string filename, char * data, int len)
{
	int index = (int)filename.find(".torrent");
	if(index < 0) {
		filename += ".torrent";
	}
	//CreateFile(m_download_directory);
	filename = "/temp/" + filename;
	if(CreateFile(filename)) {
		WriteFile(filename, data, len);
	}

}

void Poster::WriteFile(string filename, char * buf, int len)
{
	CFile t_file;

	OpenFile(filename, &t_file);
	
	TRACE("Writting %s\n", filename.c_str());
	t_file.Write(buf,len);
	TRACE("Poster::WriteFile Done\n");
//	delete [] buf;
	CloseFile(&t_file);
}
