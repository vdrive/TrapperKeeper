


#pragma once
#include "db.h"
//#include "SLSKSpooferDlg.h"
//#include "SLSKController.h"


class SLSKaccountInfo
{
private:
//	SLSKController controller;
	//CSLSKSpooferDlg s_dlg;
	db myDB;
	
	void fillName(void);
	char* genUN(void);
	char* genPass(void); 
	int random(int, int);
public:
	char* dbun;
	char* dbpass;
	char* dbname;
	char* dbip;
	char* names[100];
	CString un;
	CString pass;
	int folders, files;
	int port;
	int version;
	SLSKaccountInfo(void);
	~SLSKaccountInfo(void);
	void Init(void);
	int checkAccountInfo(void);
	int checkDeadAccounts(void);
	void createNewAccountInfo(void);
//	void InitController(SLSKController *m_controller);
	void TerminateAccount();
	void freeAccountInfo(void);
	char* getUN(void);
	char* getPass(void);
	void setPort(int);
	int getPort(void);
	int getVersion(void);
	int getFolders(void);
	int getFiles(void);
	void setAccountInfo(void);
	void getUserAccounts(void);
};