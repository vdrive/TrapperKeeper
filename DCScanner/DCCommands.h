#pragma once
#include "KeyManager.h"
#include "ProjectKeywordsVector.h"
#include "DB.h"
#include "SpoofCollector.h"
#include "Mmsystem.h"
#include "Winsock2.h"

#define NUM_SPOOFS	25		//the number of spoofs to send

class SRActiveClass
{
public:
	string str_sr;
	string ip_address;
	string port;
};
//class that represents all the commands needed for interaction with the hub and other clients
class DCCommands
{
public:
	DCCommands(void);
	~DCCommands(void);
	void SetNick(char* nick);	//sets the nick for user
	CString SendKey(char* data);	//returns all the lock and pk to establish client-server handshake
	void SenyMyInfo(char* data,char* str_myinfo);	//returns all $MyInfo string to send info to the hub
	void SendMyNick(char* data,char* str_mynick);	//returns $MyNick info when a $connect to me is received
	void SendSRPasive(char* data,vector<string> &v_sr);		//returns the $SR command when a passive search is received
	void SendSRActive(char* data,vector<string> &v_sr,char* port,char*ip_address);		//returns the $SR command when an active search is received
	void GetProjectKeywordsVector(ProjectKeywordsVector *pk);
	void GetDBConnection(DB *db);	//gets the database connection
	bool SendConnectToMe(char* data,char* str_connect);
	bool ProjectExists(string project, vector<SpoofCollector> temp);	//checks whether a project has been entered in db
	string GetSpoofPath();
	void SetHubName(char hub[200]);		//store hubname
	void SetHubIpAddress(long ip);		//store the ip address of current hub
	char* GetNick();
	int SetFileSize(int filesize);
private:
	CString m_key;
	ProjectKeywordsVector *p_pkv;	//project keyword vector
	KeyManager m_keymanager;
	vector<char*> v_SR;
	vector<string> querywords;	//vector that contains all the query words in a vector
	vector<SpoofCollector> *v_spooftracker;
	DB* m_db;
	//char* ptr_send;
	//char* ptr2_send;
	char SR[600];
	char buffer[40];
	char lock[280];
	char m_hubname[200];
	char *hub_ip;	//ip address of the
	char m_nick[200];
	int m_difference;
	string m_path;
	int m_combo;
	int m_folders;
	UINT f;
	UINT y;
	int matches;
	bool unique_track;
	bool found_match;
	bool exit;
	CString data_temp;
	CString full_string;
	CString query_words;
	char MyNick[300];
	char MyInfo[300];
	char Connect[300];
	int weight;
	bool found_killword;
	bool found_specific_track;
	vector<SpoofStruct> v_spoofs;
};
