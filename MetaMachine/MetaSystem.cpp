#include "StdAfx.h"
#include "metasystem.h"
#include "DonkeyServer.h"
#include "MetaTCP.h"
#include "DCHeader.h"
#include "ProjectKeywordsVector.h"
#include "MetaMachineDLL.h"
#include <mmsystem.h>
//#include "mysql.h"

MetaSystem* MetaSystem::sm_system=NULL;
UINT MetaSystem::sm_system_count=0;

//MYSQL		* mp_raw_mysql_instance ;  

MetaSystem::MetaSystem(void)
{
//	mp_raw_mysql_instance=NULL;
	b_searches_locked=false;
	mb_init=false;
	b_update_projects_database=false;
}

MetaSystem::~MetaSystem(void)
{
	CleanUp();
}

void MetaSystem::AddServer(const char* ip, unsigned short port)
{
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		if(ds->IsServer(ip)){
			ASSERT(0);
			//ds->Reset(true);
			return;
		}
	}
	//server not found, adding a new one.
	DonkeyServer *ds=new DonkeyServer(ip,port);
	mv_servers.Add(ds);
	ds->StartThread();
}

void MetaSystem::NewConnection(UINT handle, const char* source_ip)
{
	//figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		if(ds->IsServer(source_ip)){
			ds->NewConnection(handle);
			return;
		}
	}

	//We don't know who this is.  Let them know what we think of this...
	TRACE("MetaMachine :  MetaSystem::NewConnection()  A rogue connection from %s has been received.  Terminating it now.\n",source_ip);
	TCPReference tcp;
	tcp.TCP()->CloseConnection(handle);  
}

void MetaSystem::NewData(UINT handle, const char* source_ip, Buffer2000& data)
{
	//figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		if(ds->IsServer(source_ip)){
			ds->NewData(handle,data);
			return;
		}
	}

	//We don't know who this is.  Let them know what we think of this...
	//This shouldn't be possible... but just in case (we should be filtering these connections via NewConnection in this class.)
	TRACE("MetaMachine :  MetaSystem::NewData()  rogue data from %s has been received!!!!  This isn't possible by the way.  Terminating this freak connection now.\n",source_ip);
	TCPReference tcp;
	tcp.TCP()->CloseConnection(handle);  
}

void MetaSystem::LostConnection(UINT handle, const char* source_ip)
{
	//figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		if(ds->IsServer(source_ip)){
			ds->LostConnection(handle);
			return;
		}
	}
}



void MetaSystem::Update(void)
{
	static Timer last_search;
	static Timer last_check;
	//static Timer last_save_check;
	static bool searched_at_least_once=false;

	if(!mb_init){
		Init();
		mb_init=true;
	}

	if(last_check.HasTimedOut(30)){
		last_check.Refresh();
		TRACE("query backlog is %d.\n",mv_needed_queries.Size());
		for(UINT i=0;i<mv_servers.Size();i++){
			DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
			if(ds->IsThreadLockedUp()){
				CString log_msg;
				log_msg.Format("CRITICAL SYSTEM ERROR:  The DonkeyServer %s's main thread is locked up.\n",ds->GetIP());
				MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);				
			}
		}		
	}

	CTime cur_time=CTime::GetCurrentTime();

	if((last_search.HasTimedOut(60*60*10) ||(!searched_at_least_once && m_project_keywords_vector.v_projects.size()>0)) && CTime::GetCurrentTime().GetHour()<14 && CTime::GetCurrentTime().GetHour()>0){  //every three hours we do another search
		searched_at_least_once=true;

		//create a bunch of new search jobs
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			//ProjectKeywords *project=&m_project_keywords_vector.v_projects[i];
			if(!m_project_keywords_vector.v_projects[i].m_project_active){
				continue;
			}

			//SupplyKeywords *keywords=m_project_keywords_vector.v_projects[i].m_supply_keywords;
			SearchJob *sj=new SearchJob(m_project_keywords_vector.v_projects[i].m_supply_keywords.m_search_string.c_str(),m_project_keywords_vector.v_projects[i].m_project_name,m_project_keywords_vector.v_projects[i].m_supply_keywords.m_supply_size_threshold);

			if(m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords.size()==0){
				vector <char*> empty_vector;
				sj->AddTrack("Track0",0,empty_vector);
			}
			else{
				for(UINT j=0;j<m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords.size();j++){
					//SupplyKeyword *keyword=m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j];
					//m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords[track_index].m_track
					sj->AddTrack( m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name.c_str() , m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].m_track , m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].v_keywords );
				}
			}

			mv_queued_searches.Add(sj);
			//sj->LogTracks();
		}

		//go back through and set the killwords
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			ProjectKeywords *project=&m_project_keywords_vector.v_projects[i];
			if(!project->m_project_active){
				continue;
			}
			SupplyKeywords *keywords=&project->m_supply_keywords;
			for(UINT j=0;j<keywords->v_killwords.size();j++){
				SupplyKeyword *keyword=&keywords->v_killwords[j];
				for(UINT m=0;m<mv_queued_searches.Size();m++){
					SearchJob* sj=(SearchJob*)mv_queued_searches.Get(m);
					ASSERT(sj!=NULL);
					if(sj->IsJob(project->m_project_name.c_str(),keywords->m_search_string.c_str())){
						sj->AddKillwords(keyword->v_keywords,keyword->m_track_name.c_str());
					}
				}
			}
		}

		last_search.Refresh();
	}

	/*
	//if it hasn't saved in awhile or the current time is just after midnight, save our results to the server
	if(m_last_save<(cur_time-CTimeSpan(0,27,0,0)) || (cur_time.GetHour()==2 && m_last_save<(cur_time-CTimeSpan(0,6,0,0)))){
		SaveToServer();
		m_last_save=cur_time;
	}*/
}

/*
void MetaSystem::NewSupply(UINT job_reference,SearchResult *result)
{
	for(UINT i=0;i<mv_search_tasks.Size();i++){
		SearchJob* sj=(SearchJob*)mv_search_tasks.Get(i);
		ASSERT(sj!=NULL);
		if(sj->GetJobID()==job_reference && !sj->IsClosed()){
			//TRACE("Adding Search Result.\n");
			//result->Dump();
			
			sj->AddResult(result);
			return;
		}
		else if(sj->GetJobID()==job_reference){
			TRACE("Tried adding search result, but the search job was closed.  This is unexpected but probably caused by too many projects.\n");
		}
	}
}*/

void MetaSystem::SaveToServer(void)
{

/*	const char *database = "test";
	const char *DEFALT_SQL_STMT	= "SELECT * FROM edonkeysupply";
	char		aszFlds[ 25 ][ 25 ];
	const char * pszT;
	MYSQL		* mysql_instance ;
	MYSQL_RES	* result ;
	MYSQL_FIELD	* fd ;
	MYSQL_ROW	row ;

	int index=0;
	//....

	if ( (mysql_instance = mysql_init((MYSQL*) 0)) && 	mysql_real_connect( mysql_instance, NULL, NULL, NULL, NULL, 3306,	NULL, 0 ) )
	{
		if ( mysql_select_db( mysql_instance, database ) < 0 ) {
			printf( "Can't select the %s database !\n", database ) ;
			mysql_close( mysql_instance ) ;
			return;
		}
	}
	else {
		TRACE( "Can't connect to the mysql server on port %d !\n",	MYSQL_PORT ) ;
		mysql_close( mysql_instance ) ;
		return;
	}

	if ( ! mysql_query( mysql_instance, DEFALT_SQL_STMT ) ) {
		result = mysql_store_result( mysql_instance ) ;
		int i = (int) mysql_num_rows( result ) ; 
		index=1;
		TRACE( "Query:  %s\nNumber of records found:  %ld\n", DEFALT_SQL_STMT, i ) ;
		//....we can get the field-specific characteristics here....
		for ( int x = 0 ; fd = mysql_fetch_field( result ) ; x++ )
			strcpy( aszFlds[ x ], fd->name ) ;
		//....
		while ( row = mysql_fetch_row( result ) ) {
			int j = mysql_num_fields( result ) ;
			printf( "Record #%ld:-\n", index++ ) ;
			for ( int k = 0 ; k < j ; k++ )
				TRACE( "  Fld #%d (%s): %s\n", k + 1, aszFlds[ k ],	(((row[k]==NULL)||(!strlen(row[k])))?"NULL":row[k])) ;
			TRACE( "==============================\n" ) ;
		}
		mysql_free_result( result ) ;
	}
	else TRACE( "Couldn't execute %s on the server !\n", DEFALT_SQL_STMT ) ;
	//....
	TRACE( "====  Diagnostic info  ====\n" ) ;
	pszT = mysql_get_client_info() ;
	TRACE( "Client info: %s\n", pszT ) ;
	//....
	pszT = mysql_get_host_info( mysql_instance ) ;
	TRACE( "Host info: %s\n", pszT ) ;
	//....
	pszT = mysql_get_server_info( mysql_instance ) ;
	TRACE( "Server info: %s\n", pszT ) ;
	//....
	result = mysql_list_processes( mysql_instance ) ; 
	index = 1 ;
	if (result)
	{
		for ( int x = 0 ; fd = mysql_fetch_field( result ) ; x++ )
			strcpy( aszFlds[ x ], fd->name ) ;
		while ( row = mysql_fetch_row( result ) ) {
			int num_fields = mysql_num_fields( result ) ;
			TRACE( "Process #%ld:-\n", index++ ) ;
			for ( int k = 0 ; k < num_fields ; k++ )
				TRACE( "  Fld #%d (%s): %s\n", k + 1, aszFlds[ k ], (((row[k]==NULL)||(!strlen(row[k])))?"NULL":row[k])) ;
			TRACE( "==============================\n" ) ;
		}
	}
	else
	{
		TRACE("Got error %s when retreiving processlist\n",mysql_error(mysql_instance));
	}
	//....
	result = mysql_list_tables( mysql_instance, "%" ) ; 
	index = 1 ;
	for ( int x = 0 ; fd = mysql_fetch_field( result ) ; x++ )
		strcpy( aszFlds[ x ], fd->name ) ;
	while ( row = mysql_fetch_row( result ) ) {
		int num_fields = mysql_num_fields( result ) ;
		TRACE( "Table #%ld:-\n", index++ ) ;
		for ( int k = 0 ; k < num_fields ; k++ )
			TRACE( "  Fld #%d (%s): %s\n", k + 1, aszFlds[ k ], (((row[k]==NULL)||(!strlen(row[k])))?"NULL":row[k])) ;
		TRACE( "==============================\n" ) ;
	}
	//....
	pszT = mysql_stat( mysql_instance ) ;
	TRACE(" %s \n",pszT) ;
	//....
	mysql_close( mysql_instance ) ;
	return;*/
}

