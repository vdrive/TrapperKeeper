#include "httpconnection.h"
#include "afxwin.h"
#include "btdatacollector.h"
#include "BTScraperModDll.h"
#include "TorrentFileHeader.h"
#include <io.h>
#include "TorrentFileInfo.h"

#include "TBParser.h"
#include "PirateBayParser.h"
#include "TorrentSpyParser.h"
#include "MiniNovaParser.h"
#include "MyBittorrentParser.h"
#include "PirateBayParser.h"
#include "TRParser.h"
#include "TorrentPortalParser.h"
#include "IsoHuntParser.h"



BTDataCollector::BTDataCollector(void)
{
	bt_db="206.161.141.35";
	m_data = NULL;
	done = false;
	m_full_write_time = -1;
	m_dialog = NULL;
	b_started = false;
	m_directory_read = 0;
	m_download_directory = "C:/BTScraperDownloads/";
	m_log_file_name = "BTScraper.log";
	CreateFile(m_log_file_name);
	OpenFile(m_log_file_name, &m_log_file);
	m_log_file.SeekToEnd();
	CreateDirectory(m_download_directory);
}

BTDataCollector::~BTDataCollector(void)
{
	for(unsigned int i = 0; i < v_torrent_list.size(); i++) {
		delete (v_torrent_list[i]);
	}
	v_torrent_list.clear();

	for(unsigned int i = 0; i < v_sent_torrents.size(); i++) {
		delete (v_sent_torrents[i]);
	}
	v_sent_torrents.clear();

	for(unsigned int i = 0; i < v_no_peer_torrents.size(); i++) {
		delete (v_no_peer_torrents[i]);
	}
	v_no_peer_torrents.clear();

	v_send_list.clear();
	CloseFile(&m_log_file);

	//if(m_data != NULL) delete m_data;
	while(m_data != NULL) {
		btData * temp = m_data->GetNext();
		delete m_data;
		m_data = temp;
	}

}

void BTDataCollector::SetWindow(BTScraperDlg  * dialog) 
{
	m_dialog = dialog;
}

void BTDataCollector::Collect() 
{
	char message[64+1];
	if(b_started) return;
	b_started = true;
	Log("BTDataCollector::Collect");
	
	GetSearchStringCollection();	
	UpdateProjectData();
	sprintf(message, "Protecting %d projects", v_project_data.size());

	Log(message);

	//Reading from directory
	Log("BTDataCollector::Reading Directory");
	if (m_directory_read == 0)
	{
		ReadTorrents();
		m_directory_read++;
	}
	else
	{
		m_directory_read++;
	}

	m_data = NULL;

	if(m_directory_read == 1) {

	}

	MyBittorrentParser mb(m_data, &m_dialog->m_main_list_ctrl);
	m_data = mb.GetList();
	Log("MyBittorrent done.. " );

	IsoHuntParser ip(m_data, &m_dialog->m_main_list_ctrl);
	m_data = ip.GetList();
	Log("IsoHunt done.. " );

	TorrentPortalParser tp(m_data, &m_dialog->m_main_list_ctrl);
	m_data = tp.GetList();
	Log("TorrentPortal done.. " );

	TRParser tr(m_data, &m_dialog->m_main_list_ctrl);
	m_data = tr.GetList();
	Log("TorrentReactor done.. " );

	PirateBayParser pb(m_data, &m_dialog->m_main_list_ctrl);
	m_data = pb.GetList();
	Log("PirateBay done.. " );

	MiniNovaParser mp(m_data, &m_dialog->m_main_list_ctrl);
	m_data = mp.GetList();
	Log("MiniNova done.. " );

	TBParser tb(m_data, &m_dialog->m_main_list_ctrl);
	m_data = tb.GetList();
	Log("TorrentBox done.. ");

	TorrentSpyParser ts(m_data, &m_dialog->m_main_list_ctrl);
	m_data = ts.GetList();
	Log("TorrentSpy done.. " );




	// downloads the torrent, parses it, 
	// writes the data to the db
	// TODO:  Split this up better
	Write(m_data);
	//SendData();
	Log("Cleaning up...");

	//delete m_data;
	while(m_data != NULL) {
		btData * temp = m_data->GetNext();
		delete m_data;
		m_data = temp;
	}

	m_data = NULL;

	Log("Sleeping... ");
	done = true;
	b_started = false;
}

bool BTDataCollector::SendReliableData(string dest, void* send_data, UINT data_length, int message_type) 
{
	char buf[1024+1];
	sprintf(buf,"sending to rack %s",(char *)dest.c_str());
	Log(buf, message_type);

	return m_dialog->SendReliableData((char*)dest.c_str(), (void*)send_data, data_length);

}

void BTDataCollector::DeleteOldIPs() 
{
	Log("BTDataCollector::DeleteOldIPs Start");
	MYSQL conn;

	mysql_init(&conn);
	if (mysql_real_connect(&conn,bt_db.c_str(),"onsystems","ebertsux37","overnet_data",0,NULL,0) == NULL)
	{
		TRACE("(Failed to connect to database: %s\n", mysql_error(&conn));
		return ;
	}

	int ret = mysql_query(&conn, "delete from overnet_data.user_ips where time < now()-interval 4 hour");
	if(ret!=0) {
			TRACE(mysql_error(&conn));	
	}
	mysql_close(&conn);
	Log("BTDataCollector::DeleteOldIPs Done");
}

