#pragma once
#include "..\SamplePlugIn\Interface.h"

class KazaaLauncherInterface :	protected Interface
{
public:
	KazaaLauncherInterface(void);
	~KazaaLauncherInterface(void);
	
	int RequestKazaaNumber();		//returns how many kazaas are running on this computer
	bool KillAllKazaa();			//stop launching and kill all kazaas, returns true if successful
	bool ResumeLaunchingKazaa();	//Resume to launch kazaa, returns true if successful
	bool SetMaxKazaaNumber(UINT max_kazaa);		//set the maximum number of kazaa it can run
	bool SetKazaaLaunchingInterval(int seconds = 60);	//set the launching interval (default is 1 minute)
	bool DisableMinimizingKazaaWindows();
	bool CheckKazaaConnectionStatus(int seconds);

private:
	//Called in response to data incoming from the KazaaLauncher
	bool InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data);
};