void MetaSystem::Init(void)
{
	TinySQL m_raw_sql;
	m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306);
	m_raw_sql.Query( "CREATE TABLE RawSupply (project VARCHAR(255),track INT, time TIMESTAMP, ip INT,size INT, hash CHAR(40), file_name VARCHAR(255),id INT NOT NULL AUTO_INCREMENT, primary key(project,track,time,id))" , false);
	m_raw_sql.Query( "CREATE TABLE swarm_projects (project VARCHAR(255), primary key(project))" , false);
	m_raw_sql.Query( "CREATE TABLE swarm_labels (label VARCHAR(255), primary key(label))" , false);
	m_raw_sql.Query( "CREATE TABLE swarm_table (project VARCHAR(255), file_name VARCHAR(255),  primary key(label))" , false);
	//m_raw_sql.Query( "CREATE TABLE RawSupply (project VARCHAR(255),track VARCHAR(255), time TIMESTAMP, ip INT, hash CHAR(100), file_name VARCHAR(255))" );
	CreateDirectory("c:\\MetaMachine",NULL);
	this->LoadControllerInfo();

	Sleep(500);
	/*
	AddServer("38.119.96.17",3000);
	AddServer("38.119.96.15",4661);
	AddServer("24.198.82.55",4661);
	AddServer("61.71.81.100",4661);
	AddServer("62.193.128.32",4661);
	AddServer("62.241.35.15",4242);
	AddServer("63.105.207.29",4242);
	AddServer("63.246.128.90",4661);
	AddServer("63.246.131.50",6969);
	AddServer("64.241.35.16",4242);
	AddServer("64.246.38.98",6667);
	AddServer("64.246.48.20",4661);
	AddServer("64.246.54.12",6667);
	AddServer("66.58.18.82",4661);
	AddServer("66.111.36.40",4242);
	AddServer("66.111.54.190",4242);	
	AddServer("66.111.39.20",4242);
	AddServer("66.111.52.200",4224);
	AddServer("66.111.54.50",3000);
	AddServer("66.111.54.100",4242);
	AddServer("66.118.189.240",4242);
	AddServer("66.227.96.151",4661);
	AddServer("66.227.96.252",4661);
	AddServer("66.227.97.9",1701);
	AddServer("69.57.138.74",6667);
	AddServer("140.123.108.139",7654);
	AddServer("193.111.198.139",4242);
	AddServer("193.111.198.138",4242);
	AddServer("193.111.198.137",4242);
	AddServer("193.111.199.179",4661);
	AddServer("193.111.199.183",4661);
	AddServer("193.111.199.187",4661);
	AddServer("193.111.199.211",4242);
	AddServer("194.97.40.162",4242);
	AddServer("195.245.244.243",4661);
	AddServer("195.112.128.222",6667);
	AddServer("195.69.196.58",4242);
	AddServer("207.44.200.40",4242);
	AddServer("207.44.222.47",4661);
	AddServer("207.44.170.12",4661);
	AddServer("207.44.206.27",4242);
	AddServer("209.0.207.220",4661);
	AddServer("210.91.73.198",5555);
	AddServer("210.17.189.98",8888);
	AddServer("211.20.189.140",4661);
	AddServer("211.233.41.235",4661);
	AddServer("211.43.207.149",4661);
	AddServer("211.233.39.38",4661);
	AddServer("211.227.178.134",4242);
	AddServer("216.40.240.53",4661);
	AddServer("216.17.103.14",69);
	AddServer("217.160.176.57",9999);
	AddServer("218.160.176.57",9999);
	AddServer("218.234.22.154",4242);
	*/

	AddServer("38.119.96.17",3000);
	AddServer("38.119.96.15",4661);
	AddServer("24.198.82.55",4661);
	AddServer("61.71.81.100",4661);
	AddServer("61.172.245.120",4242);
	AddServer("62.193.128.32",4661);
	AddServer("62.241.35.15",4242);
	AddServer("62.241.35.16",4242);
	AddServer("62.241.53.2",4242);
	AddServer("62.241.53.3",4242);
	AddServer("62.241.53.4",4242);
	AddServer("62.241.61.10",4242);
	AddServer("63.105.207.29",4242);
	AddServer("63.246.128.90",4661);
	AddServer("63.246.131.50",6969);
	AddServer("64.241.35.16",4242);
	AddServer("64.246.16.11",4661);
	AddServer("64.246.38.98",6667);
	AddServer("64.246.48.20",4661);
	AddServer("64.246.54.12",6667);
	AddServer("64.246.54.33",3306);
	AddServer("64.246.54.138",3306);
	AddServer("66.58.18.82",4661);
	AddServer("66.98.148.44",4661);
	AddServer("66.98.194.14",3306);
	AddServer("66.98.194.26",3306);
	AddServer("66.111.36.40",4242);
	AddServer("66.111.39.20",4242);
	AddServer("66.111.43.80",4242);
	AddServer("66.111.52.200",4224);
	AddServer("66.111.54.50",3000);
	AddServer("66.111.54.190",4242);	
	AddServer("66.111.54.100",4242);
	AddServer("66.118.189.240",4242);
	AddServer("66.227.96.151",4661);
	AddServer("66.227.96.252",4661);	
	AddServer("69.57.138.74",6667);
	AddServer("69.57.144.98",4661);
	AddServer("80.144.172.78",6661);
	AddServer("140.123.108.139",7654);
	AddServer("193.110.78.165",4661);
	AddServer("193.111.198.139",4242);
	AddServer("193.111.198.138",4242);
	AddServer("193.111.198.137",4242);
	AddServer("193.111.199.179",4661);
	AddServer("193.111.199.183",4661);
	AddServer("193.111.199.187",4661);
	AddServer("193.111.199.211",4242);
	AddServer("194.97.40.162",4242);
	AddServer("195.245.244.243",4661);
	AddServer("195.112.128.222",6667);
	AddServer("195.69.196.58",4242);
	AddServer("207.44.170.12",4661);
	AddServer("207.44.182.87",4242);
	AddServer("207.44.200.40",4242);
	AddServer("207.44.206.27",4242);
	AddServer("207.44.222.47",4661);
	AddServer("209.0.207.220",4661);
	AddServer("210.17.189.102",8888);
	AddServer("210.91.73.198",5555);
	AddServer("211.20.189.140",4661);
	AddServer("211.214.161.107",4661);
	AddServer("211.227.178.134",4242);
	AddServer("211.233.39.38",4661);
	AddServer("211.233.41.235",4661);
	AddServer("213.54.163.14",4661);
	AddServer("216.40.240.53",4661);
	AddServer("216.17.103.14",69);
	AddServer("217.160.176.57",9999);
	AddServer("218.234.22.154",4242);


	TRACE("MetaMachine:  MetaSystem::Init() Completed.\n");
	
	/*
	if ( (mp_raw_mysql_instance = mysql_init((MYSQL*) 0)) && 	mysql_real_connect( mp_raw_mysql_instance, DB_LOCAL_HOST, "onsystems", "glad00", NULL, 3306,	NULL, 0 ) )
	{
		if ( mysql_select_db( mp_raw_mysql_instance, RAW_DB ) < 0 ) {
			CString query;
			query.Format("CREATE DATABASE %s",RAW_DB);
			if ( mysql_query( mp_raw_mysql_instance, (LPCSTR)query) ) {  //it doesn't exist yet, so create it.
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to create the required database %s!\n",	RAW_DB ) ;
				return;
			}
			if ( mysql_select_db( mp_raw_mysql_instance, RAW_DB ) < 0 ) {
				TRACE( "MetaMachine:  MetaSystem::Init() Failed to select the %s database !\n", RAW_DB ) ;
				mysql_close( mp_raw_mysql_instance ) ;
				mp_raw_mysql_instance=NULL;
				return;
			}
		}
	}
	else {
		TRACE( "MetaMachine:  MetaSystem::Init() Failed to connect to the mysql server on port %d.  Aborting Init() !\n",	MYSQL_PORT ) ;
		mysql_close( mp_raw_mysql_instance ) ;
		mp_raw_mysql_instance=NULL;
		return;
	}

	
	//try to create this table each time.  it doesn't matter if it fails, that probably means the table is already there, which is what we want.
	if ( mysql_query( mp_raw_mysql_instance, "CREATE TABLE RawSupply (project VARCHAR(255),track VARCHAR(255), time TIMESTAMP, ip VARCHAR(16), hash VARCHAR (30), file_name VARCHAR(255))" ) ) {
		TRACE( "MetaMachine:  MetaSystem::Init() Failed to create the RawSupply table.  This is probably because it already exists.\n" ) ;
	}

	//....
	//mysql_close( mysql_instance ) ;
	TRACE("MetaMachine:  MetaSystem::Init() Completed.\n");
	return;*/
}