void BTDataCollector::WriteIPs(vector<PeerList*> v_peers, int message_type) 
{
	Log("BTDataCollector::WriteIPs", message_type);
	int insert_cnt = 0;
	int size = (int)v_peers.size();
	char buf[1024];
	sprintf(buf, "%d peerlist(s) to write.", size);
	Log(buf, message_type);
	if(size == 0) return;

	MYSQL conn;

	mysql_init(&conn);
	if (mysql_real_connect(&conn,bt_db.c_str(),"onsystems","ebertsux37","overnet_data",0,NULL,0) == NULL)
	{
		char buf[1024];
		sprintf(buf, "BTDataCollector::WriteIPs Failed to connect to database: %s\n", mysql_error(&conn));
		Log(buf, message_type);
		TRACE("(Failed to connect to database: %s\n", mysql_error(&conn));
		mysql_close(&conn);
		return;
	}

	mysql_query(&conn, "delete from overnet_data.user_ips where time < now()-interval 4 hour");

	string query = "insert ignore into overnet_data.user_ips values";

	CSingleLock lock(&m_lock);
	lock.Lock();
	if(lock.IsLocked()) {
		for(int i = 0; i < (int)v_peers.size(); i++) {
			PeerList * peers = v_peers[i];
			while(peers != NULL) {
				string ip = peers->GetIP();
				string port = peers->GetPort();
				if(ip.length() > 0) {
					if(insert_cnt > 0) query += ",";
					query += "('"+ip+"','"+port+"', now())";
					insert_cnt++;
				}
				peers = peers->GetNext();
			}
		}
	}
	lock.Unlock();
	int ret = mysql_query(&conn, query.c_str());
	if (ret != 0)	// check for error
	{
		Log(string("BTDataCollector::WriteIPs ")+mysql_error(&conn), message_type);
		Log("BTDataCollector::WriteIPs "+query, message_type);
		TRACE(mysql_error(&conn));	
		TRACE("\n%s\n", query.c_str());
	}
	mysql_close(&conn);
	sprintf(buf, "BTDataCollector::WriteIPs: insert count=%d", insert_cnt);
	Log(buf, message_type);

}

bool BTDataCollector::SendData() 
{
	bool retVal = false;
	CSingleLock lock(&m_lock);
	lock.Lock();
	if(lock.IsLocked()) {
		retVal = SendData(v_torrent_list);
		for(int i = 0; i < (int)v_torrent_list.size(); i++) {
			TorrentFile * tf = v_torrent_list[i];
			v_sent_torrents.push_back(new TorrentFile(*tf));
			delete tf;
		}
		v_torrent_list.clear();
	}
	lock.Unlock();

	return retVal;
}

bool BTDataCollector::SendData(const vector<TorrentFile*> v_torrent_info, int message_type) 
{
	Log("BTDataCollector::SendData() called");
	char * send_data = NULL;
	int len = 0;
	vector<PeerList*> v_peers;

	//v_send_list.clear();
	if(v_send_list.size() == 0) {
		//NameServerInterface ns;
		//ns.RequestIP("BTScraper",v_send_list);
	}
	for(int i = 0; i < (int)v_torrent_info.size(); i++ ) {
		TorrentFile * tf = v_torrent_info[i];
		
		if(tf->GetPeerCount() == 0 || !tf->IsValid()) {
			Log("No Peers for "+tf->GetName(), message_type);
			v_no_peer_torrents.push_back(new TorrentFile(*tf));
			continue;
		}
		if(tf->GetPeerCount() == 1) 
		{
			Log(tf->GetPeers()->ToString(), message_type);
		}
		Log("Sending Data to racks, name:"+tf->GetName(), message_type);
		char message[128];
		sprintf(message, "Peercount: %d", tf->GetPeerCount());
		Log(message, message_type);
		v_peers.push_back(new PeerList(*tf->GetPeers()));
		len = tf->GetBufferSize();
		send_data = new char[len];
		tf->GetBuffer(send_data);
		for(unsigned int i = 0; i < v_send_list.size(); i++) {
			SendReliableData(v_send_list[i], (void*)send_data, len, message_type);
		}
		delete [] send_data;
	}
	//v_send_list.clear();

	//WriteIPs(v_peers, message_type);
	for(int i = 0; i < (int)v_peers.size(); i++) {
		delete v_peers[i];
	}
	v_peers.clear();

	return true;
}


void BTDataCollector::SetDone(bool _done) 
{
	done = _done;
}

void BTDataCollector::GetSearchStringCollection()
{
	ProjectManager pm;
	pm.GetProjectData(&v_project_data);
}


void BTDataCollector::Write(btData *bd)
{
	
	MYSQL conn;
	
	OpenDB(&conn);

	AddNew(bd, &conn);
	CloseDB(&conn);
}

void BTDataCollector::UpdateProjectData() 
{
	Log("Updating Project Data");
	MYSQL conn;
	if(v_project_data.size() == 0) return;

	OpenDB(&conn);

	string query = "update project_data set active = 'U'";// where active = 'A'";
	Execute(&conn, query);
	query = "insert ignore into project_data values ";
	for(unsigned int i = 0; i < v_project_data.size(); i++) {
		ProjectData pd = v_project_data[i];
		if(i!=0) query += ",";
		query += "("+pd.m_id+",'"+pd.m_owner+"','"+pd.m_search_strings+"', 'A')";
	}
	Execute(&conn, query);

	query = "update project_data set active = 'A' where project_id in (";
	for(unsigned int i = 0; i < v_project_data.size(); i++) {
		if(i!=0) query += ",";
		query += v_project_data[i].m_id;
	}
	query += ")";
	TRACE("%s\n", query.c_str());
	Execute(&conn, query);
	CloseDB(&conn);
	Log("Project Data Updated");
}


bool BTDataCollector::OpenDB(MYSQL *conn) 
{
	//Log("BTDataCollector::OpenDB");
	SQLInterface sql;
	return sql.OpenDB(conn, bt_db.c_str(), "bittorrent_data");
}

void BTDataCollector::CloseDB(MYSQL *rs) 
{
	//Log("BTDataCollector::CloseDB");
	mysql_close(rs);
}

