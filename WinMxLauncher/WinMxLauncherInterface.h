#pragma once
#include "..\sampleplugin\interface.h"

class WinMxLauncherInterface :
	protected  Interface
{
public:
	WinMxLauncherInterface(void);
	~WinMxLauncherInterface(void);

	int RequestWinMxNumber();				//returns how many kazaas are running on this computer
	bool StopRunningWinMx();
	bool RestartAllWinMx();					//Kill all running winmx and restart launch tehm
	bool SetMaxWinMxNumber(UINT max);		//set the maximum number of winmx it can run

private:
	//Called in response to data incoming from the WinMxLauncher
	bool InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data);
};