void MetaSystem::CleanUp(void)
{
	//signal them all to stop
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		ds->b_killThread=1;
	}		

	//wait for them all to stop
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		ds->StopThread();
	}		
	this->StopThread();
}

void MetaSystem::CreateSearches(void)
{
}

void MetaSystem::ComDataReceived(MetaCom *com, char* source_ip, byte* data, UINT data_length)
{	
	DCHeader *iheader=(DCHeader*)data;
	if(iheader->op==DCHeader::Init){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  Init Header.\n");
		DCHeader oheader;
		oheader.op=DCHeader::Emule_DC_Init_Response;
		oheader.size=sizeof(DCHeader);
		com->SendUnreliableData(source_ip,&oheader,sizeof(DCHeader));
	}

	if(iheader->op==DCHeader::DatabaseInfo){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  DataBaseInfo Header.\n");
		m_data_base_info.ReadFromBuffer((char*)(data+sizeof(DCHeader)));
	}

	if(iheader->op==DCHeader::ProjectChecksums){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  ProjectChecksums Header.\n");
		ProjectChecksums needed_checksums;
		ProjectChecksums pc;

		pc.ReadFromBuffer((char*)(data+sizeof(DCHeader)));

		//see which ones we need to request
		for(UINT i=0;i<pc.v_checksums.size();i++){
			//find our project checksum that is equal to the current one
			ProjectChecksum *ref=&pc.v_checksums[i];
			bool b_found=false;
			bool b_changed=false;
			for(UINT j=0;j<m_project_checksums.v_checksums.size();j++){
				if(stricmp(m_project_checksums.v_checksums[j].m_project_name.c_str(),ref->m_project_name.c_str())==0){
					b_found=true;
					if(m_project_checksums.v_checksums[j]!=*ref){
						TRACE("MetaMachine:  MetaSystem::ComDataReceived() Requesting update for %s.\n",pc.v_checksums[i].m_project_name.c_str());
						needed_checksums.v_checksums.push_back(pc.v_checksums[i]);
					}
					break;
				}
			}
			if(!b_found || b_changed){
				TRACE("MetaMachine:  MetaSystem::ComDataReceived() Requesting update for %s.\n",ref->m_project_name.c_str());
				needed_checksums.v_checksums.push_back(*ref);  //we don't know about this project, we will obviously need keywords for it.
			}
		}

		//see if we need to delete any SearchableItems
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			//SearchableItem* si=(SearchableItem*)mv_searchable.Get(i);
			
			//find the corresponding project in pc
			bool b_found=false;
			for(UINT j=0;j<pc.v_checksums.size();j++){
				if(stricmp(pc.v_checksums[j].m_project_name.c_str(),m_project_keywords_vector.v_projects[i].m_project_name.c_str())==0){
					b_found=true;
					break;
				}
			}

			if(!b_found){  //did we find this project si in the vector of checksums that the controller gave us?
				TRACE("MetaMachine:  MetaSystem::ComDataReceived() Removing project %s.\n",m_project_keywords_vector.v_projects[i].m_project_name.c_str());
				m_project_keywords_vector.v_projects.erase(m_project_keywords_vector.v_projects.begin()+i);
				i--;
			}
		}

		m_project_checksums.ReadFromBuffer((char*)(data+sizeof(DCHeader)));

		if(needed_checksums.v_checksums.size()){
			char *buf=new char[needed_checksums.v_checksums.size()*1000+1000];
			DCHeader header;
			header.op=DCHeader::UpdateProjectsRequest;
			UINT len=needed_checksums.WriteToBuffer(buf+sizeof(DCHeader));
			header.size=len;
			memcpy(buf,&header,sizeof(DCHeader));

			com->SendReliableData(source_ip,buf,len+sizeof(DCHeader));
			delete[] buf;
		}

		SaveChecksumsToDisk();
	}

	if(iheader->op==DCHeader::ProjectKeywords){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  ProjectKeywords Header.\n");
		ProjectKeywordsVector v_project_keywords;
		v_project_keywords.ReadFromBuffer((char*)(data+sizeof(DCHeader)));
		//NEW CODE
		for(UINT i=0;i<v_project_keywords.v_projects.size();i++){  //for each updated project
			//find the corresponding project in our list, if it exists
			bool b_found=false;
			for(UINT j=0;j<m_project_keywords_vector.v_projects.size();j++){
				if(stricmp(v_project_keywords.v_projects[i].m_project_name.c_str(),m_project_keywords_vector.v_projects[j].m_project_name.c_str())==0){
					b_found=true;
					TRACE("MetaMachine:  MetaSystem::ComDataReceived() Updating project %s.\n",v_project_keywords.v_projects[i].m_project_name.c_str());
					m_project_keywords_vector.v_projects[j]=v_project_keywords.v_projects[i];
				}
			}

			if(!b_found){  //we don't have this project, create it
				TRACE("MetaMachine:  MetaSystem::ComDataReceived() Adding project %s.\n",v_project_keywords.v_projects[i].m_project_name.c_str());
				m_project_keywords_vector.v_projects.push_back(v_project_keywords.v_projects[i]);
			}
		}
		SaveKeywordsToDisk();
	}	
}

void MetaSystem::UpdateProjectInfoTree(CTreeCtrl& tree)
{
	tree.DeleteAllItems();  //clear the tree

	//various HTREEITEMS for references.
	HTREEITEM tree_item;
	HTREEITEM top_node;
	HTREEITEM parent_node,keyword_parent;
	for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
		CString str;

		//set project name
		top_node=tree.InsertItem(m_project_keywords_vector.v_projects[i].m_project_name.c_str(),0,0);

		//set owner
		str.Format("Owner = %s",m_project_keywords_vector.v_projects[i].m_owner.c_str());
		tree.InsertItem(str,0,0,top_node);

		//set active
		str.Format("Active = %s",(m_project_keywords_vector.v_projects[i].m_project_active)?"true":"false");
		tree.InsertItem(str,0,0,top_node);

		//set minimum size
		str.Format("Minimum Size = %d",m_project_keywords_vector.v_projects[i].m_supply_keywords.m_supply_size_threshold);
		tree.InsertItem(str,0,0,top_node);

		//set search string
		str.Format("Search String = %s",m_project_keywords_vector.v_projects[i].m_supply_keywords.m_search_string.c_str());
		tree.InsertItem(str,0,0,top_node);

		//set keywords
		tree_item=tree.InsertItem("Supply Keywords",0,0,top_node);
		for(UINT j=0;j<m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords.size();j++){
			str.Format("Track = %s",m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name.c_str());
			parent_node=tree.InsertItem(str,0,0,tree_item);

			//set index of track
			str.Format("index = %d",m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].m_track);
			tree.InsertItem(str,0,0,parent_node);
			//set single flag
			str.Format("single = %s",(m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].m_single)?"true":"false");
			tree.InsertItem(str,0,0,parent_node);

			//add individual keywords
			keyword_parent=tree.InsertItem("Keywords",0,0,parent_node);
			for(UINT k=0;k<m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++){
				tree.InsertItem(m_project_keywords_vector.v_projects[i].m_supply_keywords.v_keywords[j].v_keywords[k],0,0,keyword_parent);
			}
		}

		//set killwords
		tree_item=tree.InsertItem("Supply Killwords",0,0,top_node);
		for(UINT j=0;j<m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords.size();j++){
			str.Format("Track = %s",m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords[j].m_track_name.c_str());
			parent_node=tree.InsertItem(str,0,0,tree_item);

			//set index of track
			str.Format("index = %d",m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords[j].m_track);
			tree.InsertItem(str,0,0,parent_node);
			//set single flag
			str.Format("single = %s",(m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords[j].m_single)?"true":"false");
			tree.InsertItem(str,0,0,parent_node);

			//add individual killwords
			keyword_parent=tree.InsertItem("Killwords",0,0,parent_node);
			for(UINT k=0;k<m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords[j].v_keywords.size();k++){
				tree.InsertItem(m_project_keywords_vector.v_projects[i].m_supply_keywords.v_killwords[j].v_keywords[k],0,0,keyword_parent);
			}
		}
	}

	tree.Invalidate(TRUE);
}