bool BTDataCollector::GetFoundData(btData **dbData, MYSQL *conn)
{
	btData * bt_data = *dbData;
	string query = "SELECT \
		title, \
		url, \
		date, \
		type, \
		source, \
		time, \
		seed, \
		dl ,\
		tracker \
		FROM scrape_data \
		where date > curdate() - interval 2 day \
		order by title, source, date";

	int ret = mysql_query(conn, query.c_str());
	if (ret != 0)	// check for error
		TRACE(mysql_error(conn));

	MYSQL_ROW row;

	MYSQL_RES * res = mysql_store_result(conn); // allocates mem (::)

	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);

		if(num_rows > 0)
		{
			// Iterate through the rowset
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[4] != NULL)
				{
					string title = row[0];
					string source = row[4];
					btData *newNode = new btData(title, source);
					newNode->SetURL(row[1]);
					if(bt_data == NULL) bt_data = newNode;
					else bt_data->AddNode(newNode);
					//TRACE("DBDATA: %s\n", newNode->ToString().c_str());
				}
			}
		}
		
		mysql_free_result(res);			// free's mem (::)
	}
	*dbData = bt_data;
	return true;
}

void BTDataCollector::AddNew(btData * bd, MYSQL *conn) 
{
	Log("BTDataCollector::AddNew");
	btData *dbData = NULL;
	GetFoundData(&dbData, conn);
	btData *curr = bd;

	bool b_full_write = false;
	CTime now_time = (CTime::GetCurrentTime());
	CTime old_time;
	if(m_full_write_time == -1) old_time = now_time;
	else old_time = CTime(m_full_write_time);

	long now = (long)now_time.GetTime();
	//TRACE("now: %u, last: %u, diff: %u\n", now, m_full_write_time, (now - m_full_write_time));

	if(m_full_write_time == -1 || (now_time-old_time).GetHours() >= 1) { // once an hour write everything for stat purposes
		m_full_write_time = now;
		//b_full_write = true;
		//Log("FULL WRITE");
		//TRACE("FULL WRITE\n");
	}

	//TRACE("now : %04u-%02u-%02u %02u:%02u:%02u - \n",now_time.GetYear(),now_time.GetMonth(),now_time.GetDay(),now_time.GetHour(),now_time.GetMinute(),now_time.GetSecond());
	//TRACE("last : %04u-%02u-%02u %02u:%02u:%02u - \n",old_time.GetYear(),old_time.GetMonth(),old_time.GetDay(),old_time.GetHour(),old_time.GetMinute(),old_time.GetSecond());

	int insert_cnt = 0;
	int total_cnt = 0;
	while(curr != NULL) {
		if(IsNew(curr, dbData) && MatchesSearchString(curr)) {
			total_cnt++;
			
			try {
				TorrentFile * tf = NULL;
				if(curr->NeedPostData()) {
					tf = GetTorrentFile(curr, curr->NeedPostData(), GetTorrentPostInfo(curr)); 
				}
				else tf = GetTorrentFile(curr); 
				string tracker = GetTracker(tf);
				if(tracker == "" || !tf->IsValid()) {
					curr = curr->GetNext();
					delete tf;
					continue;
				} 
				curr->SetTracker(tracker);
				string hash = (tf == NULL) ? "" : GetHash(tf->GetInfoHash(), 20);
				bool found = false;
				if(!b_full_write) {  // see if we've already seen this hash
					for(int i = 0; i <(int)v_hash_list.size(); i++) {\
						if(hash.compare(v_hash_list[i]) == 0) {
							found = true;
							break;
						}
					}
				}
				if(!b_full_write && found) {
					curr = curr->GetNext();
					delete tf;
					continue;
				} else {
					v_hash_list.push_back(hash);
					v_torrent_list.push_back(tf);
				}
				curr->SetHash(hash);
				TRACE("WRITE TORRENT DATA\n%s, %s\n", tf->GetName().c_str(), hash.c_str());
				WriteTorrentToDB(atoi(curr->GetProjectID().c_str()), tf, curr->GetSeeds(), curr->GetDownloads(), conn);
				//Log(curr->ToString());
			} catch(exception * e) {
				Log("BTDataCollector::AddNew() -- Exception\n");
				Log(e->what());
			}
			// DEBUG disabling Insert for testing
			Insert(curr, conn);
			
		}
		else {
			//TRACE("****************** Duplicate %s **************** \n", curr->GetTitle().c_str());
		}
		curr = curr->GetNext();
	}
	if(dbData != NULL) {
		dbData->Clear();
		delete dbData;
	}
	char message[32];
	sprintf(message, "Total inserts: %d", total_cnt);
	Log(message);
}

string BTDataCollector::GetTorrentPostInfo(btData *bd)
{
	if(!bd->NeedPostData()) return "";
	if(bd->GetSource().compare("myBittorrent") == 0) {
		MyBittorrentParser parser(NULL, &m_dialog->m_main_list_ctrl);

		unsigned short * temp = NULL;
		int size = GetData(bd->GetURL().c_str(), &temp, parser.GetURL(), MAIN_MESSAGE);
		char * p_data = new char[size];
		for(int i = 0; i < size; i++) p_data[i] = (char)temp[i];
		string data(p_data);
		delete [] p_data;
		delete [] temp;
		int cur_pos = (int)data.find("<form");
		int form_end = (int)data.find("</form>", cur_pos);
		if(cur_pos > 0) {
			const char * temp = data.c_str();
			bd->SetRefererURL(bd->GetURL());
			bd->SetPostURL(parser.GetURL() + parser.GetValue(data, &cur_pos, "action=\"", "\""));
			//int len = data.length();
			cur_pos = (int)data.find("<input", cur_pos+1);
			string post_info = "";
			post_info += GetNameValue(data, &cur_pos);
			while(cur_pos < form_end) {
				string  name_value = GetNameValue(data, &cur_pos); 
				if(name_value.length() > 0) post_info += "&" + name_value;
			}
			TRACE("%s\n", post_info.c_str());
			return post_info;
		}
		else return "";

	}
	return "";
}

