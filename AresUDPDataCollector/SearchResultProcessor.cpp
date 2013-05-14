#include "StdAfx.h"
#include "searchresultprocessor.h"
#include "..\tkcom\TinySQL.h"
#include "SearchJob.h"

SearchResultProcessor::SearchResultProcessor(void)
{
}

SearchResultProcessor::~SearchResultProcessor(void)
{
}

UINT SearchResultProcessor::Run(void)
{
	while(!this->b_killThread){
		Sleep(100);
		if(mv_results_to_process.Size()>0){

			TinySQL sql_ips;
			sql_ips.Init(RESULT_DATABASE,"onsystems","ebertsux37","midata_ip_ares",3306);

			TinySQL sql;
			sql.Init(RESULT_DATABASE,"onsystems","ebertsux37","midata",3306);

			CSingleLock lock(&m_lock,TRUE);
			//CString query="insert ignore into ares_supply values ";
			
			int count=0;
			while(mv_results_to_process.Size()>0 && !this->b_killThread){
				SearchJob *job=(SearchJob*)mv_results_to_process.Get(0);

				if(job->mv_supply_ips.size()>0){

					CString table_name=GetProjectTableName(job->m_artist.c_str()).c_str();
					CString ip_query;
					ip_query.Format("insert ignore into %s values ",table_name);
		
					CString tmp;

					CString artist=job->m_artist.c_str();
					CString album=job->m_album.c_str();
					CString track_name=job->m_track_name.c_str();

					PrepareStringForDatabase(artist);
					PrepareStringForDatabase(album);
					PrepareStringForDatabase(track_name);

					//insert the ips into the database
					for(int j=0;j<(int)job->mv_supply_ips.size();j++){
						CString tmp2;
						unsigned int ip1,ip2,ip3,ip4;
						sscanf(job->mv_supply_ips[j].c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
						UINT ip=(ip1<<0)|(ip2<<8)|(ip3<<16)|(ip4<<24);  //little endian


						if(j>0){
							tmp2.Format(",(now(),'%s','%s',%u,'%s')",album,track_name,ip,job->mv_supply_hashes[j].c_str());
						}
						else{
							tmp2.Format("(now(),'%s','%s',%u,'%s')",album,track_name,ip,job->mv_supply_hashes[j].c_str());		
						}
						ip_query+=tmp2;
					}

					if(!sql_ips.Query(ip_query,false)){
						//the query failed, lets make sure the table exists
						CString table_create;
						CString table_drop;
						table_create.Format("create table %s (timestamp DATE NOT NULL,album varchar(255),title varchar(255),ip INT NOT NULL,hash varchar(40) NOT NULL, primary key(timestamp,ip,hash))",table_name);
						table_drop.Format("drop table %s",table_name);  //use if all else fails
						
						if(!sql_ips.Query(table_create,false)){ //create or recreate the table
							//we failed to create the table, it must already exist, but be corrupt
							sql_ips.Query(table_drop,false);  //drop it
							sql_ips.Query(table_create,false);  //try to create it once more
						}
						if(!this->b_killThread)
							sql_ips.Query(ip_query,false);  //try to insert these ips again
					}

					CString query;

					query.Format("insert ignore into ares_supply values (now(),'%s','%s','%s',%u)",artist,album,track_name,3*job->mv_supply_ips.size());
					sql.Query(query,false);
					count++;
				}

				mv_results_to_process.Remove(0);
			}
			//if(count>0 && !this->b_killThread){

			//	sql.Query(query,false);
			//}
		}

		
	}
	return 0;
}

void SearchResultProcessor::AddResults(Vector* v_results)
{
	if(mv_results_to_process.Size()>0)
		return;

	CSingleLock lock(&m_lock,TRUE);
	for(int j=0;j<NUMCONBINS;j++){
		for(int i=0;i<(int)v_results[j].Size();i++)
			mv_results_to_process.Add(v_results[j].Get(i));
	}
}

