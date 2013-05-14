//AUTHOR:  Ty Heath
//LAST MODIFIED: 9.26.2003
//PURPOSE:  DCMaster Protocol

#pragma once
#include "dll.h"
#include "TkComInterface.h"
#include "DCHeader.h"
#include "ProjectKeywordsVector.h"
#include "DataBaseInfo.h"
#include "ProjectCheckSums.h"
#include "ProjectCheckSum.h"
#include "ProjectKeywords.h"

class ProjectManager
{
private:
	class ProjectCom : public TKComInterface{
	public:
		ProjectManager *mp_manager;
		ProjectCom(){
		}
		void SetManager(ProjectManager* pm){
			mp_manager=pm;
		}

		void DataReceived(char *source_ip, void *data, UINT data_length){
			mp_manager->OnComReceive(source_ip,(byte*)data,data_length);
		}
	};
	ProjectCom mp_com;
	string m_file_path;
	void OnComReceive(const char* source_ip, byte* data, UINT data_length);
	friend class ProjectCom;
	DataBaseInfo m_data_base_info;
	ProjectChecksums m_project_checksums;
	bool mb_projects_changed;

	void SaveChecksumsToDisk(void);
	void SaveKeywordsToDisk(void);
	void LoadControllerInfo(void);
public:
	ProjectManager();
	~ProjectManager(void);
	inline const char* GetCentralDBHost(){return m_data_base_info.m_db_host.c_str();}
	inline const char* GetCentralDBUser(){return m_data_base_info.m_db_user.c_str();}
	inline const char* GetCentralDBPassword(){return m_data_base_info.m_db_password.c_str();}
	
	ProjectKeywordsVector m_project_keywords_vector;
	void DllStart(Dll* p_dll,const char* app_name);
	bool HaveProjectsChanged(void);
};
