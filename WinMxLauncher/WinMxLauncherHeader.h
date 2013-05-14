#pragma once

struct WinMxLauncherHeader
{
	enum op_code
	{
		Request_WinMx_Number = 0,
		Restart_All_WinMx,
		Set_Number_Of_WinMx_To_Run,
		Stop_Running_WinMx
	};
	op_code op;
	unsigned int size;
};
