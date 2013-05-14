#include "StdAfx.h"
#include "TinySQL.h"

//
//
//
TinySQL::TinySQL(void)
{
	m_nFailureCode=-1;
	m_pSQLReference=NULL;
	m_ppResults=NULL;
	m_nFields=0;
	m_nRows=0;
	m_pSQLResults=NULL;
}

//
//
//
TinySQL::~TinySQL(void)
{

	if(m_pSQLReference)
	{
		mysql_close( (MYSQL*)m_pSQLReference );
		m_pSQLReference = NULL;
	}

	ClearResults();
}

//
//
//
bool TinySQL::Init(const char *pHost, const char *pLogin, const char *pPassword, const char *pDatabase, UINT nPort)
{
	m_initHost = pHost;
	m_initLogin = pLogin;
	m_initPassword = pPassword;
	m_initDatabase = pDatabase;
	m_nInitPort = nPort;

	if ( ( m_pSQLReference = mysql_init( (MYSQL*)0) ) && mysql_real_connect( m_pSQLReference, pHost, pLogin, pPassword, NULL, nPort, NULL, 0 ) )
	{
		if ( mysql_select_db( m_pSQLReference , pDatabase ) < 0 ) {
			string query = "CREATE DATABASE ";
			query.append( pDatabase );
			if ( mysql_query( m_pSQLReference, (LPCSTR)query.c_str() ) )
			{
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to create the required pDatabase %s!\n",	pDatabase ) ;
				return false;
			}
			if ( mysql_select_db ( m_pSQLReference , pDatabase ) < 0 )
			{
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to select the %s pDatabase !\n" , pDatabase ) ;
				mysql_close( m_pSQLReference ) ;
				m_pSQLReference=NULL;
				return false;
			}
		}
	}
	else
	{
		TRACE( "MetaMachine:  MetaSystem::Init() ERROR: %s - Aborting Init()!\n", mysql_error( m_pSQLReference ) );
		mysql_close( m_pSQLReference );
		m_pSQLReference = NULL;
		return false;
	}

	TRACE("TinySQL::Init() Completed Successfully.\n");

	return true;
}

//
//
//
bool TinySQL::Query(const char *pQuery, bool bStoreResults)
{
	ClearResults();
	if(m_pSQLReference){
		int stat=mysql_query( m_pSQLReference, pQuery );
		if(stat){
			m_nFailureCode=stat;
			m_failureReason=mysql_error(m_pSQLReference);
			TRACE("MetaMachine:  MetaSystem::Query() FAILED (code = %d, reason = %s) to execute query %s\n",m_nFailureCode,m_failureReason.c_str(),pQuery);

			return false;
		}
		else{
			if(bStoreResults){
				m_pSQLResults = mysql_store_result( m_pSQLReference ) ;
				if(m_pSQLResults!=NULL){
					m_nFields = (UINT) mysql_num_fields( m_pSQLResults ) ;
					m_nRows = (UINT) mysql_num_rows( m_pSQLResults ) ;

					m_ppResults=new string*[m_nRows];  //create an array of rows
					for(UINT i=0;i<m_nRows;i++){
						m_ppResults[i]=new string[m_nFields];  //create each row
					}

					MYSQL_ROW	row ;
					int row_index=0;
					while ( row = mysql_fetch_row( m_pSQLResults ) ) {
						for ( UINT k = 0 ; k < m_nFields ; k++ ){
							if(row[k]!=NULL){
								m_ppResults[row_index][k] = row[k];
							}
							else{
								m_ppResults[row_index][k]="";
							}
						}
						row_index++;
					}
				}
			}

			return true;
		}
	}
	else{
		TRACE("MetaMachine:  MetaSystem::Query() FAILED because not initialized or init failed.\n");
		return false;
	}
}

//
//
//
bool TinySQL::BinaryQuery(const char *pQuery, size_t nLen)
{
	ClearResults();
	if( m_pSQLReference )
	{
		int stat = mysql_real_query( m_pSQLReference, pQuery, (unsigned int)nLen );
		if(stat)
		{
			m_nFailureCode=stat;
			m_failureReason=mysql_error(m_pSQLReference);
			TRACE("MetaMachine:  MetaSystem::BinaryQuery() FAILED (code = %d, reason = %s) to execute query %s\n",
				m_nFailureCode,m_failureReason.c_str(), pQuery );

			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		TRACE("MetaMachine: MetaSystem::BinaryQuery() FAILED because not initialized or init failed.\n");
		return false;
	}
}

//
//
//
void TinySQL::Reset()
{
	if(m_pSQLReference)
	{
		mysql_close( (MYSQL*)m_pSQLReference ) ;
		m_pSQLReference=NULL;
	}

	ClearResults();

	m_nFailureCode=-1;
	m_pSQLReference=NULL;
	m_ppResults=NULL;
	m_nFields=0;
	m_nRows=0;
	m_pSQLResults=NULL;

	if(m_initHost.size()>0){
		Init(m_initHost.c_str(),m_initLogin.c_str(),m_initPassword.c_str(),m_initDatabase.c_str(),m_nInitPort);
	}
}

void TinySQL::ClearResults(void)
{

	if(m_ppResults==NULL)
		return;
	if(m_pSQLResults){
		mysql_free_result(m_pSQLResults);
		m_pSQLResults=NULL;
	}

	//delete each row
	for(UINT i=0;i<m_nRows;i++){
		delete[] m_ppResults[i];
	}

	m_nRows=0;
	m_nFields=0;

	//delete the array of rows
	delete [] m_ppResults;
	m_ppResults=NULL;
}