string BTDataCollector::GetNameValue(string data, int * pos) 
{
	MyBittorrentParser parser(NULL, &m_dialog->m_main_list_ctrl);
	int cur_pos = *pos;
	cur_pos = (int)data.find("<input", cur_pos);
	string type = parser.GetValue(data, &cur_pos, "type=\"", "\"");
	string name_value = "";
	if(type.compare("hidden") == 0) {
		name_value = GetEncodedString(parser.GetValue(data, &cur_pos, "name=\"", "\""));
		name_value += "=" + GetEncodedString(parser.GetValue(data, &cur_pos, "value=\"", "\"")); 
	} else if(type.compare("checkbox") == 0) {
		int checked_index = (int)data.find("checked", cur_pos);
		string value = GetEncodedString(parser.GetValue(data, &cur_pos, "value=\"", "\"")); 
		string name = GetEncodedString(parser.GetValue(data, &cur_pos, "name=\"", "\""));
		if(checked_index != -1 && checked_index < cur_pos) {
			name_value = name + "=" + value;
		}
	}
	*pos = cur_pos;
	return name_value;
}
	

bool BTDataCollector::MatchesSearchString(btData *data) 
{
	int size = (int)v_project_data.size();
	string title = data->GetTitle();
	string type = data->GetType();
	unsigned int file_size = data->GetSize();

	//TRACE("title: %s\n", title.c_str());

	for(int i = 0; i < size; i++) {
		string search_string = v_project_data[i].m_search_strings;
		unsigned int min_size = atoi(v_project_data[i].m_min_size.c_str());
		if(search_string.length() == 0) continue;
		string search_type = v_project_data[i].m_search_types;
		if((search_type == "EVERYTHING" || type == "OTHER" || search_type == type) &&  ContainsAllSearchString(title, search_string)) {
			if(file_size < min_size || ContainsAnySearchString(title, v_project_data[i].m_kill_words)) {
				if(file_size < min_size) TRACE("%u < %u\n", file_size, min_size);
				continue;
			}
			data->SetProjectID(v_project_data[i].m_id);
			data->SetType(search_type);
			Log("title: "+title+" search string: " + search_string);
			return true;
		}
	}
	return false;
}

bool BTDataCollector::ContainsAllSearchString(string title, string temp) {
	CString lower = title.c_str();
	lower = lower.MakeLower();
	title = lower.GetString();

	lower = temp.c_str();
	lower = lower.MakeLower();
	temp = lower.GetString();

	vector<string> search_items;
	int start = 0;
	int index = (int)temp.find(" ");
	while(index > 0) {
		search_items.push_back(temp.substr(start, index - start));
		start = index+1;
		index = (int)temp.find(" ", start);
	}
	search_items.push_back(temp.substr(start));
	
	bool b_found = false; 

	index = -1;
	const char *c_title = title.c_str();

	for(unsigned int i = 0; i < search_items.size(); i++) {
		string item = search_items[i];
		
		if(index < (int)title.find(item))
		{	
			index = (int)title.find(item);
		}
		else {
			b_found = false;
			break;
		}
		if((index == 0 || !(iswalpha(c_title[index-1]))) && 
			((index+item.length() == title.length()) || !(iswalpha(c_title[index+item.length()])))) {
			//TRACE("MATCH: %s, >%s<, %s\n", title.c_str(), item.c_str(), temp.c_str());
			b_found = true;
		}
		else {
			//TRACE("NO MATCH: %s, >%s<, %s\n", title.c_str(), item.c_str(), temp.c_str());
			i--;
			continue;
		}
	}
	if(b_found) {
		TRACE("found: title=%s, temp=%s\n", title.c_str(),temp.c_str());
	}
	else  {
		//TRACE("not found: title=%s, temp=%s\n", title.c_str(),temp.c_str());
	}

	return b_found;
}

bool BTDataCollector::ContainsAnySearchString(string title, string temp) {
	CString lower = title.c_str();
	lower = lower.MakeLower();
	title = lower.GetString();

	vector<string> searchItems;
	int start = 0;
	int index = (int)temp.find(" ");
	while(index > 0) {
		searchItems.push_back(temp.substr(start, index - start));
		start = index+1;
		index = (int)temp.find(" ", start);
	}
	searchItems.push_back(temp.substr(start));
	
	bool found = false; 
	vector <string>::iterator v1_Iter;

	int indexFound = -1;
	for(v1_Iter = searchItems.begin(); v1_Iter != searchItems.end( ) ;v1_Iter++) {
		string item = *v1_Iter;
		if(item == "") continue;
		lower = item.c_str();
		lower = lower.MakeLower();
		item = lower.GetString();
		
		
		if(title.find(item) != -1) {
			//Log(string("title: "+title+" killword: "+item).c_str());
			TRACE(string("title: "+title+" killword: "+item+"\n").c_str());
			return true;
		}
	
	}
	return found;
}

bool BTDataCollector::IsNew(btData *check, btData *db) 
{
	btData *curr = db;
	if(db == NULL) return true;

	string url = check->GetURL();
	for(unsigned int i = 0; i < v_download_list.size(); i++) {
		if(url == v_download_list[i]) {
			//TRACE("Ignoring, %s == %s\n", url.c_str(), v_download_list[i].c_str());
			return false;
		}
	}
	
	while(curr != NULL) {
		if(url == curr->GetURL()) {
			//TRACE("Ignoring, %s == %s\n", url.c_str(), curr->GetURL().c_str());
			return false;
		}
		curr = curr->GetNext();
	}
	return true;
}


