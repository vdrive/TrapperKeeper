#pragma once

struct KazaaControllerHeader
{
	enum op_code
	{
		//from source
		Source_Init = 6577,
		Source_Supernodes_Reply,
		Source_Kill_All_Kazaa,
		Source_Launch_All_Kazaa,
		Source_Restart_All_Kazaa,
		Source_Number_Of_Kazaa_To_Run,
		Source_Renew_Supernode,
		Source_Ran_Out_Of_Supernodes,
		Source_Request_Current_Supernodes_list,

		//from dest
		Dest_Init_Response,
		Dest_Supernode_Request,
		Dest_Request_New_Supernode,
		Dest_Current_Supernodes_list,
		Dest_Check_New_Supernode,
		Dest_Remove_Supernode,

		//from other Dlls
		Request_Kazaa_Number,
		Request_All_Supernodes,
	};
	op_code op;
	unsigned int size;
};
