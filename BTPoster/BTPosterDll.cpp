#include "StdAfx.h"
#include "BTPosterdll.h"
#include "Resource.h"
#include "HTTPConnector.h"
#include "TorrentSpyPoster.h"
#include "ByteNovaPoster.h"
#include "BTGeneratorInterface.h"

//
//
//
BTPosterDll::BTPosterDll(void)
{
}

//
//
//
BTPosterDll::~BTPosterDll(void)
{
}

//
//
//
void BTPosterDll::DllInitialize()
{
	m_dlg.Create(IDD_BTPOSTERDLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTPosterDll::DllUnInitialize()
{

}

//
//
//
void BTPosterDll::DllStart()
{
	BTGeneratorInterface gen;
	//string id = gen.GenerateTorrentFile("Stewie.Griffin.The.Untold.Story.The.Family.Guy.Movie.2005.DVDRip.XviD.torrent", "The Untold Story", "711962133");
	string id = "62";
	if(gen.IsReady(id)) {
		int size = -1;
		string name;
		string filename;
		if(!gen.GetInfo(id, &filename, &name, &size)) {
			TRACE("ERROR\n");
			return;
		};
		if(size == -1) return;
		TRACE("id=%s,filename=%s,name=%s,size=%d\n",id.c_str(),filename.c_str(),name.c_str(),size);
		char * torrent = new char[size+1];
		size = gen.GetTorrentFile(id, torrent);
		//Post(filename, name, torrent, size);
		delete [] torrent;
	}
}

//
//
//
void BTPosterDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTPosterDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void BTPosterDll::SupplySynched(const char* source_ip)
{
}

void BTPosterDll::Post(string filename, string name, char *torrent, int size)
{
	//ByteNovaPoster poster;
	//poster.Post(filename, name, torrent, size);
	TorrentSpyPoster tp;
	tp.Post(filename, name, torrent, size);
}

string BTPosterDll::GetPostInfo(string data)
{

	unsigned short * temp = NULL;
	string url;
	int size = GetData(url.c_str(), &temp, url);
	char * p_data = new char[size];
	for(int i = 0; i < size; i++) p_data[i] = (char)temp[i];
	data = string(p_data);
	delete [] p_data;
	delete [] temp;
	int cur_pos = (int)data.find("<form");
	cur_pos = (int)data.find("<form", cur_pos+1);
	
	if(cur_pos > 0) {
		const char * temp = data.c_str();
		//int len = data.length();
		cur_pos = (int)data.find("<input", cur_pos+1);
		string post_info = GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\"")); 
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\""));
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\""));
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\""));
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\""));
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\""));
		post_info += "=" + GetEncodedString(GetSubstring(data, &cur_pos, "value=\"", "\""));
		cur_pos = (int)data.find("<input", cur_pos+1);
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\"")) + "=";
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\"")) + "=";
		post_info += "&" + GetEncodedString(GetSubstring(data, &cur_pos, "name=\"", "\"")) + "=1";
		//TRACE("%s\n", post_info.c_str());
		return post_info;
	}

	return "";
}

string BTPosterDll::GetSubstring(const string data, int * pos, string startStr, string endStr) 
{
	int cur_pos = *pos;
	const char * temp = data.c_str();
	int start = (int)data.find(startStr, cur_pos);
	if(start < 0) {
		return "";
	}
	start += (int)startStr.length();
	int len = (int)data.find(endStr, start);
	if(len < 0) len = (int)data.length() - start;
	else len -= start;
	cur_pos = start + len;

	*pos = cur_pos;
	//TRACE("%s\n", data.substr(start, len).c_str());
	return data.substr(start, len);
}