void MetaSystem::InitSearch(void)
{
}

void MetaSystem::SearchedProject(const char* project){
	int count=0;
	while(b_searches_locked && count++<70000 && !this->b_killThread){  //a crappy thread synch system but collisions are extremely rare even without it.
		Sleep(100);
	}
	if(count>=70000){
		//MessageBox(NULL,"Operation Timed Out - Couldn't enter a search into the c:\\MetaMachine\\searches.txt file.  This is probably because another thread had the file locked.","Error",MB_OK);
	}
	b_searches_locked=true;
	CTime date=CTime::GetCurrentTime();
	CFile searches("c:\\MetaMachine\\searches.txt",CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::typeBinary);

	//output into this file that we searched this project
	CString line;
	//project name : year,month,day
	line.Format("%s : %d.%d.%d\n",project,date.GetYear(),date.GetMonth(),date.GetDay());
	searches.SeekToEnd();
	searches.Write((LPCSTR)line,line.GetLength());
	b_searches_locked=false;
}

void MetaSystem::SaveChecksumsToDisk(void)
{
	CFile file_checksums("c:\\MetaMachine\\checksums.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

	//save checksums
	UINT len=m_project_checksums.GetBufferLength();
	char *buf=new char[len];
	m_project_checksums.WriteToBuffer(buf);
	file_checksums.Write(buf,len);
	delete []buf;
}

void MetaSystem::SaveKeywordsToDisk(void)
{
	CFile file_keywords("c:\\MetaMachine\\keywords.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
	
	//save keywords
	UINT len=m_project_keywords_vector.GetBufferLength();
	char *buf=new char[len];
	m_project_keywords_vector.WriteToBuffer(buf);
	file_keywords.Write(buf,len);
	delete []buf;
}

void MetaSystem::LoadControllerInfo(void)
{
	OFSTRUCT tstruct;
	if(HFILE_ERROR!=OpenFile( "c:\\MetaMachine\\keywords.txt",   &tstruct,  	OF_EXIST	)){
		CFile file_keywords("c:\\MetaMachine\\keywords.txt",CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_keywords.GetLength();
		char *buf=new char[len];
		file_keywords.Read(buf,len);
		m_project_keywords_vector.ReadFromBuffer(buf);
		delete []buf;
	}

	if(HFILE_ERROR!=OpenFile( "c:\\MetaMachine\\checksums.txt",   &tstruct,  	OF_EXIST	)){
		CFile file_checksums("c:\\MetaMachine\\checksums.txt",CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_checksums.GetLength();
		char *buf=new char[len];
		file_checksums.Read(buf,len);
		m_project_checksums.ReadFromBuffer(buf);
		delete []buf;
	}
}

//The system must run this thread to be able to do queued database queries without blocking the rest of the searching process.
UINT MetaSystem::Run()
{
	static Timer last_report;
	static Timer last_search;
	static Timer last_insert;
	static Timer project_output;
	static Timer last_protection_table_build;
	while(!this->b_killThread){
		int qcount=0;
		m_main_thread_check.Refresh();  //we are looping in our main thread so it must still be alive.
		if(mv_needed_queries.Size()>0 && last_insert.HasTimedOut(30)){  //once every 20 seconds we will form a connection and pump a lot of queries into the database.
			MetaMachineDll::GetDlg()->WriteToLogFile2("Started:  Running Needed Queries.");
			TinySQL m_raw_sql;
			if(m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306)){
				while(mv_needed_queries.Size()>0 && !this->b_killThread && qcount++<15000){
					Query *q=(Query*)mv_needed_queries.Get(0);
					bool stat=m_raw_sql.Query(q->m_query.c_str(),false);
					if(!stat){
						MetaMachineDll::GetDlg()->AddFailedQuery(q->m_query.c_str());
					}
					mv_needed_queries.Remove(0);
				}
			}
			MetaMachineDll::GetDlg()->SetRemainingQueries((int)mv_needed_queries.Size());
			last_insert.Refresh();  //restart our insert timer
			MetaMachineDll::GetDlg()->WriteToLogFile2("Finished:  Running Needed Queries.");
		}

		//allow the servers to start on or queue a new search every X seconds.
		if(last_search.HasTimedOut(40)){
			MetaMachineDll::GetDlg()->WriteToLogFile2("Started:  Adding a new search.");
			if(mv_queued_searches.Size()>0){
				int sindex=mv_queued_searches.Size()-1;
				SearchJob *sj=(SearchJob*)mv_queued_searches.Get(sindex);
				
				this->SearchedProject(sj->GetProject());

				for(UINT m=0;m<mv_servers.Size();m++){
					DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(m);
					SearchJob *tsj=new SearchJob(sj);
					ds->Search(tsj);
				}
				mv_queued_searches.Remove(sindex);
			}
			last_search.Refresh();
			MetaMachineDll::GetDlg()->WriteToLogFile2("Finished:  Adding a new search.");
		}

		if(last_report.HasTimedOut(3*60*60)){
			MetaMachineDll::GetDlg()->WriteToLogFile2("Started:  Reporting all results.");
			int count=0;
			while(b_searches_locked && count++<700 && !this->b_killThread){  //a crappy synch system but collisions are extremely rare even without it.
				Sleep(100);
			}
			if(b_searches_locked)
				continue;
			b_searches_locked=true;

			ReportAllResults();
			b_searches_locked=false;
			last_report.Refresh();
			MetaMachineDll::GetDlg()->WriteToLogFile2("Finished:  Reporting all results.");
		}//every 8 hours we will update our protection tables
		if(last_protection_table_build.HasTimedOut(60*60*8)){
			MetaMachineDll::GetDlg()->WriteToLogFile2("Started:  Rebuilding Protection Tables.");
			BuildSpoofTable();
			BuildFalseDecoyTable();
			BuildSwarmTable();
			MetaMachineDll::GetDlg()->WriteToLogFile2("Finished:  Rebuilding Protection Tables.");
			last_protection_table_build.Refresh();
		}
		if(project_output.HasTimedOut(30*60) || b_update_projects_database){
			b_update_projects_database=false;
			if(m_project_keywords_vector.v_projects.size()>0){
				TinySQL m_project_sql;
				if(m_project_sql.Init("38.119.66.51","onsystems","tacobell","final_data",3306)){			
					CString project_query="insert into project_list values ";
					
					int project_query_count=0;
					
					m_project_sql.Query("create table track_list (project varchar(255),track_index int,track_name varchar(255))",false);
					for(int project_index=0;project_index<(int)m_project_keywords_vector.v_projects.size() && !this->b_killThread;project_index++){
						bool b_active=m_project_keywords_vector.v_projects[project_index].m_project_active;
						CString tmp;
						CString project_name_tmp=m_project_keywords_vector.v_projects[project_index].m_project_name.c_str();
						project_name_tmp.Replace("\\","\\\\");
						project_name_tmp.Replace("'","\\'");
						CString album_name_tmp=m_project_keywords_vector.v_projects[project_index].m_album_name.c_str();
						album_name_tmp.Replace("\\","\\\\");
						album_name_tmp.Replace("'","\\'");
						CString artist_name_tmp=m_project_keywords_vector.v_projects[project_index].m_artist_name.c_str();
						artist_name_tmp.Replace("\\","\\\\");
						artist_name_tmp.Replace("'","\\'");
						CString owner_name_tmp=m_project_keywords_vector.v_projects[project_index].m_owner.c_str();
						owner_name_tmp.Replace("\\","\\\\");
						owner_name_tmp.Replace("'","\\'");
						if(project_query_count==0){
							tmp.Format("('%s','%s','%s','%s','%s')",project_name_tmp,owner_name_tmp,album_name_tmp,artist_name_tmp,(b_active)?"TRUE":"FALSE");
						}
						else{
							tmp.Format(",('%s','%s','%s','%s','%s')",project_name_tmp,owner_name_tmp,album_name_tmp,artist_name_tmp,(b_active)?"TRUE":"FALSE");
						}
						project_query+=tmp;
						project_query_count++;
										
						CString track_query="INSERT INTO track_list VALUES ";
						int track_query_count=0;
						//query.Format("insert into project_list values ('%s','%s','%s','%s','%s')",m_project_keywords_vector.v_projects[project_index].m_project_name.c_str(),m_project_keywords_vector.v_projects[project_index].m_owner.c_str(),m_project_keywords_vector.v_projects[project_index].m_album_name.c_str(),m_project_keywords_vector.v_projects[project_index].m_artist_name.c_str(),(b_active)?"TRUE":"FALSE");
						for(int track_index=0;track_index<(int)m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords.size();track_index++){
							CString cstr_track_name=m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords[track_index].m_track_name.c_str();
							cstr_track_name.Replace("'","\\'");
							if(track_query_count==0){
								tmp.Format("('%s',%d,'%s')",m_project_keywords_vector.v_projects[project_index].m_project_name.c_str(),m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords[track_index].m_track,(LPCSTR)cstr_track_name);
							}
							else{
								tmp.Format(",('%s',%d,'%s')",m_project_keywords_vector.v_projects[project_index].m_project_name.c_str(),m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords[track_index].m_track,(LPCSTR)cstr_track_name);
							}
							track_query+=tmp;	
							track_query_count++;
						}
						CString delete_query;
						delete_query.Format("delete from track_list where project='%s'",m_project_keywords_vector.v_projects[project_index].m_project_name.c_str());
						m_project_sql.Query(delete_query,true);
						if(track_query_count>0){
							CString tmp;
							tmp.Format("Running track insert query: %s",track_query);
							MetaMachineDll::GetDlg()->WriteToLogFile3(tmp);
							m_project_sql.Query(track_query,false);
						}
						else{
							CString tmp;
							tmp.Format("Project %s has no tracks! Project vector has length %d.  Track vector for this project has length %d.",m_project_keywords_vector.v_projects[project_index].m_project_name.c_str(),m_project_keywords_vector.v_projects.size(),m_project_keywords_vector.v_projects[project_index].m_supply_keywords.v_keywords.size());
							MetaMachineDll::GetDlg()->WriteToLogFile3(tmp);
							m_project_sql.Query(track_query,false);
						}
					}
					m_project_sql.Query("drop table project_list",false);
					m_project_sql.Query("create table project_list (project varchar(255),owner varchar(255),album varchar(255),artist varchar(255),active varchar(10))",false);
					m_project_sql.Query(project_query,false);
					CString reason=m_project_sql.m_fail_reason.c_str();
					MetaMachineDll::GetDlg()->WriteToLogFile3(reason+": "+project_query);
				}
			}
			project_output.Refresh();
		}
		Sleep(20);
	}
	return 0;
}

void MetaSystem::AddQuery(const char* query)
{
	Query *nq=new Query();
	nq->m_query=query;
	mv_needed_queries.Add(nq);
	MetaMachineDll::GetDlg()->SetRemainingQueries((int)mv_needed_queries.Size());
	int count=0;
	while(mv_needed_queries.Size()>900000 && count++<600){  //slow it down if its way over loaded.  we can only query so fast.
		Sleep(1000);
	}
}

void MetaSystem::ReportAllResults(void)
{
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [0]");
	bool b_central_init=false;
	CString log_msg;
	log_msg.Format("Reporting Results - Initing central sql with host = '%s' , user = '%s' , password = '%s' \n",m_data_base_info.m_db_host.c_str(),m_data_base_info.m_db_user.c_str(),m_data_base_info.m_db_password.c_str());
	MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
	TRACE((LPCSTR)log_msg);
	TinySQL m_central_sql;
	if(m_central_sql.Init(m_data_base_info.m_db_host.c_str(),m_data_base_info.m_db_user.c_str(),m_data_base_info.m_db_password.c_str(),"dcdata",3306))
		b_central_init=true;

	if(!b_central_init)
		return;

	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [1]");
	TRACE("MetaSystem:  MetaSystem::ReportAllResults(void)\n");

	//make sure a searches file has been created or the stupid mfc classes throw an exception
	OFSTRUCT tstruct;
	if(HFILE_ERROR==OpenFile( "c:\\MetaMachine\\searches.txt",   &tstruct,  	OF_EXIST	)){
		return;
	}
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [2]");

	TinySQL m_raw_sql;
	m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306);

	CStdioFile searches("c:\\MetaMachine\\searches.txt",CFile::modeRead|CFile::typeBinary);
	CString line;

	//5 parallel vectors
	vector <string> v_projects;  //the project
	vector <int> v_years;  //the year
	vector <int> v_months;  //the month
	vector <int> v_days;  //the date
	vector <int> v_count;   //how many instances of this project on the specified date, to figure out an average when we run the query

	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [3]");
	while(searches.ReadString(line)){
		int index=line.Find(":");
		if(index==-1)
			continue;
		CString project=line.Mid(0,index);
		CString date=line.Mid(index+1);

		project=project.Trim();
		date=date.Trim();

		index=date.Find(".");  //reuse index variable to store the location of the first '.'
		int index2=date.Find(".",index+1);  //create index2 to store the location of the second '.'
		if(index==-1 || index2==-1)
			continue;
		CString year=date.Mid(0,index);
		CString month=date.Mid(index+1,index2-index);
		CString day=date.Mid(index2+1);

		int iyear=atoi(year);
		int imonth=atoi(month);
		int iday=atoi(day);

		//see if the project is already in the list for this day...
		bool b_found=false;
		for(UINT i=0;i<v_projects.size();i++){
			if(iday==v_days[i] && imonth==v_months[i] && iyear==v_years[i] && stricmp(v_projects[i].c_str(),project)==0){
				v_count[i]=v_count[i]+1;
				b_found=true;
				break;
			}
		}
		if(b_found)
			continue;

		v_projects.push_back(string((LPCSTR)project));
		v_years.push_back(iyear);
		v_months.push_back(imonth);
		v_days.push_back(iday);
		v_count.push_back(1);
	}
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [4]");

	vector <string> v_log_projects;  //the project
	vector <int> v_log_years;  //the year
	vector <int> v_log_months;  //the month
	vector <int> v_log_days;  //the date
	vector <int> v_log_count;   //how many instances of this project on the specified date

	CTime cur_time=CTime::GetCurrentTime();
	for(UINT i=0;i<v_projects.size();i++){
		CTime p_time(v_years[i],v_months[i],v_days[i],2,1,1);
		if(p_time<(cur_time-CTimeSpan(1,0,0,0))){  //if this is from yesterday or before we need to report it
			v_log_projects.push_back(v_projects[i]);  //add it to a vector to note for later that we need to report this project on this date.
			v_log_years.push_back(v_years[i]);
			v_log_months.push_back(v_months[i]);
			v_log_days.push_back(v_days[i]);
			v_log_count.push_back(v_count[i]);  //use the count to figure out an average

			v_projects.erase(v_projects.begin()+i);  //erase it since it is about to be processed and we don't want to write it back to file
			v_years.erase(v_years.begin()+i);
			v_months.erase(v_months.begin()+i);
			v_days.erase(v_days.begin()+i);
			v_count.erase(v_count.begin()+i);
			i--;
		}
	}
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [5]");

	//v_log vectors now contain project/date combinations that need to be reported to the main database.
	//make sure our table exists on the central server
	m_central_sql.Query( "CREATE TABLE edonkeysupply (time TIMESTAMP, project VARCHAR(255),track int, supply INT)" , false);
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [6]");
	bool b_was_successful=false;
	CString log_str;
	log_str.Format("Reporting %d projects to central sql database.",v_log_projects.size());
	m_raw_sql.Query("DELETE from rawsupply where time<(now()-INTERVAL 2 DAY)",false);
	MetaMachineDll::GetDlg()->WriteToLogFile(log_str);
	for(UINT i=0;i<v_log_projects.size();i++){
		Sleep(1000);
		CString select_query;

		//add up the results from this day;
		select_query.Format("SELECT track,count(*) from rawsupply where project='%s' AND YEAR(time)=%d AND MONTH(time)=%d AND DAYOFMONTH(time)=%d GROUP BY track",v_log_projects[i].c_str(),v_log_years[i],v_log_months[i],v_log_days[i]);
		m_raw_sql.Query(select_query,true);

		//ASSERT( m_raw_sql.m_num_rows <= 1 && m_raw_sql.m_num_fields <= 1 );
		for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
			int count=atoi(m_raw_sql.mpp_results[j][1].c_str());
			CString insert_query;
			insert_query.Format("INSERT INTO edonkeysupply VALUES ('%d-%d-%d 00:00:00','%s',%s,%d)",v_log_years[i],v_log_months[i],v_log_days[i],v_log_projects[i].c_str(),m_raw_sql.mpp_results[j][0].c_str(),count);
			if(!m_central_sql.Query((LPCSTR)insert_query,false)){  //report this to the central server
				TRACE("Failed to run a query on the central sql database. Aborting project reporting for now, will retry in an hour.\n  The query was %s.\n",(LPCSTR)insert_query);
				CString log_msg;
				log_msg.Format("FAILED central sql QUERY. The query was %s.",(LPCSTR)insert_query);				
				MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
				MetaMachineDll::GetDlg()->WriteToLogFile(log_msg);
				if(!b_was_successful){  //was it even successful once?  if so maybe we should just continue and try to report the rest of them.
					return;
				}
			}
			else{
				b_was_successful=true;
			}
		}

		//run another query to delete these results from the database since we reported them
		//query.Format("DELETE from rawsupply where project='%s' AND YEAR(time)=%d AND MONTH(time)=%d AND DAYOFMONTH(TIME)=%d",v_log_projects[i].c_str(),v_log_years[i],v_log_months[i],v_log_days[i]);
		//m_raw_sql.Query(query,false);
	}
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [6]");
	
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [7]");

	searches.Close();
	
	//if the query fails because the central server and/or network is down, we need to make sure we save the logs back into the file so we can try this again later
	
	//reopen searches.txt for writing to spit out remaining unreported projects where the search is too new to report
	CFile searches_out("c:\\MetaMachine\\searches.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

	//write v_projects back to file (because we remove the ones that we have reported.)
	log_str.Format("Writing %d projects back to searches file for later processing.",v_projects.size());
	MetaMachineDll::GetDlg()->WriteToLogFile(log_str);
	for(UINT i=0;i<v_projects.size();i++){
		line.Format("%s : %d.%d.%d\n",v_projects[i].c_str(),v_years[i],v_months[i],v_days[i]);
		searches_out.Write((LPCSTR)line,line.GetLength());
	}
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [8]");

	m_raw_sql.Query("OPTIMIZE table rawsupply",false);  //this will free harddrive space by freeing the disk space associated with the rows we deleted.
	MetaMachineDll::GetDlg()->WriteToLogFile("MetaSystem::ReportAllResults [done]");
}

void MetaSystem::UpdateServerTree(CTreeCtrl& tree)
{
	tree.DeleteAllItems();  //clear the tree

	//various HTREEITEMS for references.
	//HTREEITEM tree_item;
	HTREEITEM top_node;
	//HTREEITEM parent_node,keyword_parent;
	for(UINT i=0;i<mv_servers.Size();i++){
		DonkeyServer *ds=(DonkeyServer*)mv_servers.Get(i);
		CString str;
		CString up_time;
		CTime connected=ds->ConnectionTime();
		CTimeSpan time_span=CTime::GetCurrentTime()-connected;
		if(time_span.GetDays()>400){
			up_time="Not Connected";
		}
		else{
			up_time.Format("%d minutes",time_span.GetTotalMinutes());
		}
		str.Format("%s:%d ( uptime = %s ) ( %d search jobs remaining )",ds->GetIP(),ds->GetPort(),up_time,ds->GetSearchTaskCount());
		top_node=tree.InsertItem(str,0,0);
	}

	tree.Invalidate(TRUE);
}

void MetaSystem::BuildSpoofTable(void)
{
	srand(timeGetTime());
	TRACE("MetaSystem:  MetaSystem::BuildSpoofTable(void)\n");

	//MessageBox(NULL,"Building Spoof Table","Notice.",MB_OK);
	TinySQL m_raw_sql;
	m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306);

	//v_log vectors now contain project/date combinations that need to be reported to the main database.
	//make sure our table exists on the central server

	vector <string>v_queries;
	int query_adds=0;
	//do the spoof_add table  ( the table that lets us manually add things
	m_raw_sql.Query("select file_name,size from spoof_adds",true);
	for(UINT m=0;m<m_raw_sql.m_num_rows;m++){  //take top 30 file names (the 30 highest counts)
		CString file_name=m_raw_sql.mpp_results[m][0].c_str();
		int size=atoi(m_raw_sql.mpp_results[m][1].c_str());
		if(size<800000)  //skip the small files
			continue;
		//insert some random hashes for each file name

		for(UINT hash_count=0;hash_count<25;hash_count++){  //we can generate up to 1800 fake links per project
			CString hash=GenerateRandomHash();

			CString query;
			query_adds++;
			int file_size=max(10,atoi(m_raw_sql.mpp_results[m][1].c_str())-100000+(rand()%200000));  //create a random file_size that will very likely be slighly more or less than the real file size
			query.Format("INSERT INTO spoof_table VALUES ('%s','%s',%d)",m_raw_sql.mpp_results[m][0].c_str(),(LPCSTR)hash,file_size);
			v_queries.push_back(string((LPCSTR)query));  //push this query onto the vector of queries we need to do in a moment.
		}
	}
	CString msg;
	msg.Format("Finished inserting %d user defined files with %d hashes.",m_raw_sql.m_num_rows,query_adds);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	vector <string>v_spoof_projects;  //two parallel vectors
	vector <int>v_spoof_priorities;
	m_raw_sql.Query("select * from spoof_labels",true);

	int project_adds=0;
	//add all the projects in for each of the spoof labels
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string label=m_raw_sql.mpp_results[j][0].c_str();
		int priority=atoi(m_raw_sql.mpp_results[j][1].c_str());

		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			if(stricmp(m_project_keywords_vector.v_projects[i].m_owner.c_str(),label.c_str())==0){
				//add this project
				project_adds++;
				v_spoof_projects.push_back(m_project_keywords_vector.v_projects[i].m_project_name);
				v_spoof_priorities.push_back(priority);
			}
		}
	}

	msg.Format("Finished adding %d labels and %d projects from them.",m_raw_sql.m_num_rows,project_adds);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	m_raw_sql.Query("select * from spoof_projects",true);
	//add all the projects in for each of the spoof labels
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string project=m_raw_sql.mpp_results[j][0].c_str();
		int priority=atoi(m_raw_sql.mpp_results[j][1].c_str());
		v_spoof_projects.push_back(project);
		v_spoof_priorities.push_back(priority);
	}

	//Build a list of projects from the supply table.
	//m_raw_sql.Query("select project from rawsupply group by project",true);
	//ASSERT( m_raw_sql.m_num_rows <= 1 && m_raw_sql.m_num_fields <= 1 );


	//query.Format("select file_name,size,count(*) from rawsupply where project='%s' group by file_name order by 3",v_projects[j].c_str());
	if(m_last_rawsupply_table_clean.HasTimedOut(60*60*2)){
		m_raw_sql.Query("delete from rawsupply where (time<(now()-INTERVAL 2 DAY))",false);
		m_last_rawsupply_table_clean.Refresh();
	}
	//m_raw_sql.Query("optimize table rawsupply",false);

	msg.Format("Finished running delete query to clean up the rawsupply table.",m_raw_sql.m_num_rows);
	//MessageBox(NULL,msg,"Notice.",MB_OK);
	bool query_stat=m_raw_sql.Query("select project,file_name,size,count(*) from rawsupply group by file_name",true);

	query_adds=0;
	msg.Format("Finished running main query which has %d rows in it.",m_raw_sql.m_num_rows);
	//MessageBox(NULL,msg,"Notice.",MB_OK);
	if(!query_stat){
		msg.Format("Main query failure code=%d reason=%s.",m_raw_sql.m_fail_code,m_raw_sql.m_fail_reason.c_str());
		//MessageBox(NULL,msg,"Notice.",MB_OK);
	}

	int project_finds=0;
	//loop from hell, but its better than doing a query for every project as that takes many hours
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		CString the_project=m_raw_sql.mpp_results[j][0].c_str();
		the_project.MakeLower();
		CString file_name=m_raw_sql.mpp_results[j][1].c_str();
		int the_count=atoi(m_raw_sql.mpp_results[j][3].c_str());
		int the_size=atoi(m_raw_sql.mpp_results[j][2].c_str());
		file_name.MakeLower();
		if(file_name.Find(".jpg")!=-1 || file_name.Find(".bmp")!=-1 || the_size<500000){
			continue;
		}
		//int file_name_count=atoi(m_raw_sql.mpp_results[j][3].c_str());
		if(the_count<3)  //if it is a decent file name several people will probably have it.  Filtering off a lot of crap.
			continue;

		if(the_project.Find("monitor")!=-1)// no monitor projects for now, too much work load and they aren't supposed to be protected anyways.
			continue;

		//ok now go through and see if a project exists for this item
		for(UINT i=0;i<(UINT)v_spoof_projects.size();i++){
			if(the_project.CompareNoCase(v_spoof_projects[i].c_str())==0){
				project_finds++;
				int priority=v_spoof_priorities[i];
				for(UINT hash_count=0;hash_count<(UINT)priority;hash_count++){  //we can generate up to 8 fake links per file name.  It would be nice to do more, but we have to keep this under control, and we simply don't have enough racks to do more properly.
					CString hash=GenerateRandomHash();

					CString query;
					query_adds++;
					int file_size=max(2000000+(rand()%1000000),the_size-100000+(rand()%200000));  //create a random file_size that will very likely be slighly more or less than the real file size
					while((file_size%137)!=0)  //make the size evenly divisble by 1397 for identification purposes.  This has much better odds against missing valid files than the 137 rule
						file_size++;
					query.Format("INSERT INTO spoof_table VALUES ('%s','%s',%d)",m_raw_sql.mpp_results[j][1].c_str(),(LPCSTR)hash,file_size);
					v_queries.push_back(string((LPCSTR)query));  //push this query onto the vector of queries we need to do in a moment.
				}
				break;
			}
		}	
	}

	msg.Format("Finished adding %d spoof insert queries from main query.  Project Matches=%d.  Actually doing %d inserts now...",query_adds,project_finds,v_queries.size());
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	/*
	//extract the emule projects (protected ones)
	vector <string>v_projects;
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		bool b_universal=false;
		CString target=m_raw_sql.mpp_results[j][0].c_str();
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			CString q_str=m_project_keywords_vector.v_projects[i].m_owner.c_str();
			q_str.MakeLower();
			if(q_str.Find("universal")!=-1){  //look for 'universal' to see if we need to do emule spoofing at this point.
				b_universal=true;
			}
		}	

		if(b_universal){
			v_projects.push_back(m_raw_sql.mpp_results[j][0]);
			TRACE("MetaSystem:  MetaSystem::BuildSpoofTable(void) Pushing on project %s.\n",m_raw_sql.mpp_results[j][0].c_str());
		}
	}


	//do the project info collected from the supply system.
	//for each of the projects we have isolated, lets query the rawsupply for a list of file names associated with each project
	for(UINT j=0;j<v_projects.size();j++){
		CString query;
		//we want a table off files that we can add that aren't showing up in the real world also (fake supply)
		query.Format("select file_name,size,count(*) from rawsupply where project='%s' group by file_name order by 3",v_projects[j].c_str());
		m_raw_sql.Query(query,true);


		if(m_raw_sql.m_num_fields!=3){
			ASSERT(0);
			return;
		}
	
		int file_count=0;
		//count up the files, ignoring the small ones (prolly album covers)
		for(UINT m=0;m<m_raw_sql.m_num_rows;m++){  //take top 30 file names (the 30 highest counts)
			int size=atoi(m_raw_sql.mpp_results[m][1].c_str());
			file_count++;
			if(size<800000)  //lets not count small files
				continue;
		}
		int current_files=0;

		for(UINT m=0;m<m_raw_sql.m_num_rows && current_files<60;m++){  //take top 30 file names (the 30 highest counts)
			CString file_name=m_raw_sql.mpp_results[m][0].c_str();
			int size=atoi(m_raw_sql.mpp_results[m][1].c_str());
			if(size<800000)  //skip the small files
				continue;
			//insert 50 random hashes for each file name
			current_files++;  //how many files we've worked with so far
			UINT hash_limit=max(1,400/min(file_count,60));  //make lots and lots of hashes if there are few file names
			hash_limit=max(5,hash_limit); //at least 5 hashes
			hash_limit=min(20,hash_limit); //no more than 20 hashes
			for(UINT hash_count=0;hash_count<hash_limit;hash_count++){  //we can generate up to 1800 fake links per project
				CString hash=GenerateRandomHash();

				CString query;
				int file_size=max(10,atoi(m_raw_sql.mpp_results[m][1].c_str())-500+rand()%1000);  //create a random file_size that will very likely be slighly more or less than the real file size
				query.Format("INSERT INTO spoof_table VALUES ('%s','%s',%d)",m_raw_sql.mpp_results[m][0].c_str(),(LPCSTR)hash,file_size);
				v_queries.push_back(string((LPCSTR)query));  //push this query onto the vector of queries we need to do in a moment.
			}
		}
	}
*/

