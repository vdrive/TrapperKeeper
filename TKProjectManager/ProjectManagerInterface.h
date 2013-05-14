#pragma once
#include "c:\cvs\mdproj\trapperkeeper\sampleplugin\interface.h"
#include "c:\cvs\mdproj\trapperkeeper\DCMaster\DCHeader.h"
#include "..\DCMaster\ProjectKeywordsVector.h"

class ProjectManagerInterface :	protected Interface
{
public:
	ProjectManagerInterface(void);
	~ProjectManagerInterface(void);

	void Init(DCHeader::op_code cliend_id); //identify yourself what to respond to DCMaster Init message
	bool HaveProjectsChanged();			//Any changes to projects setting since last check?
	void GetProjectsCopy(ProjectKeywordsVector &projects);
	bool IsReady();

private:
	//Called in response to data incoming from ProjectManager
	bool InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data);
};
