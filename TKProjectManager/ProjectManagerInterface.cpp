#include "StdAfx.h"
#include "projectmanagerinterface.h"
#include "DllInterface.h"
#include "ProjectManagerInterfaceHeader.h"
#include "../DllLoader/AppID.h"


ProjectManagerInterface::ProjectManagerInterface(void)
{
}

//
//
//
ProjectManagerInterface::~ProjectManagerInterface(void)
{
}

//
//
//
bool ProjectManagerInterface::InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data)
{
	return false; //pass this data to the main app
}

//
//
//
void ProjectManagerInterface::Init(DCHeader::op_code client_id)
{
	byte buf[sizeof(ProjectManagerInterfaceHeader)+sizeof(DCHeader::op_code)];
	ProjectManagerInterfaceHeader* header = (ProjectManagerInterfaceHeader*)buf;
	header->op = ProjectManagerInterfaceHeader::Init;
	header->size = sizeof(DCHeader::op_code);
	memcpy(&buf[sizeof(ProjectManagerInterfaceHeader)],&client_id,sizeof(DCHeader::op_code));
	AppID app_id;
	app_id.m_app_id = 21010; //ProjectManager app id
	app_id.m_version = 0x00000001;
	DllInterface::SendData(app_id,buf);
}

//
//
//
bool ProjectManagerInterface::HaveProjectsChanged()
{
	byte buf[sizeof(ProjectManagerInterfaceHeader)];
	ProjectManagerInterfaceHeader* header = (ProjectManagerInterfaceHeader*)buf;
	header->op = ProjectManagerInterfaceHeader::HaveProjectsChanged;
	header->size = 0;
	AppID app_id;
	app_id.m_app_id = 21010; //ProjectManager app id
	app_id.m_version = 0x00000001;
	bool ret=false;
	DllInterface::SendData(app_id,buf,(void*)&ret);
	return ret;
}

//
//
//
bool ProjectManagerInterface::IsReady()
{
	byte buf[sizeof(ProjectManagerInterfaceHeader)];
	ProjectManagerInterfaceHeader* header = (ProjectManagerInterfaceHeader*)buf;
	header->op = ProjectManagerInterfaceHeader::IsReady;
	header->size = 0;
	AppID app_id;
	app_id.m_app_id = 21010; //ProjectManager app id
	app_id.m_version = 0x00000001;
	bool ret=false;
	DllInterface::SendData(app_id,buf,(void*)&ret);
	return ret;
}

//
//
//
void ProjectManagerInterface::GetProjectsCopy(ProjectKeywordsVector &projects)
{
	byte buf[sizeof(ProjectManagerInterfaceHeader)];
	ProjectManagerInterfaceHeader* header = (ProjectManagerInterfaceHeader*)buf;
	header->op = ProjectManagerInterfaceHeader::GetProjectsCopy;
	header->size = 0;
	AppID app_id;
	app_id.m_app_id = 21010; //ProjectManager app id
	app_id.m_version = 0x00000001;
	DllInterface::SendData(app_id,buf,(void*)&projects);
}