//	m_raw_sql.Query( "drop TABLE spoof_table" , false);  //lets destroy this table
	m_raw_sql.Query( "CREATE TABLE spoof_table (file_name VARCHAR(255),hash VARCHAR(40),size INT)" , false);
	m_raw_sql.Query( "delete from spoof_table" , false);  //in case it couldn't drop it for some reason like if it was being accessed by a bunch of racks i don't know

	for(UINT query_index=0;query_index<v_queries.size();query_index++){
		m_raw_sql.Query(v_queries[query_index].c_str(),false);
	}

	//m_raw_sql.Query("OPTIMIZE table spoof_table",false);  //this will free harddrive space by freeing the disk space associated with the rows we deleted.
}

CString MetaSystem::GenerateRandomHash(void)
{
	static char alpha_bet[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	vector <int> v_hash;  //a vector of indexes into alpha_bet
	for(UINT i=0;i<32;i++){  //create a magikal 32 byte hash
		v_hash.push_back(rand()%16);
	}
	//make sure the last 3 groups of 2 bytes each sum up to 15.  that makes for an extremely rare chance 1/27000 of misidentification
	for(UINT i=26;i<32;i+=2){
		int sum=v_hash[i]+v_hash[i+1];
		int desired_sum=15;
		int attempt_count=0;
		while(sum!=desired_sum && attempt_count++<1000){  //attempt count is to make sure it doesn't go on to long just in case.  it is not a kluge as there is a very very very small chance that rand() could return a bad index over and over again
			int rand_index=i+(rand()&1);//rand()&1 is either 0 or 1 of course
			sum=v_hash[i]+v_hash[i+1];
			if(sum<desired_sum){
				if(v_hash[rand_index]<15)
					v_hash[rand_index]++;
			}
			else if(sum>desired_sum){
				if(v_hash[rand_index]>0)
					v_hash[rand_index]--;				
			}
		}
	}

	//convert the vector of ints into a character alphabet for easy storage into the database
	CString hash;
	for(UINT i=0;i<v_hash.size();i++){
		hash+=alpha_bet[v_hash[i]];
	}
	return hash;
}

bool MetaSystem::IsMainThreadDead(void)
{
	if(m_main_thread_check.HasTimedOut(60))
		return true;
	else
		return false;
}

void MetaSystem::UpdateProjectsDatabase(void)
{
	b_update_projects_database=true;
}

void MetaSystem::BuildSwarmTable(void)
{
	srand(timeGetTime());
	TRACE("MetaSystem:  MetaSystem::BuildSwarmTable(void)\n");
	//MessageBox(NULL,"Building Swarm Table.","Notice.",MB_OK);
	//MessageBox(NULL,"Building Spoof Table","Notice.",MB_OK);
	TinySQL m_raw_sql;
	m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306);

	vector <string>v_queries;
	int query_adds=0;

	vector <string>v_swarm_projects;  //two parallel vectors
	m_raw_sql.Query("select * from swarm_labels",true);

	int project_adds=0;
	//add all the projects in for each of the swarm labels
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string label=m_raw_sql.mpp_results[j][0].c_str();
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			if(stricmp(m_project_keywords_vector.v_projects[i].m_owner.c_str(),label.c_str())==0){
				//add this project
				project_adds++;
				v_swarm_projects.push_back(m_project_keywords_vector.v_projects[i].m_project_name);
			}
		}
	}

	CString msg;
	msg.Format("Finished adding %d labels from swarm_labels table and %d projects from them.",m_raw_sql.m_num_rows,project_adds);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	m_raw_sql.Query("select * from swarm_projects",true);
	//add all the projects in for each of the spoof labels
	project_adds=0;
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string project=m_raw_sql.mpp_results[j][0].c_str();
		v_swarm_projects.push_back(project);
		project_adds++;
	}
	msg.Format("Finished adding %d projects from swarm_projects table.",project_adds);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	//query.Format("select file_name,size,count(*) from rawsupply where project='%s' group by file_name order by 3",v_projects[j].c_str());
	m_raw_sql.Query("delete from rawsupply where (time<(now()-INTERVAL 2 DAY))",false);
	//m_raw_sql.Query("optimize table rawsupply",false);

	msg.Format("Finished running delete query to clean up the rawsupply table.",m_raw_sql.m_num_rows);
	//MessageBox(NULL,msg,"Notice.",MB_OK);
	m_raw_sql.Query("delete from swarm_table where ((time_created<(now()-INTERVAL 7 DAY)) AND project!='SWARMMAKER')",false);
	bool query_stat=m_raw_sql.Query("select project,file_name,hash,size,count(*) from rawsupply group by hash",true);

	query_adds=0;
	msg.Format("Finished running main query which has %d rows in it.",m_raw_sql.m_num_rows);
	//MessageBox(NULL,msg,"Notice.",MB_OK);
	if(!query_stat){
		msg.Format("Main query failure code=%d reason=%s.",m_raw_sql.m_fail_code,m_raw_sql.m_fail_reason.c_str());
		//MessageBox(NULL,msg,"Notice.",MB_OK);
	}

	int project_finds=0;
	//loop from hell, but its better than doing a query for every project as that takes many hours
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		CString the_project=m_raw_sql.mpp_results[j][0].c_str();
		the_project.MakeLower();
		CString file_name=m_raw_sql.mpp_results[j][1].c_str();
		CString file_hash=m_raw_sql.mpp_results[j][2].c_str();
		int the_count=atoi(m_raw_sql.mpp_results[j][4].c_str());
		int the_size=atoi(m_raw_sql.mpp_results[j][3].c_str());
		file_name.MakeLower();
		if(file_name.Find(".jpg")!=-1 || file_name.Find(".bmp")!=-1 || the_size<500000){
			continue;
		}
		//int file_name_count=atoi(m_raw_sql.mpp_results[j][3].c_str());
		//if(the_count<1)  //if it is a decent file name several people will probably have it.  Filtering off a lot of crap.
		//	continue;

		if(the_project.Find("monitor")!=-1)// no monitor projects for now, too much work load and they aren't supposed to be protected anyways.
			continue;

		//ok now go through and see if a project exists for this item
		for(UINT i=0;i<(UINT)v_swarm_projects.size();i++){	
			if(the_project.CompareNoCase(v_swarm_projects[i].c_str())==0){	
				project_finds++;

				CString query;
				query_adds++;
				//int file_size=max(2000000+(rand()%1000000),the_size-100000+(rand()%200000));  //create a random file_size that will very likely be slighly more or less than the real file size
				query.Format("INSERT INTO swarm_table VALUES ('%s','%s',%d,'%s',NULL,now())",the_project,file_name,the_size,(LPCSTR)file_hash);
				v_queries.push_back(string((LPCSTR)query));  //push this query onto the vector of queries we need to do in a moment.

				break;
			}	
		}	
	}

	msg.Format("Finished adding %d swarm insert queries from main query.  Project Matches=%d.  Actually doing %d inserts now...",query_adds,project_finds,v_queries.size());
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	//m_raw_sql.Query( "CREATE TABLE swarm_table (file_name VARCHAR(255),hash VARCHAR(40),size INT)" , false);
	//m_raw_sql.Query( "delete from spoof_table" , false);  //in case it couldn't drop it for some reason like if it was being accessed by a bunch of racks i don't know

	for(UINT query_index=0;query_index<v_queries.size();query_index++){
		m_raw_sql.Query(v_queries[query_index].c_str(),false);
	}
}

