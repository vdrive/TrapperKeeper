#include "StdAfx.h"
#include "BTTrackerCheckerdll.h"
#include "HTTPConnector.h"

//
//
//
BTTrackerCheckerDll::BTTrackerCheckerDll(void)
{
}

//
//
//
BTTrackerCheckerDll::~BTTrackerCheckerDll(void)
{
}

//
//
//
void BTTrackerCheckerDll::DllInitialize()
{
	m_dlg.Create(IDD_BTTRACKERCHECKERDLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTTrackerCheckerDll::DllUnInitialize()
{

}

//
//
//
void BTTrackerCheckerDll::DllStart()
{
	m_dlg.Log("started");
	m_dlg.p_dll = this;
	m_dlg.SetTimer(2, 5*60*1000, NULL); // check for new trackers
	m_dlg.SetTimer(3, 60*1000, NULL); // update seed count of trackers

}


//
//
//
void BTTrackerCheckerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTTrackerCheckerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void BTTrackerCheckerDll::SupplySynched(const char* source_ip)
{
}

bool check_trackers;
bool BTTrackerCheckerDll::RecheckTracker()
{
	if(check_trackers) return false;
	check_trackers = true;
	string query = 
		"select url, hash, t.hash_id, a.tracker_number, f.name \
		from tracker_status t, torrent_id i, announce_list a, project_data d, supply_info s, torrent_info f \
		where t.hash_id = i.hash_id \
		and a.hash_id = i.hash_id \
		and s.hash_id = i.hash_id \
		and f.hash_id = i.hash_id \
		and s.project_id = d.project_id \
		and status_code in (2,3) \
		and t.tracker_number = a.tracker_number \
		and (num_tries < 6 or url='http://tracker.prq.to/announce') \
		and active = 'a' \
		and last_checked < now() - interval 3 hour \
		group by url, hash, a.tracker_number \
		order by hash_id \
		limit 500";

	bool returnValue = CheckTracker(query);
	check_trackers = false;
	if(!returnValue) (m_dlg.Log("Error with re-check tracker"));
	return returnValue;
}

bool BTTrackerCheckerDll::CheckTracker()
{
	string query = 
		"select url, hash, t.hash_id, a.tracker_number, f.name \
		from tracker_status t, torrent_id i, announce_list a, project_data d, supply_info s, torrent_info f \
		where t.hash_id = i.hash_id \
		and a.hash_id = i.hash_id \
		and s.hash_id = i.hash_id \
		and f.hash_id = i.hash_id \
		and s.project_id = d.project_id \
		and status_code = 1 \
		and a.tracker_number = t.tracker_number \
		and (num_tries < 6 or url='http://tracker.prq.to/announce')\
		and active = 'a' \
		group by url, hash, a.tracker_number \
		order by hash_id \
		limit 500";
	bool returnValue =  CheckTracker(query);
	if(!returnValue) (m_dlg.Log("Error with check tracker"));
	return returnValue;
}

bool BTTrackerCheckerDll::CheckTracker(string query)
{

	vector<ScrapeData> v_scrape_data;
	vector<ScrapeData> v_update_data;
	vector<string> v_zero_seed;

	MYSQL_RES *res;
	MYSQL conn;
	MYSQL_ROW row;	

	v_zero_seed.clear();
	if(!OpenDB(&conn)) {
		m_dlg.Log("Error connecting to database");
		return false;
	}

	res = Query(&conn, "select hash from found_zero_seed");
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0) {
				for(int i = 0; i < num_rows; i++) {
					row = mysql_fetch_row(res);
					v_zero_seed.push_back(string(row[0]));
				}
		}
		mysql_free_result(res);		
	}
	while(true) {
		v_scrape_data.clear();

		TRACE("CheckTracker query: %s\n", query.c_str());
		res = Query(&conn, query);
		if(res != NULL)
		{
			int num_rows = (int)mysql_num_rows(res);
			TRACE("numrows: %d\n", num_rows);
			if(num_rows > 0)
			{
				for(int i = 0; i < num_rows; i++) {
					row = mysql_fetch_row(res);
					string scrape_url = string(row[0]);
					string hash = string(row[1]);
					string id = string(row[2]);
					string tracker_number = string(row[3]);
					string name = string(row[4]);

					ScrapeData sd(id, tracker_number, hash, scrape_url);
					sd.m_name = name;
					v_scrape_data.push_back(sd);
				}
				
			}
			mysql_free_result(res);			// free's mem (::)
			if(num_rows == 0) {
				m_dlg.Log("Done Checking Trackers");
				return true;
			}
		}

		if(v_scrape_data.size() == 0) {
			m_dlg.Log("Done Checking Trackers");
			return true;
		}

		if(v_scrape_data.size() > 0) {
			m_dlg.Log("Checking trackers");
			v_update_data.clear();
			for(unsigned int i = 0; i < v_scrape_data.size(); i++) {
				ScrapeData sd = v_scrape_data[i];
				string url = GetScrapeURL(sd.m_url,sd.m_hash);
				TRACE("URL: %s\n", url.c_str());
				m_dlg.Log(string("Checking Tracker URL: ")+url);
				unsigned short * buf = NULL;
				int size = ::GetData(url.c_str(), &buf, "");
				if(size > 0) {
					if(sd.Parse(size, buf)) sd.m_status = "found";
					else {
						sd.m_status = "not found";
					}
				}
				else sd.m_status = "not responding";
				//v_scrape_data.push_back(sd);
				m_dlg.Log(sd.m_hash_id + "(" + sd.m_tracker_number + ") status: " + sd.m_status);
				if(buf != NULL) delete [] buf;
				v_scrape_data[i] = sd;
				v_update_data.push_back(sd);
				if(i%25==0) {
					UpdateTrackerStatus(&v_update_data, &v_zero_seed);
					v_update_data.clear();

				}
			}
		}
		CloseDB(&conn);
		UpdateTrackerStatus(&v_update_data,&v_zero_seed);
		v_update_data.clear();
		m_dlg.Log("Done Checking Trackers");
		return true;
	}

	return false;
}

