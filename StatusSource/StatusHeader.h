#pragma once

struct StatusHeader
{
	enum op_code
	{
		//from Dest,
		Status_Request = 7895,
		Kill_All_Kazaa,
		Launch_All_Kazaa,
		Restart_All_Kazaa,
		Number_Of_Kazaa_To_Run,
		Restart_Computer,
		System_Info_Request,

		//from source
		Status_Reply,
		System_Info_Reply,
		Current_Number_Of_Kazaa_To_Run,

		//additional ops
		Num_Kazaa_Uploads_Request,
		Num_Kazaa_Uploads,
		Request_Shared_Files,
		Shared_Files_Reply
	};
	op_code op;
	unsigned int size;
};