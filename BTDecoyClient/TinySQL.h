#pragma once
#include "MySQL.h"
#include <string>
using namespace std;

class TinySQL
{
public:
	TinySQL(void);
	~TinySQL(void);

	bool Init(const char *pHost, const char *pLogin, const char *pPassword, const char *pDatabase, UINT nPort);
	bool Query(const char *pQuery, bool bStoreResults);
	bool BinaryQuery(const char *pQuery, size_t nLen);
	void ClearResults(void);
	void Reset(void);

public:
	string **			m_ppResults;		// string[row][column]
	int					m_nFailureCode;
	string				m_failureReason;
	UINT				m_nRows;
	UINT				m_nFields;

private:
	MYSQL_RES *			m_pSQLResults;
	MYSQL *				m_pSQLReference;	// really a MYSQL data type.
	string				m_initHost;
	string				m_initLogin;
	string				m_initPassword;
	string				m_initDatabase;
	UINT				m_nInitPort;
};