TorrentFile * BTDataCollector::GetTorrentFile(btData * bt_data, bool b_post, string post_info) {
	TorrentFile *tf = NULL;
	string urlStr = bt_data->GetURL();
	if(b_post) urlStr = bt_data->GetPostURL();
	string referer=bt_data->GetRefererURL();
	string title=bt_data->GetTitle();
	// checking if we've already downloaded this torrent
	for(unsigned int i = 0; i < v_download_list.size(); i++) 	{  
		if(v_download_list[i] == urlStr) return tf; 
	}
	if(urlStr.find("http") == 0) {
		char * url = GetFormatedURL(urlStr);
		unsigned short * data = NULL;
		int size = 0;
		if(b_post) {
			size = PostData(url, &data, referer, MAIN_MESSAGE, post_info);
		}
		else {
			size = GetData(url, &data, referer, MAIN_MESSAGE);
		}
		
		if(data != NULL && size > 0)  {
			if(title.length() > 0 && data[0] == 'd') {
				WriteTorrentFile(title+"-"+bt_data->GetSource(), data, size);
			}
			tf = new TorrentFile();
			if(!tf->ParseTorrentFile(data, size)) {
				Log("ParseTorrentFile Error");
				TRACE("ParseTorrentFile Error: %s\n", url);
			}
			if(!b_post) v_download_list.push_back(url);
			else v_download_list.push_back(bt_data->GetRefererURL());
			delete [] data;
		}
		if(url != NULL) delete url;
	}
	else {
		Log("ParseTorrent-bad url:");
		Log(urlStr);
	}
	return tf;

}

bool BTDataCollector::CreateDirectory(string dirname)
{
	return (::CreateDirectory(dirname.c_str(), NULL) != 0);
}

bool BTDataCollector::CreateFile(string filename)
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

