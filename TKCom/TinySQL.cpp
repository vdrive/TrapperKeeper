#include "StdAfx.h"
#include "tinysql.h"

TinySQL::TinySQL(void)
{
	m_fail_code=-1;
	mp_sql_reference=NULL;
	mpp_results=NULL;
	m_num_fields=0;
	m_num_rows=0;
	mp_sql_results=NULL;
}

TinySQL::~TinySQL(void)
{
	
	if(mp_sql_reference){
		mysql_close( (MYSQL*)mp_sql_reference ) ;
		mp_sql_reference=NULL;
	}

	ClearResults();
}

bool TinySQL::Init(const char* host, const char* login, const char* password, const char* database, UINT port)
{
	m_init_host=host;
	m_init_login=login;
	m_init_password=password;
	m_init_database=database;
	m_init_port=port;
	if ( ( mp_sql_reference = mysql_init( (MYSQL*)0) ) && 	mysql_real_connect ( mp_sql_reference , host , login , password, NULL , port ,	NULL , 0 ) )
	{
		if ( mysql_select_db( mp_sql_reference , database ) < 0 ) {
			CString query;
			query.Format("CREATE DATABASE %s",database);
			if ( mysql_query( mp_sql_reference, (LPCSTR)query ) ) {  //it doesn't exist yet, so create it.
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to create the required database %s!\n",	database ) ;
				return false;
			}
			if ( mysql_select_db ( mp_sql_reference , database ) < 0 ) {
				CString tmp_error_msg;
				tmp_error_msg.Format("Failed to create and or select database %s",database);
				LogError(tmp_error_msg);
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to select the %s database !\n" , database ) ;
				mysql_close( mp_sql_reference ) ;
				mp_sql_reference=NULL;
				return false;
			}
		}
	}
	else {
		CString tmp_error_msg;
		tmp_error_msg.Format("Failed to connect to the mysql server %s on port %d, Aborting Init()",m_init_host.c_str(),m_init_port);
		LogError(tmp_error_msg);
		TRACE( "TinySQL::Init() Failed to connect to the mysql server on port %d.  Aborting Init() !\n",	port ) ;
		mysql_close( mp_sql_reference ) ;
		mp_sql_reference = NULL;
		return false;
	}

	//try to create this table each time.  it doesn't matter if it fails, that probably means the table is already there, which is what we want.
	//if ( mysql_query( mp_sql_reference, "CREATE TABLE RawSupply (project VARCHAR(255),track VARCHAR(255), time TIMESTAMP, ip VARCHAR(16), hash VARCHAR (30), file_name VARCHAR(255))" ) ) {
	//	TRACE( "MetaMachine:  MetaSystem::Init() Failed to create the RawSupply table.  This is probably because it already exists.\n" ) ;
	//}

	TRACE("TinySQL::Init() Completed Successfully.\n");

	CString tmp_msg;
	tmp_msg.Format("SQL Connection to %s:%d created successfully",m_init_host.c_str(),m_init_port);
	LogError(tmp_msg);

	return true;
}

bool TinySQL::Query(const char* query,bool b_store_results)
{
	ClearResults();
	if(mp_sql_reference){
		int stat=mysql_query( mp_sql_reference, query );
		if(stat){
			m_fail_code=stat;
			m_fail_reason=mysql_error(mp_sql_reference);
			TRACE("TinySQL::Query() FAILED (code = %d, reason = %s)\n",m_fail_code,m_fail_reason.c_str());
			CString tmp_error_msg;
			tmp_error_msg.Format("Query Failed: code= %d reason = %s\r\nQuery = %s",m_fail_code,m_fail_reason.c_str(),query);
			LogError(tmp_error_msg);

			return false;
		}
		else{
			if(b_store_results){
				mp_sql_results = mysql_store_result( mp_sql_reference ) ;
				if(mp_sql_results!=NULL){
					m_num_fields = (UINT) mysql_num_fields( mp_sql_results ) ;
					m_num_rows = (UINT) mysql_num_rows( mp_sql_results ) ;

					mpp_results=new string*[m_num_rows];  //create an array of rows
					for(UINT i=0;i<m_num_rows;i++){
						mpp_results[i]=new string[m_num_fields];  //create each row
					}
					
					MYSQL_ROW	row ;
					int row_index=0;
					while ( row = mysql_fetch_row( mp_sql_results ) ) {
						for ( UINT k = 0 ; k < m_num_fields ; k++ ){
							if(row[k]!=NULL){
								mpp_results[row_index][k] = row[k];
							}
							else{
								mpp_results[row_index][k]="";
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

void TinySQL::Reset(){
	if(mp_sql_reference){
		mysql_close( (MYSQL*)mp_sql_reference ) ;
		mp_sql_reference=NULL;
	}

	ClearResults();

	m_fail_code=-1;
	mp_sql_reference=NULL;
	mpp_results=NULL;
	m_num_fields=0;
	m_num_rows=0;
	mp_sql_results=NULL;

	if(m_init_host.size()>0){
		Init(m_init_host.c_str(),m_init_login.c_str(),m_init_password.c_str(),m_init_database.c_str(),m_init_port);
	}
}

void TinySQL::ClearResults(void)
{

	if(mpp_results==NULL)
		return;
	if(mp_sql_results){
		mysql_free_result(mp_sql_results);
		mp_sql_results=NULL;
	}

	//delete each row
	for(UINT i=0;i<m_num_rows;i++){
		delete[] mpp_results[i];
	}

	m_num_rows=0;
	m_num_fields=0;

	//delete the array of rows
	delete [] mpp_results;
	mpp_results=NULL;
}

void TinySQL::LogError(const char* error){
	HANDLE log_file= CreateFile("c:\\tiny_sql_error_log.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                OPEN_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 


	DWORD tmp=0;
	DWORD len=GetFileSize(log_file,&tmp);

	if(len>10000000){
		//recreate the file because its getting to long
		CloseHandle(log_file);
		DeleteFile("c:\\tiny_sql_error_log.txt");

		HANDLE log_file= CreateFile("c:\\tiny_sql_error_log.txt",			// open file at local_path 
					GENERIC_WRITE,              // open for writing 
					FILE_SHARE_READ,			// we don't like to share
					NULL,						// no security 
					OPEN_ALWAYS,				// create new whether file exists or not
					FILE_ATTRIBUTE_NORMAL,		// normal file 
					NULL);						// no attr. template 


		tmp=0;
		len=GetFileSize(log_file,&tmp);
	}

	LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
	LONG low_move=len;  
	DWORD dwPtr=SetFilePointer(log_file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s:  %s\r\n\r\n",cur_time.Format("%A %I:%M %p"),error);
	tmp=0;
	WriteFile(log_file,output.GetBuffer(output.GetLength()),output.GetLength(),&tmp,NULL);

	CloseHandle(log_file);
}
