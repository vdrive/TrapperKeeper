#pragma once


#include "afxmt.h"


#include "SQLInterface.h"

#include "ProjectData.h"
#include "ProjectManager.h"
#include "peerconnector.h"
#include "btData.h"
#include "PeerList.h"
//#include "ScrapeData.h"

#include "TorrentFile.h"
#include "string.h"

#include "../BTScraperDll/BTScraperDlg.h"

class BTDataCollector 
{
public:
	BTDataCollector(void);
	~BTDataCollector(void);
	void Collect();
	void SetDone(bool);
	void Write(btData *);
	string GetTracker(TorrentFile * tf);
	string GetHash(unsigned short * p_hash, int size);
	string GetInsertString(string value);
	void FindSeeds(TorrentFile * tf);

	string GetEncodedString(string input);

	int GetData(const char *url, unsigned short **buf, string referer, int message_type);
	int PostData(const char *url, unsigned short **buf, string referer, int message_type, string post_info);
	int GetHTTPData(const char *url, unsigned short **buf, string referer, int message_type, bool b_post=false, string post_info="");
	void SetWindow(BTScraperDlg * dialog);

	void UpdateProjectData();


	bool SendData();
	bool ResendData();
	bool ResendData(const vector<TorrentFile*> * send_list, int message_type);
	bool SendData(const vector<TorrentFile*> v_torrents_info, int message_type=MAIN_MESSAGE); 

	void ReadTorrents();  //For manual torrents
	void ExploreFolder(const char * folder, vector<string> * filenames, int days_back=-1);
	char * m_download_directory;
	char * m_log_file_name;
	void WriteFile(string filename, unsigned short * data, int len);
	bool CreateFile(string filename);
	bool CreateDirectory(string dirname);
	void OpenFile(string filename, CFile *file);
	void CloseFile(CFile *file);
	
	bool Execute(MYSQL * conn, string query);
	MYSQL_RES * Query(MYSQL * conn, string query);

protected:
	string bt_db;

	BTScraperDlg * m_dialog;
	vector<TorrentFile*> v_torrent_list;

	void Log(const char *buf, int type=MAIN_MESSAGE);
	void Log(string buf, int type=MAIN_MESSAGE); 
	void LogAppend(string message, int len);


	bool SendReliableData(string dest, void* data, UINT data_length, int message_type);

	void WriteIPs(vector<PeerList*> v_peers, int message_type);
	void DeleteOldIPs();

	bool GetFoundData(btData **dbData, MYSQL * conn);

private:
	string GetTorrentPostInfo(btData *data); // 
	string GetNameValue(string data, int * pos);

	static string GetHeaders(string referer);
	static int UnZip(unsigned short ** dest, unsigned short * source, int size);
	bool done;
	bool b_started;
	long m_full_write_time;

	CFile m_log_file;
	
	// TODO: these 5 should probably be their own object.
	vector<ProjectData> v_project_data;

	// vector with the ip's we send data to
	vector<string> v_send_list;

	// list of torrents we've already downloaed
	vector<string> v_download_list;  

	// torrents we've already sent.
	vector<TorrentFile*> v_sent_torrents;

	// torrents we have not peers for
	vector<TorrentFile*> v_no_peer_torrents;

	bool OpenDB(MYSQL *);
	void CloseDB(MYSQL *);

	void AddNew(btData *, MYSQL *);
	bool IsNew(btData *, btData *);
	bool MatchesSearchString(btData *);
	void Insert(btData *, MYSQL *);

	bool WriteTorrentToDB(int project_id, TorrentFile * tf, int seed, int  peer, MYSQL * conn);
	bool WriteTorrentToDB(vector<TorrentFile> * tf);

	void GetSearchStringCollection();
	bool ContainsAllSearchString(string title, string search_str);
	bool ContainsAnySearchString(string title, string search_str);

	TorrentFile * GetTorrentFile(btData * data, bool b_post=false, string post_info="");
	void WriteTorrentFile(string title, unsigned short * data, int len);
	PeerList * GetPeers(TorrentFile * tf, string title, int message_type=MAIN_MESSAGE);

	char * GetFormatedURL(string urlStr);
	char * GetTrackerURL(TorrentFile * tf, const char * url, char * peer_id);
	string GetScrapeURL(string tracker, string hash);
	char * GetPeerID();
	char * GetKeyID();

	int m_directory_read;
	btData *m_data;
	vector<string> v_hash_list; // hashes we've sent

	CCriticalSection m_lock;
};
	static char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	//static string cur_title;