void BTDataCollector::OpenFile(string filename, CFile *t_file)
{

	if(t_file->Open(filename.c_str(),CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}
	Log(filename+" open");
}

void BTDataCollector::CloseFile(CFile *file)
{
	Log("closing file");
	file->Flush();
	file->Close();
}

void BTDataCollector::WriteTorrentFile(string filename, unsigned short * data, int len)
{
	int index = (int)filename.find(".torrent");
	if(index < 0) {
		filename += ".torrent";
	}
	//CreateFile(m_download_directory);
	filename = m_download_directory + filename;
	if(CreateFile(filename)) {
		WriteFile(filename, data, len);
	}

}

void BTDataCollector::WriteFile(string filename, unsigned short * data, int len)
{
	CFile t_file;

	OpenFile(filename, &t_file);
	
	TRACE("Writting %s\n", filename.c_str());
	char * buf = new char[len];
	for(int i = 0; i < len; i++) {
		buf[i] = (char)data[i];
	}
	t_file.Write(buf,len);
	TRACE("BTDataCollector::WriteFile Done\n");
	delete [] buf;
	CloseFile(&t_file);
}

void BTDataCollector::LogAppend(string message, int len)
{
	CTime now=CTime::GetCurrentTime();
	char time[32];
	sprintf(time,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());

	m_log_file.Write(time, (int)strlen(time));
	m_log_file.Write(message.c_str(), len);
	m_log_file.Write("\r\n", 2);
}

string BTDataCollector::GetTracker(TorrentFile *tf) {
	if(tf == NULL) return "";
	return tf->GetURL().c_str();
}

string BTDataCollector::GetHash(unsigned short * p_hash, int size) {
	if(p_hash == NULL) return "";
	bool b_has_data = false;
	for(int i = 0; i < size; i++) {
		if(p_hash[i] != 0) {
			b_has_data = true;
			break;
		}
	}
	if(!b_has_data) return "";
	unsigned short * hash = new unsigned short[size*2];
	memcpy(hash, p_hash, size*sizeof(unsigned short));
	char * hashStr = new char[2*size+1];
	char szTemp[4];
	strcpy(szTemp, "");
	strcpy(hashStr, "");
	for(int i = 0; i < size; i++) {
		int value = (unsigned char)hash[i];
		if(value < 16) strcat(hashStr, "0");
		itoa(value, szTemp, 16);
		strcat(hashStr, szTemp);
	}
	//TRACE("hash: %s\n", hashStr);
	delete [] hash;
	string retVal= string(hashStr);
	delete [] hashStr;
	return retVal;
}

char * BTDataCollector::GetTrackerURL(TorrentFile * tf, const char *url, char * peer_id) {
	char * trackerURL = new char[512];
	char buffer[20];
	char temp[2];
	strcpy(temp, " ");
	strcpy(trackerURL, "");
	//for(int i = 0; i < (int)strlen(url); i++) {  // gets rid of extra spaces
	//	temp[0] = url[i];
	//	if(temp[0] != ' ') {
	//		strcat(trackerURL, temp);
	//	}
	//}

	strcat(trackerURL, "?info_hash=");
	strcat(trackerURL, tf->GetInfoHashString().c_str());
	strcat(trackerURL, "&peer_id=");
	strcat(trackerURL, peer_id);
	strcat(trackerURL, "&port=49153");
	itoa(tf->GetLength(), buffer, 10);
	strcat(trackerURL, "&uploaded=");
	strcat(trackerURL, "0");
	strcat(trackerURL, "&downloaded=0&left=");
	strcat(trackerURL, "0");
	//strcat(trackerURL, buffer);
	strcat(trackerURL, "&numwant=1000");
	strcat(trackerURL, "&compact=1&key=");

	char * key_id = GetKeyID();

	strcat(trackerURL, key_id);

	delete [] key_id;

	return trackerURL;
}

PeerList *  BTDataCollector::GetPeers(TorrentFile * tf, string title, int message_type) {
	char * peer_id = GetPeerID();

	unsigned short * peers = NULL;
	PeerList * list = NULL;
	char * trackerURL = NULL;
	try{
		int size = 0;
		vector <string>::iterator v1_Iter;
		vector<string> v_list = tf->GetAnnounceList();
		for(v1_Iter = v_list.begin(); v1_Iter != v_list.end( ) ;v1_Iter++) {
			trackerURL = GetTrackerURL(tf, (*v1_Iter).c_str(), peer_id);
			if(trackerURL[0] == '?') continue;
			size = GetData(trackerURL, &peers, "", message_type); // ** 
			if(peers != NULL && peers[0] == 'd' && (peers[1] == '8' || (peers[1] == '1' && peers[2] == '0'))) {
				PeerList * sub_list = new PeerList(peers, size, peer_id, title);
				if(list != NULL) list->Add(sub_list);
				else list = sub_list;
			} else if (size > 0) {
				char * temp = new char[size+1];
				for(int i = 0; i < size; i++) temp[i] =  (char)peers[i];
				temp[size] = 0;
				Log(string("BTDataCollector::GetPeers -- Tracker error: ") + string(temp), message_type);
				TRACE("TRACKER ERROR MESSAGE: %s\n", temp);
				TRACE("%s\n", trackerURL);
				delete [] temp;
				temp = NULL;
			}
			delete [] trackerURL;
			if(peers != NULL) delete [] peers; // **
			peers = NULL;
		}

	} catch(exception * e) {
		delete [] trackerURL;
		Log("BTDataCollector::GetPeers() -- Exception\n", message_type);
		Log(e->what());
	}
	
	delete [] peer_id;
	if(peers != NULL) delete [] peers;
	return list;
}

char * BTDataCollector::GetPeerID() {
	int id_len = 20;
	char * id = new char [id_len+1];
	char * prefix = "-AZ2202-";
	strcpy(id, prefix);
	srand( (unsigned)time( NULL ) );
	for(int i = (int)strlen(prefix); i < id_len; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[id_len] = 0;
	return id;
}

char * BTDataCollector::GetKeyID() {
	int key_len = 8;
	char * id = new char [key_len+1];
	strcpy(id, "");
	srand( (unsigned)time( NULL ) );
	for(int i = 0; i < key_len; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[key_len] = 0;
	return id;
}

char * BTDataCollector::GetFormatedURL(string urlStr) {
	int size = (int)urlStr.length();
	int numSpaces = 0;
	for(int i = 0 ; i < size; i++) {
		if(urlStr[i] == ' ') numSpaces++;
	}
	int new_url_size = size+numSpaces*2;
	char * url = new char[new_url_size+1];
	int j = 0;
	for(int i = 0 ; i < size; i++) {
		url[j] = urlStr[i];
		if(url[j] == ' ') {
			url[j] = '%';
			url[j+1] = '2';
			url[j+2] = '0';
			j = j + 2;
		}
		j++;
	}
	url[new_url_size] = '\0';
	return url;
}

bool BTDataCollector::WriteTorrentToDB(vector<TorrentFile> * pv_tf)
{
	MYSQL conn;
	bool retVal = true;
	mysql_init(&conn);
	if (mysql_real_connect(&conn,bt_db.c_str(),"onsystems","ebertsux37","bittorrent_data",0,NULL,0) == NULL)
	{
		TRACE("(Failed to connect to database: %s\n", mysql_error(&conn));
		return false;
	}
	for(unsigned int i = 0; i < pv_tf->size(); i++) {
		if(!WriteTorrentToDB(0, &((*pv_tf)[i]), 0, 0, &conn)) {
			retVal = false;
			break;	
		};
	}
	mysql_close(&conn);
	return retVal;
}

bool BTDataCollector::WriteTorrentToDB(int project_id, TorrentFile * tf, int seed, int peer, MYSQL * conn)
{
	if(tf == NULL) return false;
	bool debug = false;
	peer = -1;
	seed = -1;
	vector<string> announce_list = tf->GetAnnounceList();
	CString hash = GetHash(tf->GetInfoHash(),20).c_str();
	int hash_id = 0;

	CString query;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	query.Format("insert ignore into torrent_id (hash) values('%s')", hash);

	if(!Execute(conn, query.GetBuffer())) return false;

	query.Format("select hash_id from torrent_id where hash = '%s'", hash);
	res = Query(conn, query.GetBuffer());
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			row = mysql_fetch_row(res);
			hash_id = atoi(row[0]);
		}
		mysql_free_result(res);			// free's mem (::)
	}

	// Check if we already did a full insert of this
	query.Format("select * from tracker_status where hash_id = %d", hash_id);
	res = Query(conn, query.GetBuffer());
	if(res != NULL && mysql_num_rows(res) == announce_list.size()) {
		mysql_free_result(res);			// free's mem (::)
		return true;
	}

	query.Format("insert ignore into torrent_seed_count values(%d, %d, %d, %d, -1, now(), null, %d, %d, now())", hash_id, project_id, seed, peer, seed,peer);
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) return false;

	query.Format("insert into supply_info values(%d,%d,curdate(), now())", hash_id, project_id);
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) {
		query.Format("update supply_info set last_seen = now() where hash_id = %d and date = curdate()", hash_id);
		//return Execute(conn, query.GetBuffer());
		if(!Execute(conn, query.GetBuffer())) return false;
	}
	
	query.Format("insert ignore into torrent_data values (%d,'%s',%d,'%s')",
		hash_id, tf->GetCreatedBy().c_str(), tf->GetCreationDate(), tf->GetEncoding().c_str());
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) return false;

	query = "insert ignore into announce_list values";
	for(unsigned int i = 0; i < announce_list.size(); i++) {
		if(i!=0) query.Append(",");
		query.AppendFormat("(%d,'%s', %d)", hash_id, announce_list[i].c_str(), i);
	}
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) return false;


	query.Format("insert ignore into torrent_info values(%d,'%s','%s','%s',%d,'%s','%s','%s','%s','%s')", 
		hash_id, GetHash(tf->GetEd2k(),16).c_str(), tf->GetName().c_str(), tf->GetNameUTF8().c_str(), tf->GetPieceLength(),
		GetHash(tf->GetSHA1(),20).c_str(), tf->GetPublisher().c_str(), tf->GetPublisherURL().c_str(), 
		tf->GetPublisherURL_UTF8().c_str(), tf->GetPublisherUTF8().c_str());
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) return false;

	vector<CTorrentFileInfo*> files = tf->GetFileInfo();
	int num_files = (int)files.size();
	if(num_files > 0) {
		query = "insert ignore into torrent_file_info values";
		for(int i = 0; i < num_files; i++) {
			CTorrentFileInfo file = *files[i];
			if(i!=0) query.Append(",");
			query.AppendFormat("(%d, %d, '%s', '%s',%u,'%s','%s','%s')", hash_id, i, 
				GetHash(file.GetEd2k(), 16).c_str(), file.GetMd5sum().c_str(), (unsigned int)file.GetLength(), GetInsertString(file.GetPath()).c_str(), 
				GetInsertString(file.GetPathUTF8()).c_str(), GetHash(file.GetSha1(), 20).c_str());
		}
		if(debug) TRACE("QUERY: %s\n", query);
		if(!Execute(conn, query.GetBuffer())) return false;
	}
	else {
		TRACE("no file info for this file\n");
	}
	int num_pieces = tf->GetNumPieces();
	vector<unsigned short*>pieces = tf->GetPieces();
	query = "insert ignore into torrent_pieces values";
	for(int i = 0; i < num_pieces; i++) {
		if(i!=0) query.Append(",");
		query.AppendFormat("(%d,'%d','%s')", hash_id, i, GetHash(pieces[i],20).c_str());
	}
	if(debug) TRACE("QUERY: %s\n", query);
	if(!Execute(conn, query.GetBuffer())) return false;

	for(unsigned int i = 0; i < announce_list.size(); i++ ) {
		query.Format("insert ignore into tracker_status values(%d, 1, now(), 0, %d)", hash_id, i);
		if(debug) TRACE("QUERY: %s\n", query);
		if(!Execute(conn, query.GetBuffer())) return false;
	}

	return true;
}