void MetaSystem::BuildFalseDecoyTable(void)
{
	srand(timeGetTime());
	TRACE("MetaMachine:  MetaSystem::BuildFalseDecoyTable(void)\n");
	//MessageBox(NULL,"Building Swarm Table.","Notice.",MB_OK);
	//MessageBox(NULL,"Building Spoof Table","Notice.",MB_OK);
	TinySQL m_raw_sql;
	m_raw_sql.Init("38.119.64.66","onsystems","tacobell","MetaMachine",3306);

	//make sure all the tables exist properly
	m_raw_sql.Query("create table false_decoy_labels (label VARCHAR(255) NOT NULL, primary key(label))",false);
	m_raw_sql.Query("create table false_decoy_uploads(project VARCHAR(240),file_name VARCHAR(255),ip VARCHAR(40),time DATE)",false);
	m_raw_sql.Query("create table false_decoy_projects (project VARCHAR(255) NOT NULL, primary key(project))",false);
	m_raw_sql.Query("create table false_decoy_table (project VARCHAR(240) NOT NULL, file_name VARCHAR(255) NOT NULL,file_size INT NOT NULL, time_created TIMESTAMP NOT NULL,primary key(project,file_name))",false);


	vector <string>v_queries;  //a global list of queries to be executed at the end of this function
	int query_adds=0;

	vector <string>v_swarm_projects;  //two parallel vectors
	m_raw_sql.Query("select * from false_decoy_labels",true);

	int project_adds=0;
	//add all the projects in for each of the swarm labels
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string label=m_raw_sql.mpp_results[j][0].c_str();
		for(UINT i=0;i<m_project_keywords_vector.v_projects.size();i++){
			if(stricmp(m_project_keywords_vector.v_projects[i].m_owner.c_str(),label.c_str())==0){
				//add this project
				project_adds++;
				v_swarm_projects.push_back(m_project_keywords_vector.v_projects[i].m_project_name);
			}
		}
	}

	CString msg;
	msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Finished adding %d labels from false_decoy_labels table and %d projects from them.\n",m_raw_sql.m_num_rows,project_adds);
	TRACE(msg);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	m_raw_sql.Query("select * from false_decoy_projects",true);
	//add all the projects in for each of the spoof labels
	project_adds=0;
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		string project=m_raw_sql.mpp_results[j][0].c_str();
		v_swarm_projects.push_back(project);
		project_adds++;
	}
	msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Finished adding %d projects from false_decoy_projects table.\n",project_adds);
	TRACE(msg);

	//query.Format("select file_name,size,count(*) from rawsupply where project='%s' group by file_name order by 3",v_projects[j].c_str());
	m_raw_sql.Query("delete from false_decoy_table where (time_created<(now()-INTERVAL 10 DAY))",false);
	if(m_last_rawsupply_table_clean.HasTimedOut(60*60*2)){
		m_raw_sql.Query("delete from rawsupply where (time<(now()-INTERVAL 2 DAY))",false);
		m_last_rawsupply_table_clean.Refresh();
	}
	//m_raw_sql.Query("optimize table rawsupply",false);

	msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Finished running delete query to clean up the rawsupply table.\n",m_raw_sql.m_num_rows);
	TRACE(msg);
	//m_raw_sql.Query("delete from swarm_table where ((time_created<(now()-INTERVAL 7 DAY)) AND project!='SWARMMAKER')",false);
	bool query_stat=m_raw_sql.Query("select project,file_name,hash,size,count(*) from rawsupply group by hash",true);

	query_adds=0;
	msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Finished running main query which has %d rows in it.\n",m_raw_sql.m_num_rows);
	TRACE(msg);
	//MessageBox(NULL,msg,"Notice.",MB_OK);
	if(!query_stat){
		msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Main query failure code=%d reason=%s.\n",m_raw_sql.m_fail_code,m_raw_sql.m_fail_reason.c_str());
		TRACE(msg);
		//MessageBox(NULL,msg,"Notice.",MB_OK);
	}

	int project_finds=0;
	//loop from hell, but its better than doing a query for every project as that takes many hours
	for(UINT j=0;j<m_raw_sql.m_num_rows;j++){
		CString the_project=m_raw_sql.mpp_results[j][0].c_str();
		the_project.MakeLower();
		CString file_name=m_raw_sql.mpp_results[j][1].c_str();
		CString file_hash=m_raw_sql.mpp_results[j][2].c_str();
		int the_count=atoi(m_raw_sql.mpp_results[j][4].c_str());
		int the_size=atoi(m_raw_sql.mpp_results[j][3].c_str());
		CString tmp=file_name;
		tmp.MakeLower();
		if(tmp.Find(".jpg")!=-1 || tmp.Find(".bmp")!=-1 || tmp.Find(".exe")!=-1 || the_size<800000){
			continue;
		}
		//int file_name_count=atoi(m_raw_sql.mpp_results[j][3].c_str());
		//if(the_count<1)  //if it is a decent file name several people will probably have it.  Filtering off a lot of crap.
		//	continue;

		if(the_project.Find("monitor")!=-1)// no monitor projects for now, too much work load and they aren't supposed to be protected anyways.
			continue;

		//ok now go through and see if a project exists for this item
		for(UINT i=0;i<(UINT)v_swarm_projects.size();i++){	
			if(the_project.CompareNoCase(v_swarm_projects[i].c_str())==0){	
				project_finds++;

				CString query;
				query_adds++;
				//int file_size=max(2000000+(rand()%1000000),the_size-100000+(rand()%200000));  //create a random file_size that will very likely be slighly more or less than the real file size
				int temp_size=the_size;
				while((temp_size%137)!=0)  //make the size evenly divisble by 1397 for identification purposes.  This has much better odds against missing valid files than the 137 rule
					temp_size++;
				query.Format("INSERT INTO false_decoy_table VALUES ('%s','%s',%d,now())",the_project,file_name,temp_size);
				v_queries.push_back(string((LPCSTR)query));  //push this query onto the vector of queries we need to do in a moment.

				break;
			}	
		}	
	}

	msg.Format("MetaMachine - Metasystem::BuildFalseDecoyTable() Finished adding %d decoy insert queries from main query.  Project Matches=%d.  Actually doing %d inserts now...\n",query_adds,project_finds,v_queries.size());
	TRACE(msg);
	//MessageBox(NULL,msg,"Notice.",MB_OK);

	//m_raw_sql.Query( "CREATE TABLE swarm_table (file_name VARCHAR(255),hash VARCHAR(40),size INT)" , false);
	//m_raw_sql.Query( "delete from spoof_table" , false);  //in case it couldn't drop it for some reason like if it was being accessed by a bunch of racks i don't know

	for(UINT query_index=0;query_index<v_queries.size();query_index++){
		m_raw_sql.Query(v_queries[query_index].c_str(),false);
	}
}
