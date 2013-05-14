//AUTHOR:  Ty Heath
//LAST MODIFIED: 9.26.2003
//PURPOSE:  DCMaster Protocol

#include "StdAfx.h"
#include "projectmanager.h"

ProjectManager::ProjectManager()
{
	mb_projects_changed=true;
	m_last_ping=CTime(1980,1,1,1,1,1);
}

ProjectManager::~ProjectManager(void)
{

}

void ProjectManager::OnComReceive(const char* source_ip, byte* data, UINT data_length)
{
	DCHeader *iheader=(DCHeader*)data;
	if(iheader->op==DCHeader::Init){
		CSingleLock lock(&m_lock,TRUE);

		TRACE("ProjectManager::ComDataReceived()  Init Header.\n");
		DCHeader oheader;
		oheader.op=DCHeader::SoulSeek_Spoofer_Init_Response;
		oheader.size=sizeof(DCHeader);
		mp_com.SendUnreliableData((char*)source_ip,&oheader,sizeof(DCHeader));

		if(!IsReady()) {
			oheader.op=DCHeader::RequestProjectChecksums;
			oheader.size=sizeof(DCHeader);
			mp_com.SendUnreliableData((char*)source_ip,&oheader,sizeof(DCHeader));
		}

		m_last_ping=CTime::GetCurrentTime();
	}
	else if(iheader->op==DCHeader::DatabaseInfo){
		CSingleLock lock(&m_lock,TRUE);
		TRACE("ProjectManager::ComDataReceived()  DataBaseInfo Header.\n");
		m_data_base_info.ReadFromBuffer((char*)(data+sizeof(DCHeader)));
	}
	else if(iheader->op==DCHeader::ProjectChecksums){
		CSingleLock lock(&m_lock,TRUE);
		TRACE("ProjectManager::ComDataReceived()  ProjectChecksums Header.\n");
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
				TRACE("ProjectManager::ComDataReceived() Requesting update for %s.\n",ref->m_project_name.c_str());
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
				TRACE("ProjectManager::ComDataReceived() num_projects: %d.\n", m_project_keywords_vector.v_projects.size());
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
	else if(iheader->op==DCHeader::ProjectKeywords){
		CSingleLock lock(&m_lock,TRUE);
		TRACE("ProjectManager::ComDataReceived()  ProjectKeywords Header.\n");
		ProjectKeywordsVector v_project_keywords;
		v_project_keywords.ReadFromBuffer((char*)(data+sizeof(DCHeader)));
		
		for(UINT i=0;i<v_project_keywords.v_projects.size();i++){  //for each updated project
			//find the corresponding project in our list, if it exists
			bool b_found=false;
			string project_name = v_project_keywords.v_projects[i].m_project_name;
			bool supply = v_project_keywords.v_projects[i].m_soulseek_supply_enabled;
			bool spoofing = v_project_keywords.v_projects[i].m_soulseek_spoofing_enabled;
			for(UINT j=0;j<m_project_keywords_vector.v_projects.size();j++){
				if(stricmp(v_project_keywords.v_projects[i].m_project_name.c_str(),m_project_keywords_vector.v_projects[j].m_project_name.c_str())==0){
					b_found=true;
					TRACE("MetaMachine:  MetaSystem::ComDataReceived() Updating project %s.\n",v_project_keywords.v_projects[i].m_project_name.c_str());
					m_project_keywords_vector.v_projects[j]=v_project_keywords.v_projects[i];
				}
			}

			if(!b_found){  //we don't have this project, create it
				TRACE("ProjectManager::ComDataReceived() Adding project %s.\n",v_project_keywords.v_projects[i].m_project_name.c_str());
				m_project_keywords_vector.v_projects.push_back(v_project_keywords.v_projects[i]);
			}
		}
		SaveKeywordsToDisk();
		mb_projects_changed=true;
		ProjectKeywordsVector projects;
		GetProjectsCopy(projects);
	}	
}

void ProjectManager::SaveChecksumsToDisk(void)
{
	CSingleLock lock(&m_lock,TRUE);
	TRACE("ProjectManager::SaveChecksumsToDisk() BEGIN\n");
	CString path=m_file_path.c_str();
	path+="\\checksums.txt";
	CFile file_checksums(path,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

	//save checksums
	UINT len=m_project_checksums.GetBufferLength();
	char *buf=new char[len];
	m_project_checksums.WriteToBuffer(buf);
	file_checksums.Write(buf,len);
	delete []buf;
	TRACE("ProjectManager::SaveChecksumsToDisk() END\n");
}

void ProjectManager::SaveKeywordsToDisk(void)
{
	CSingleLock lock(&m_lock,TRUE);
	TRACE("ProjectManager::SaveKeywordsToDisk() BEGIN\n");
	CString path=m_file_path.c_str();
	path+="\\keywords.txt";
	CFile file_keywords(path,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
	
	//save keywords
	UINT len=m_project_keywords_vector.GetBufferLength();
	char *buf=new char[len];
	m_project_keywords_vector.WriteToBuffer(buf);
	file_keywords.Write(buf,len);
	delete []buf;
	TRACE("ProjectManager::SaveKeywordsToDisk() END\n");
}

void ProjectManager::LoadControllerInfo(void)
{
	CSingleLock lock(&m_lock,TRUE);
	TRACE("ProjectManager::LoadControllerInfo() BEGIN\n");
	OFSTRUCT tstruct;
	CString path=m_file_path.c_str();
	path+="\\keywords.txt";
	if(HFILE_ERROR!=OpenFile( path,   &tstruct,  	OF_EXIST	)){ //avoid the exception thrown by cfile
		CFile file_keywords(path,CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_keywords.GetLength();
		char *buf=new char[len];
		file_keywords.Read(buf,len);
		m_project_keywords_vector.ReadFromBuffer(buf);
		delete []buf;
	}
	path=m_file_path.c_str();
	path+="\\checksums.txt";	
	if(HFILE_ERROR!=OpenFile( path,   &tstruct,  	OF_EXIST	)){  //avoid the exception thrown by cfile
		CFile file_checksums(path,CFile::modeRead|CFile::typeBinary);
		UINT len=(UINT)file_checksums.GetLength();
		char *buf=new char[len];
		file_checksums.Read(buf,len);
		m_project_checksums.ReadFromBuffer(buf);
		delete []buf;
	}
	TRACE("ProjectManager::LoadControllerInfo() END\n");
}

void ProjectManager::DllStart(Dll* p_dll,const char* app_name)
{
	CString tmp;
	tmp.Format("c:\\DCMaster Info\\%s",app_name);
	CreateDirectory("c:\\DCMaster Info",NULL);
	CreateDirectory(tmp,NULL);
	m_file_path=(LPCSTR)(tmp);
	mp_com.SetManager(this);
	LoadControllerInfo();
	mp_com.Register(p_dll,35);
}

bool ProjectManager::HaveProjectsChanged(void)
{
	CSingleLock lock(&m_lock,TRUE);
	bool stat=mb_projects_changed;
	mb_projects_changed=false;
	return stat;
}

//returns a copy of the projects object with all the keyword/killword strings converted to lower case and inactive projects removed
void ProjectManager::GetProjectsCopy(ProjectKeywordsVector &projects)
{
	TRACE("ProjectManager::GetProjectsCopy() BEGIN\n");
	CSingleLock lock(&m_lock,TRUE);

	byte *buf=new byte[m_project_keywords_vector.GetBufferLength()];

	m_project_keywords_vector.WriteToBuffer((char*)buf);

	projects.ReadFromBuffer((char*)buf);

	delete []buf;

	for(int i = 0; i < projects.v_projects.size(); i ++) {
			TRACE("PROJECT NAME: %s/%s\n", m_project_keywords_vector.v_projects[i].m_project_name.c_str(), projects.v_projects[i].m_project_name.c_str());
		//	TRACE("SUPPLY: %d/%d\n", m_project_keywords_vector.v_projects[i].m_soulseek_supply_enabled, projects.v_projects[i].m_soulseek_supply_enabled);
			TRACE("SPOOF: %d/%d\n", m_project_keywords_vector.v_projects[i].m_soulseek_spoofing_enabled, projects.v_projects[i].m_soulseek_spoofing_enabled);

	}

	TRACE("ProjectManager::GetProjectsCopy() END\n");
}

//returns true if the system is confident that is has the current list of projects
bool ProjectManager::IsReady(void)
{
	CSingleLock lock(&m_lock,TRUE);
	if(m_project_keywords_vector.v_projects.size()<1 || m_last_ping<(CTime::GetCurrentTime()-CTimeSpan(0,5,0,0)))  //if no ping for x hours, we can't assume we have the current projects
		return false;
	else
		return true;
}

void ProjectManager::ConvertToLowerCase(string& tmp)
{
	for(int i=0;i<(int)tmp.size();i++){
		tmp[i]=(char)tolower(tmp[i]);
	}
}