void BTTrackerCheckerDll::UpdateTrackerStatus(vector<ScrapeData> * v_scrape_data, vector<string> * v_zero_seed) 
{
	vector<string> v_update_string;
	char buf[1024+1];

	vector<string> v_hash_id;

	string last_hash_id = "";
	int seed_count = 0;
	boolean b_new = true;

	for(unsigned int i = 0; i < v_scrape_data->size(); i++ ) {
		ScrapeData sd = (*v_scrape_data)[i]; 
		string status = sd.m_status;
		string hash_id = sd.m_hash_id;
		boolean b_found = (status.compare("found") == 0);
		b_new = !(last_hash_id.compare(hash_id) == 0);
		if(!b_new && b_found) {
			if(seed_count < 0) seed_count = 0;
			seed_count += sd.m_complete;
		}
		if(b_new) {
			seed_count = sd.m_complete;
			last_hash_id = hash_id;
		}
		string tracker_number = sd.m_tracker_number;
		if(status.compare("not responding") == 0) {
			sprintf(buf, 
				"update tracker_status \
				set status_code = 3, last_checked = now(), num_tries = num_tries +1 \
				where hash_id = %s and tracker_number = %s",
				sd.m_hash_id.c_str(),sd.m_tracker_number.c_str());
			//TRACE("QUERY: %s\n", buf);
			v_update_string.push_back(string(buf));
			if(b_new) {
				sprintf(buf, "update torrent_seed_count \
							 set seed_count = -1, peer_count = -1, \
							 last_seen = now() where hash_id = %s",hash_id.c_str());
				//TRACE("QUERY: %s\n", buf);
				v_update_string.push_back(string(buf));
			}
		}
		else if(status.compare("found") == 0) {
			sprintf(buf, 
				"update tracker_status \
				set status_code = 2, last_checked = now(), num_tries = 0 \
				where hash_id = %s and tracker_number = %s",
				sd.m_hash_id.c_str(),sd.m_tracker_number.c_str());
			//TRACE("QUERY: %s\n", buf);
			v_update_string.push_back(string(buf));
			sprintf(buf, "update torrent_seed_count \
						 set seed_count = %d, peer_count = %d, download_count = %d, \
						 last_seen = now() where hash_id = %s",
						 seed_count,sd.m_incomplete,sd.m_complete, hash_id.c_str());
			//TRACE("QUERY: %s\n", buf);
			v_update_string.push_back(string(buf));
			if(seed_count == 0) {
				v_hash_id.push_back(sd.m_hash_id);
				if(Find(sd.m_hash, v_zero_seed) == -1) {
					//v_hash_id.push_back(sd.m_hash_id);
					sprintf(buf, "insert ignore into found_zero_seed values (%s, '%s', '%s')", 
						hash_id.c_str(), sd.m_hash.c_str(), sd.m_url.c_str());
					v_zero_seed->push_back(sd.m_hash);
					//TRACE("QUERY: %s\n", buf);
					v_update_string.push_back(string(buf));
					sprintf(buf, "insert ignore into seed_inflation_data \
							 (hash_id, url, hash, name, seed_count, source, filesize) values \
							 (%s, '%s', '%s', '%s', %d, 'A')", hash_id.c_str(), 
							 sd.m_url.c_str(), sd.m_hash.c_str(), GetInsertString(sd.m_name).c_str(), seed_count);
					//TRACE("QUERY: %s\n", buf);
					v_update_string.push_back(string(buf));
				}
			}
		}
		else if(status.compare("not found") == 0) {
			sprintf(buf, 
				"update tracker_status \
				set status_code = 5, last_checked = now() \
				where hash_id = %s and tracker_number = %s",
				sd.m_hash_id.c_str(),sd.m_tracker_number.c_str());
			//TRACE("QUERY: %s\n", buf);
			v_update_string.push_back(string(buf));
			if(b_new) {
				sprintf(buf, "update torrent_seed_count \
							set seed_count = -1, peer_count = -1, \
							last_seen = now() where hash_id = %s",hash_id.c_str());
				//TRACE("QUERY: %s\n", buf);
				v_update_string.push_back(string(buf));
			}
		}
		else {
			TRACE("Unknown status: %s\n", status.c_str());
		}	
	}

	MYSQL conn;
	OpenDB(&conn);
	for(unsigned int i = 0; i < v_update_string.size(); i++ ) {
		//TRACE("QUERY: %s\n" ,v_update_string[i].c_str());
		if(v_update_string[i].length() > 0) Execute(&conn, v_update_string[i]);
	}
	UpdateFileSize(&v_hash_id, &conn);
	CloseDB(&conn);

}