string BTDataCollector::GetInsertString(string value) 
{
	CString temp = value.c_str();
	temp.Replace("'","\\'");  
	return temp.GetBuffer();

}

void BTDataCollector::Insert(btData *newData, MYSQL *rs)
{

	CTime dt = CTime::GetCurrentTime();
	string cur_date = dt.Format("%Y-%m-%d");
	string cur_time = dt.Format("%H:%M:%S");
	int seeds = 0;
	int dls = 0;
	seeds = newData->GetSeeds();
	dls = newData->GetDownloads();

	char buffer[1024+1];
	sprintf(buffer, "insert into bittorrent_data.scrape_data values('%s','%s','%s','%s','%s','%s',%d%,%d,'%s','%s',%s)",
	newData->GetTitle().c_str(), newData->GetURL().c_str(),cur_date.c_str(), 
	newData->GetType().c_str(), newData->GetSource().c_str(), cur_time.c_str(),
	seeds,dls, newData->GetTracker().c_str(), newData->GetHash().c_str(), 
	newData->GetProjectID().c_str());

	int ret = mysql_query(rs, buffer);
	if (ret != 0)	// check for error
	{
		TRACE(mysql_error(rs));	
		TRACE("\n%s\n", buffer);
	}
	//TRACE("\nquery: %s\n", query.c_str());


}


void BTDataCollector::FindSeeds(TorrentFile *tf) {
	PeerList * peers = tf->GetPeers();
	if(peers != NULL) {
		PeerConnector * pc = new PeerConnector(tf);
		pc->FindSeeds();
		delete pc;
	}
}

string BTDataCollector::GetHeaders(string referer) {
	char * headers = new char[1024];
	strcpy(headers, "Accept: */*\r\n");
	strcat(headers, "User-Agent: Azereus 2.2.0.2\r\n");
	strcat(headers, "Accept-Encoding: gzip\r\n");
	strcat(headers, "Connection: Keep-Alive\r\n");
	if(referer != "") {
		strcat(headers, "Referer: ");
		strcat(headers, referer.c_str());
		strcat(headers, "\r\n");
	}
	string retStr = headers;
	delete [] headers;
	return retStr;
}


// size = size of zipped data.  returns size of unzipped data
int BTDataCollector::UnZip(unsigned short ** dest, unsigned short * source, int len) {
	return ::UnZip(dest, source, len);
		//bool write = false;
		//Bytef *uncompr = NULL;
		//z_stream strm;
		//uLong comprLen = 10000*sizeof(int); /* don't overflow on MSDOS */
		//uLong uncomprLen = comprLen;

		//int zip_status = 0;
		////dest = new unsigned short[size*3];
		//uncompr = new Bytef[uncomprLen];
		//strcpy((char*)uncompr, "garbage");

		//// The fields next_in, avail_in, zalloc, zfree and opaque must be initialized
		//Bytef * byte_src = new Bytef[len];
		//for(int i = 0; i < len; i++) {
		//	byte_src[i] = (Bytef)source[i];
		//}
		////strm.next_in = (Bytef*)source;
		//strm.next_in = byte_src;
		//strm.avail_in = 0;
		//strm.zalloc = (alloc_func)0;
		//strm.zfree = (free_func)0;
		//strm.opaque = (voidpf)0;
		//strm.next_out = (Bytef*)uncompr;

		//zip_status = inflateInit2 (&strm, 15+16); 
		//if(zip_status != Z_OK) { 
		//	delete [] uncompr;
		//	delete [] byte_src;
		//	return 0;
		//}
		//int size = 0;
		//while (strm.total_out < uncomprLen && strm.total_in < comprLen) {
		//	size++;
		//	strm.avail_in = strm.avail_out = 1; /* force small buffers */
		//	zip_status = inflate(&strm, Z_NO_FLUSH);
		//	if (zip_status == Z_STREAM_END) break;
		//	if (zip_status == Z_DATA_ERROR) {
		//		zip_status = inflateSync(&strm);
		//	}
		//	if (zip_status == Z_BUF_ERROR) {
		//		TRACE("unzip error: %d\n", strm.msg);
		//	}
		//	if(zip_status != Z_OK) { 
		//		delete [] uncompr;
		//		delete [] byte_src;
		//		return 0;
		//	}
		//	
		//}
		//zip_status = inflateEnd(&strm); 
		//if(zip_status != Z_OK) { 
		//	delete [] uncompr;
		//	delete [] byte_src;
		//	return 0;
		//}
		//unsigned short * data = *dest;
		//data = new unsigned short[size];
		//for(int i = 0; i < size; i++) {
		//	data[i] = uncompr[i];
		//}
		//delete [] uncompr;
		//delete [] byte_src;
		//*dest = data;
		//return size;
}

