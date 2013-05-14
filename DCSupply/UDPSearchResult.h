#pragma once
#include "uasyncsocket.h"
#include "SupplyKeywords.h"
#include "SupplyKeyword.h"
#include "DB.h"		//database object

class UDPSearchResult :
	public UAsyncSocket
{
public:
	UDPSearchResult(void);
	~UDPSearchResult(void);
	void InitSocket(int socket);
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	int ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short *port,unsigned int*num_read);
	void OnSend(int error_code);
	int SendTo(void *data,unsigned int len,unsigned int ip,unsigned short port);
	void SetProjectData(SupplyKeywords &supply_keywords,char* project_name);
	void SetDialog(void *dialog);
	void DataProcess();
	void DCMasterInsert();
	//int SendTo(void *data,unsigned int len,unsigned ip,unsigned short port,unsigned int *num_sent=NULL);
private:
	SupplyKeywords* p_supply_keywords;
	char* m_project_name;	//name of the current project
	char filename[200];		//stores the file name
	char filesize[200];		//stores the file size
	char user[200];		//user that is sharing the file
	char ip[200];		//user's ip address
	DB db_connection;	//database object for insertions
	void* p_window;
};
