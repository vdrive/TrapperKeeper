//AUTHOR:  Ty Heath
//EDITED BY: Ivan Kwok
//LAST MODIFIED: 1.28.2005
//PURPOSE:  DCMaster Protocol


#pragma once
#include "..\SamplePlugIn\dll.h"
#include "..\TKCom\TkComInterface.h"
#include "..\DCMaster\DCHeader.h"
#include "..\DCMaster\ProjectKeywordsVector.h"
#include "..\DCMaster\DataBaseInfo.h"
#include "..\DCMaster\ProjectCheckSums.h"
#include "..\DCMaster\ProjectCheckSum.h"
#include "..\DCMaster\ProjectKeywords.h"
#include "ProjectManagerDlg.h"
#include <afxmt.h>

class ProjectManager : public Dll
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

	CCriticalSection m_lock;

	ProjectKeywordsVector m_project_keywords_vector;
	CTime m_last_ping;
public:
	ProjectManager();
	~ProjectManager(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void Init(AppID source, DCHeader::op_code opcode);

	inline const char* GetCentralDBHost(){CSingleLock(&m_lock,TRUE);return m_data_base_info.m_db_host.c_str();}
	inline const char* GetCentralDBUser(){CSingleLock(&m_lock,TRUE);return m_data_base_info.m_db_user.c_str();}
	inline const char* GetCentralDBPassword(){CSingleLock(&m_lock,TRUE);return m_data_base_info.m_db_password.c_str();}
	
	bool HaveProjectsChanged(void);
	void GetProjectsCopy(ProjectKeywordsVector &projects);
	bool IsReady(void);
	void ConvertToLowerCase(string& tmp);

	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);


protected:
	AppID m_source_id;
	DCHeader::op_code m_opcode;
	CProjectManagerDlg m_dlg;
	bool m_first_init_from_dcmaster;
};