void BTTrackerCheckerDll::UpdateFileSize(vector<string> *v_hash_id, MYSQL *conn)
{
	string query;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	for(unsigned int i = 0; i < v_hash_id->size();i++) {
		string hash_id = (*v_hash_id)[i];
		query = "select sum(length) from torrent_file_info where hash_id = "+hash_id;
		string size = "0";
		res = Query(conn, query);
		if(res != NULL)
		{
			int num_rows = (int)mysql_num_rows(res);
			if(num_rows > 0) {
					for(int i = 0; i < num_rows; i++) {
						row = mysql_fetch_row(res);
						if(row[0] != NULL) size = (string(row[0]));
					}
			}
			mysql_free_result(res);		
			res = NULL;
		}
		query = "update seed_inflation_data set filesize = "+size+" where hash_id = "+hash_id;
		Execute(conn, query);
		
	}
}

string BTTrackerCheckerDll::GetScrapeURL(string tracker, string hash)
{
	const char * unencoded_hash = hash.c_str();
	char encoded_hash[60+1];
	int cur_pos = 0;
	for(int i = 0; i < 40; i+=2) {
		encoded_hash[cur_pos] = '%';
		cur_pos++;
		encoded_hash[cur_pos] = unencoded_hash[i];
		cur_pos++;
		encoded_hash[cur_pos] = unencoded_hash[i+1];
		cur_pos++;
	}
	encoded_hash[cur_pos] = '\0';
	
	CString temp = tracker.c_str();
	temp.Replace("announce", "scrape");
	tracker = temp.GetBuffer();
	//TRACE("encoded hash %s\n", encoded_hash);

	return tracker + ((tracker.find("?") == -1) ? "?" : "&") + string("info_hash=") + string(encoded_hash);
}

bool BTTrackerCheckerDll::OpenDB(MYSQL *conn) 
{
	SQLInterface sql;
	return sql.OpenDB(conn, "206.161.141.35", "bittorrent_data");
}

void BTTrackerCheckerDll::CloseDB(MYSQL *rs) 
{
	mysql_close(rs);
}

bool BTTrackerCheckerDll::Execute(MYSQL * conn, string query)
{
	SQLInterface sql;
	return sql.Execute(conn, query);
}

MYSQL_RES * BTTrackerCheckerDll::Query(MYSQL * conn, string query)
{
	SQLInterface sql;
	return sql.Query(conn, query);
}

string BTTrackerCheckerDll::GetInsertString(string value) 
{
	CString temp = value.c_str();
	temp.Replace("'","\\'");  
	return temp.GetBuffer();

}

int BTTrackerCheckerDll::Find(string target, vector<string> * vect, int start)
{
	unsigned int size = (unsigned int)vect->size();
	for(unsigned int i = start; i < size; i++) {
		//TRACE("CHECKING: %s\n", (*vect)[i].c_str());
		if(target.compare((*vect)[i]) == 0) return i;
	}
	TRACE("not found: %s\n", target.c_str());
	return -1;
}