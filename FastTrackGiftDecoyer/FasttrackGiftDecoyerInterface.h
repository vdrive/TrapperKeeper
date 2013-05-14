#pragma once
#include "..\SamplePlugIn\Interface.h"

class FasttrackGiftDecoyerInterface :	protected Interface
{
public:
	FasttrackGiftDecoyerInterface(void);
	~FasttrackGiftDecoyerInterface(void);
	
	bool StopSharing();
	bool StartSharing();

private:
	//Called in response to data incoming from the KazaaLauncher
	bool InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data);
};