int BTDataCollector::GetData(const char * url, unsigned short **body, string referer, int message_type) 
{
	return GetHTTPData(url, body, referer, message_type);
}

int BTDataCollector::PostData(const char * url, unsigned short **body, string referer, int message_type, string post_info) 
{
	return GetHTTPData(url, body, referer, message_type, true, post_info);
}

int BTDataCollector::GetHTTPData(const char * url, unsigned short **body, string referer, int message_type, bool b_post, string post_data) 
{
	//Log(url);
	return ::GetHTTPData(url, body, referer, b_post, post_data);
}

string BTDataCollector::GetEncodedString(string input) 
{
	CString source = input.c_str();
	source.Replace("%", "%25");
	source.Replace("=", "%3D");
	source.Replace("[", "%5B");
	source.Replace("|", "%7C");
	source.Replace(":", "%3A");
	source.Replace("/", "%2F");
	source.Replace("]", "%5D");
	//TRACE("%s\n", source.GetBuffer());
	return source.GetBuffer();
}

void BTDataCollector::Log(string buf, int type) 
{
	Log(buf.c_str(), type);
}

void BTDataCollector::Log(const char *buf, int type) 
{
	if(m_dialog != NULL) {
		m_dialog->Log(buf, type);
	}
	else TRACE("%s\n", buf);
	LogAppend(buf, (int)strlen(buf));
}


void BTDataCollector::ReadTorrents()
{
	vector<string> filenames;
	
	ExploreFolder("C:/torrenttest/",&filenames);
	//ExploreFolder(m_download_directory, &filenames, 3);
	char fname[256];
	vector<TorrentFile> v_tfile;
	for (int i = 0; i < (int)filenames.size(); i++)
	{
		TorrentFile tfile;
		unsigned short * data = NULL;
		strcpy(fname,filenames[i].c_str());
		TRACE("Filename: %s\n", fname);
		tfile.ReadInTorrentFile(fname);
		v_tfile.push_back(tfile);
		delete [] data;
	}
	WriteTorrentToDB(&v_tfile);

}

void BTDataCollector::ExploreFolder(const char * folder, vector<string> * filenames, int days_back)
{
	char buf[4096];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	string tempname;
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			tempname = folder;
			tempname +=data.name;
			tempname += "/";

			ExploreFolder((char *)tempname.c_str(),filenames);
			data.size;

		}
	}
	else
	{
		tempname = folder;
		tempname += data.name;

		filenames->push_back(tempname);
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				tempname = folder;
				tempname +=data.name;
				tempname += "/";
				ExploreFolder((char * )tempname.c_str(),filenames);
				data.size;
			}
		}
		else
		{
			tempname = folder;
			tempname += data.name;

			CTime create_time(data.time_write);
			CTimeSpan diff_time = CTime::GetCurrentTime()-create_time;
			int time_diff = (int)diff_time.GetDays();
			string cur_date = create_time.Format("%Y-%m-%d");
			//TRACE("%s created: %s\n", tempname.c_str(), cur_date.c_str());
			//TRACE("days ago: %u\n", diff_time.GetDays());
			if(days_back < 0 || time_diff < days_back) {
				filenames->push_back(tempname);
			}
		}
	}
	_findclose(handle);
}

bool BTDataCollector::ResendData()
{
	DeleteOldIPs();
	return ResendData(&v_sent_torrents, RESEND_MESSAGE);
}

bool BTDataCollector::ResendData(const vector<TorrentFile*> *v_send_list, int message_type)
{
	vector<TorrentFile*> torrent_list;

	CSingleLock lock(&m_lock);
	lock.Lock();
	if(lock.IsLocked()) {
		unsigned int size = (unsigned int)v_send_list->size();
		for(unsigned int i = 0; i < size; i++) {
			TorrentFile * temp = new TorrentFile(*(*v_send_list)[i]);
			torrent_list.push_back(temp);
		}
		for(unsigned int i = 0; i < v_no_peer_torrents.size(); i++) {
			delete v_no_peer_torrents[i];
		}
		v_no_peer_torrents.clear();
	}
	lock.Unlock();

	unsigned int size = (unsigned int)torrent_list.size();
	for(unsigned int i = 0; i < size; i++) {
		TorrentFile * tf = torrent_list[i];
		PeerList * peers = GetPeers(tf, "", message_type);
		tf->SetPeers(peers);
	}

	bool retVal = SendData(torrent_list, message_type);
	for(unsigned int i = 0; i < size; i++) {
		delete torrent_list[i];
	}
	Log("Send done", message_type);
	return retVal;
}


bool BTDataCollector::Execute(MYSQL * conn, string query)
{
	SQLInterface sql;
	return sql.Execute(conn, query);
	//int ret = mysql_query(conn, query.c_str());
	//if (ret != 0)	{// check for error
	//	TRACE(mysql_error(conn));
	//	TRACE("\nQuery: %s\n", query.c_str());
	//	return false;
	//}
	//return true;
}

MYSQL_RES * BTDataCollector::Query(MYSQL * conn, string query)
{
	SQLInterface sql;
	return sql.Query(conn, query);
	//MYSQL_RES *res;
	//int ret = mysql_query(conn, query.c_str());
	//if (ret != 0)	{// check for error
	//	TRACE(mysql_error(conn));
	//	if(string(mysql_error(conn)).compare("MySQL server has gone away") == 0) {
	//		OpenDB(conn);
	//		ret = mysql_query(conn, query.c_str());
	//	}
	//	if (ret != 0) {
	//		TRACE("\nQuery: %s\n", query.c_str());
	//		TRACE(mysql_error(conn));
	//		return NULL;
	//	}
	//}
	//res = mysql_store_result(conn); // allocates mem (::)
	//return res;
}

