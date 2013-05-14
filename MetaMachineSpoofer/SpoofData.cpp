#include "StdAfx.h"
#include "spoofdata.h"
#include "..\tkcom\tinysql.h"
#include "spoof.h"

SpoofData::SpoofData(void)
{
	mb_inited=false;
}

SpoofData::~SpoofData(void)
{
}

void SpoofData::Update(void)
{
	if(mb_inited && !m_last_update.HasTimedOut(8*60*60)){
		return;
	}
	
	mb_inited=true;

	/*
	CSingleLock lock(&m_data_lock,TRUE);
	m_last_update.Refresh();

	mv_spoofs.Clear();
	
	CString query;
	query.Format("select project,file_name,file_size from spoof_table2 order by rand() limit 20000");

	TinySQL sql;
	sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);
	sql.Query(query,true);	

	vector <string> v_projects;
	vector <int> v_counts;

	for(int i=(int)sql.m_num_rows-1;i>=0;i--){
		const char* project=sql.mpp_results[i][0].c_str();
		bool b_too_many=false;
		bool b_found=false;
		for(int j=0;j<(int)v_projects.size();j++){
			if(stricmp(v_projects[j].c_str(),project)==0){
				v_counts[j]++;  
				b_found=true;
				if(v_counts[j]>400){  //only allow 400 spoofable files per project, so some projects don't completely drown out others
					b_too_many=true;
				}
				break;
			}
		}
		if(!b_found){  //if we don't have a count going for this project, then add one
			v_projects.push_back(string(project));
			v_counts.push_back(1);
		}

		if(!b_too_many){
			mv_spoofs.Add(new Spoof(sql.mpp_results[i][1].c_str(),atoi(sql.mpp_results[i][2].c_str())));
		}
	}*/
}