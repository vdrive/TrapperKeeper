// DBInterface.h: interface for the DBInterface class.
//
//	This class is to be the inital experiment for transfering our
//	logfile technique to a new database back-end.  We'll use MySQL
//	for now because it is free and sleezy.
//
//	-p was here...
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBINTERFACE_H__7A4E8E53_BE2B_4CB5_B039_F8AA03C9D705__INCLUDED_)
#define AFX_DBINTERFACE_H__7A4E8E53_BE2B_4CB5_B039_F8AA03C9D705__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mysql.h"

class DBInterface  
{
public:
	DBInterface();
	virtual ~DBInterface();

	void OpenConnection(CString ip, CString login, CString pass, CString db);
	void CloseConnection();

	int InsertData(CString table, CString project, unsigned int internal_ip, CString user_name, CString title, CString file_name,
							CString file_size, CString hash, CString time_stamp);

	int InsertToSearchedIps(CString project, CString ip, int file_count);
	int UpdateFileCount(CString project, CString ip, CString today, int file_count);

	static int InsertToOtherArtists(MYSQL *conn, CString project, CString artist, CString album, CString day);
	static void CheckArtistName(CString *artist);

	int NeedToProcess(CString raw_data_table_name);

	bool IsNewIp(CString project, CString ip, CString day);
	bool ReleaseLock();

//	CString m_ip;
//	CString m_login;
//	CString m_pass;

	MYSQL *p_conn;
};

#endif // !defined(AFX_DBINTERFACE_H__7A4E8E53_BE2B_4CB5_B039_F8AA03C9D705__INCLUDED_)
