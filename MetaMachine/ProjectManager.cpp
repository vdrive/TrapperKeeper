//AUTHOR:  Ty Heath
//LAST MODIFIED: 9.26.2003
//PURPOSE:  DCMaster Protocol

#include "StdAfx.h"
#include "projectmanager.h"


ProjectManager::ProjectManager()
{
	mb_projects_changed=true;
}

ProjectManager::~ProjectManager(void)
{

}

void ProjectManager::OnComReceive(const char* source_ip, byte* data, UINT data_length)
{
	DCHeader *iheader=(DCHeader*)data;
	if(iheader->op==DCHeader::Init){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  Init Header.\n");
		DCHeader oheader;
		oheader.op=DCHeader::Emule_DC_Init_Response;
		oheader.size=sizeof(DCHeader);
		mp_com.SendUnreliableData((char*)source_ip,&oheader,sizeof(DCHeader));
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

			//find the corresponding project in pc
			bool b_found=false;
			for(UINT j=0;j<pc.v_checksums.size();j++){
				if(stricmp(pc.v_checksums[j].m_project_name.c_str(),m_project_keywords_vector.v_projects[i].m_project_name.c_str())==0){
					b_found=true;
					break;
				}
			}

			if(!b_found){  //did we find this project si in the vector of checksums that the controller gave us?
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

			mp_com.SendReliableData((char*)source_ip,buf,len+sizeof(DCHeader));
			delete[] buf;
		}

		SaveChecksumsToDisk();
	}

	if(iheader->op==DCHeader::ProjectKeywords){
		TRACE("MetaMachine:  MetaSystem::ComDataReceived()  ProjectKeywords Header.\n");
		ProjectKeywordsVector v_project_keywords;
		v_project_keywords.ReadFromBuffer((char*)(data+sizeof(DCHeader)));
		
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
		mb_projects_changed=true;
	}	
}

void ProjectManager::SaveChecksumsToDisk(void)
{
	CString path=m_file_path.c_str();
	path+="\\checksums.txt";
	CFile file_checksums(path,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

	//save checksums
	UINT len=m_project_checksums.GetBufferLength();
	char *buf=new char[len];
	m_project_checksums.WriteToBuffer(buf);
	file_checksums.Write(buf,len);
	delete []buf;
}

void ProjectManager::SaveKeywordsToDisk(void)
{
	CString path=m_file_path.c_str();
	path+="\\keywords.txt";
	CFile file_keywords(path,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
	
	//save keywords
	UINT len=m_project_keywords_vector.GetBufferLength();
	char *buf=new char[len];
	m_project_keywords_vector.WriteToBuffer(buf);
	file_keywords.Write(buf,len);
	delete []buf;
}

void ProjectManager::LoadControllerInfo(void)
{
	OFSTRUCT tstruct;
	CString path=m_file_path.c_str();
	path+="\\checksums.txt";	
	if(HFILE_ERROR!=OpenFile( path,   &tstruct,  	OF_EXIST	)){ //avoid the exception thrown by cfile
		CFile file_keywords(path,CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_keywords.GetLength();
		char *buf=new char[len];
		file_keywords.Read(buf,len);
		m_project_keywords_vector.ReadFromBuffer(buf);
		delete []buf;
	}
	path=m_file_path.c_str();
	path+="\\keywords.txt";
	if(HFILE_ERROR!=OpenFile( path,   &tstruct,  	OF_EXIST	)){  //avoid the exception thrown by cfile
		CFile file_checksums(path,CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_checksums.GetLength();
		char *buf=new char[len];
		file_checksums.Read(buf,len);
		m_project_checksums.ReadFromBuffer(buf);
		delete []buf;
	}
}

void ProjectManager::DllStart(Dll* p_dll,const char* app_name)
{
	CString tmp;
	tmp.Format("c:\\DCMaster Info\\%s",app_name);
	CreateDirectory(tmp,NULL);
	m_file_path=(LPCSTR)(tmp);
	mp_com.SetManager(this);
	LoadControllerInfo();
	mp_com.Register(p_dll,35);
}

bool ProjectManager::HaveProjectsChanged(void)
{
	bool stat=mb_projects_changed;
	mb_projects_changed=false;
	return stat;
}
