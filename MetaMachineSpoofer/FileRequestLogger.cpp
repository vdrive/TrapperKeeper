#include "StdAfx.h"
#include "filerequestlogger.h"
#include "..\tkcom\TinySQL.h"

FileRequestLogger::FileRequestLogger(void)
{
}

FileRequestLogger::~FileRequestLogger(void)
{
}

UINT FileRequestLogger::Run(){
	while(!this->b_killThread){
		vector <string> v_requests;
		while(mv_false_decoy_requests.Size()>0){
			FalseDecoyRequest *rq=(FalseDecoyRequest*)mv_false_decoy_requests.Get(0);
			CString tmp;
			tmp.Format("('%s','%s','%s',now())",rq->m_project.c_str(),rq->m_file_name.c_str(),rq->m_ip.c_str());
			v_requests.push_back(string((LPCSTR)tmp));
			mv_false_decoy_requests.Remove(0);
		}

		if(v_requests.size()>0){
			int count=0;
			TinySQL sql;
			sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);
			CString query="insert IGNORE into false_decoy_uploads (project,file_name,ip,time) VALUES ";
			for(int i=0;i<(int)v_requests.size();i++){
				if(i==0){
					query+=v_requests[i].c_str();
				}
				else{  //we are appending more inserts so we need to add a comma
					CString tmp=",";
					tmp+=v_requests[i].c_str();
					query+=tmp;
				}
			}
			sql.Query(query,false);  //run an insert of these requests
			
		}
		for(int i=0;i<20000 && !this->b_killThread;i++){
			Sleep(100);  //sleep 2000 seconds and then log some requests
		}
	}

	return 0;
}

void FileRequestLogger::LogFalseDecoyRequest(const char* project, const char* ip, const char* file_name)
{
	//TRACE("Logging File Request project=%s, file_name=%s, ip=%s\n",project,file_name,ip);
	mv_false_decoy_requests.Add(new FalseDecoyRequest(project,ip,file_name));
}
