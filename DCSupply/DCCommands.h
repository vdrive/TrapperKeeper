#pragma once
#include "KeyManager.h"
#include <vector>

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
	char* GetSpoofFileSize();		//uses random numbers to create spoof file sizes
	CString SendKey(char* data);	//returns all the lock and pk to establish client-server handshake
	void SenyMyInfo(char* data,char* str_myinfo);	//returns all $MyInfo string to send info to the hub
	bool SendMyNick(char* data,char* str_mynick);	//returns $MyNick info when a $connect to me is received
	void SendSRPasive(char* data,vector<string> &v_sr);		//returns the $SR command when a passive search is received
	void SendSRActive(char* data,vector<string> &v_sr,char* port,char*ip_address);		//returns the $SR command when an active search is received
	bool SendConnectToMe(char* data,char* str_connect);		//used for client to client handshake

private:
	//char* ip_address;
	CString m_key;
	KeyManager m_keymanager;
	vector<char*> v_SR;
	char SR[250];
};
