//AUTHOR:  Ty Heath
//EDITED BY: Ivan Kwok
//LAST MODIFIED: 1.28.2005
//PURPOSE:  DCMaster Protocol

#include "StdAfx.h"
#include "projectmanager.h"
#include "DllInterface.h"
#include "ProjectManagerInterfaceHeader.h"

ProjectManager::ProjectManager()
{
	mb_projects_changed=true;
	m_last_ping=CTime(1980,1,1,1,1,1);
	m_opcode = DCHeader::Generic_Init_Response;
	m_first_init_from_dcmaster=true;
}

ProjectManager::~ProjectManager(void)
{

}

void ProjectManager::DllInitialize()
{
	TRACE("ProjectManager::DllInitialize()\n");
	m_dlg.Create(IDD_Project_Manager_DIALOG,CWnd::GetDesktopWindow());
	CreateDirectory("c:\\DCMaster Info",NULL);
	CreateDirectory("c:\\DCMaster Info\\Project Manager",NULL);
	m_file_path="c:\\DCMaster Info\\Project Manager";
	LoadControllerInfo();
}

void ProjectManager::DllUnInitialize()
{
	m_dlg.DestroyWindow();
}

void ProjectManager::DllStart()
{
	mp_com.SetManager(this);
	mp_com.Register(this,35);
}

void ProjectManager::DllShowGUI()
{
	m_dlg.ShowWindow(SW_SHOW);
	m_dlg.BringWindowToTop();
}

void ProjectManager::Init(AppID source, DCHeader::op_code opcode)
{
	TRACE("ProjectManager::Init()\n");
	m_source_id = source;		
	m_opcode = opcode;
}

bool ProjectManager::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
	ProjectManagerInterfaceHeader* header = (ProjectManagerInterfaceHeader*)input_data;
	byte *pData=(byte *)input_data;
	pData+=sizeof(ProjectManagerInterfaceHeader);

	switch(header->op)
	{
		case ProjectManagerInterfaceHeader::Init:
		{
			m_opcode = *(DCHeader::op_code*)pData;
			CString client="Client: ";
			switch(m_opcode)
			{
				case DCHeader::BitTorrent_Init_Response:
				{
					client+="BitTorrent";
					break;
				}
				case DCHeader::Direct_Connect_Init_Response:
				{
					client+="Direct Connect";
					break;
				}
				case DCHeader::Direct_Connect_Spoofer_Init_Response:
				{
					client+="Direct Connect Spoofer";
					break;
				}
				case DCHeader::Emule_DC_Init_Response:
				{
					client+="Emule DC";
					break;
				}
				case DCHeader::FastTrack_DC_Init_Response:
				{
					client+="Fasttrack DC";
					break;
				}
				case DCHeader::FastTrack_Spoofer_Init_Response:
				{
					client+="Fasttrack Spoofer";
					break;
				}
				case DCHeader::FastTrack_Swarmer_Init_Response:
				{
					client+="Fasttrack Swarmer";
					break;
				}
				case DCHeader::Generic_Init_Response:
				{
					client+="Generic";
					break;
				}
				case DCHeader::Gnutella_DC_Init_Response:
				{
					client+="Gnutella DC";
					break;
				}
				case DCHeader::Gnutella_Protector_Init_Response:
				{
					client+="Gnutella Protector";
					break;
				}
				case DCHeader::Gnutella_Spoofer_Spited_Init_Response:
				{
					client+="Gnutella Spoofer";
					break;
				}
				case DCHeader::Kazaa_DC_Init_Response:
				{
					client+="Kazaa DC";
					break;
				}
				case DCHeader::Overnet_DC_Init_Response:
				{
					client+="Overnet DC";
					break;
				}
				case DCHeader::Overnet_Spoofer_Init_Response:
				{
					client+="Overnet Spoofer";
					break;
				}
				case DCHeader::Piolet_DC_Init_Response:
				{
					client+="Piolet DC";
					break;
				}
				case DCHeader::Piolet_Spoofer_Init_Response:
				{
					client+="Piolet Spoofer";
					break;
				}
				case DCHeader::SoulSeek_Spoofer_Init_Response:
				{
					client+="SoulSeek Spoofer";
					break;
				}
				case DCHeader::WinMx_DC_Init_Response:
				{
					client+="WinMX DC";
					break;
				}
				case DCHeader::WinMx_Decoyer_Init_Response:
				{
					client+="WinMX Decoyer";
					break;
				}
				case DCHeader::WinMx_Spoofer_Init_Response:
				{
					client+="WinMX Spoofer";
					break;
				}
				case DCHeader::WinMx_Swarmer_Init_Response:
				{
					client+="WinMX Swarmer";
					break;
				}
				default:
				{
					m_opcode=DCHeader::Generic_Init_Response;
					client+="Generic";
				}
			}
			m_dlg.SetClient(client);
			return true;
		}
		case ProjectManagerInterfaceHeader::HaveProjectsChanged:
		{
			bool &output = *(bool*)output_data;
			output = HaveProjectsChanged();
			return true;
		}
		case ProjectManagerInterfaceHeader::IsReady:
		{
			bool &output = *(bool*)output_data;
			output = IsReady();
			return true;
		}
		case ProjectManagerInterfaceHeader::GetProjectsCopy:
		{
			ProjectKeywordsVector &output = *(ProjectKeywordsVector*)output_data;
			GetProjectsCopy(output);
			return true;
		}
	}
	return false;
}

void ProjectManager::OnComReceive(const char* source_ip, byte* data, UINT data_length)
{
	DCHeader *iheader=(DCHeader*)data;
	if(iheader->op==DCHeader::Init){
		CSingleLock lock(&m_lock,TRUE);

		TRACE("ProjectManager::ComDataReceived()  Init Header.\n");
		DCHeader oheader;
		oheader.op=m_opcode;
		oheader.size=sizeof(DCHeader);
		mp_com.SendUnreliableData((char*)source_ip,&oheader,sizeof(DCHeader));

		if(!IsReady()||m_first_init_from_dcmaster) {
			m_first_init_from_dcmaster=false;
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
		else
		{
			SaveKeywordsToDisk();
			mb_projects_changed=true;
			m_dlg.UpdateProjectKeyowrdTree(&m_project_keywords_vector.v_projects);
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
		m_dlg.UpdateProjectKeyowrdTree(&m_project_keywords_vector.v_projects);
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
	m_dlg.UpdateProjectKeyowrdTree(&m_project_keywords_vector.v_projects);
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
	TRACE("ProjectManager::GetProjectsCopy() END\n");
}

//returns true if the system is confident that is has the current list of projects
bool ProjectManager::IsReady(void)
{
	CSingleLock lock(&m_lock,TRUE);
//#ifndef _DEBUG
//	if(m_project_keywords_vector.v_projects.size()<1 || m_last_ping<(CTime::GetCurrentTime()-CTimeSpan(0,5,0,0)))  //if no ping for x hours, we can't assume we have the current projects
//#else
	if(m_project_keywords_vector.v_projects.size()<1)
//#endif
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
