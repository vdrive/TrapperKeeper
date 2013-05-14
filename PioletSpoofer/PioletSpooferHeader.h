#pragma once

struct PioletSpooferHeader
{
	enum op_code
	{
		//from source
		Poisoner_Init = 500,
		Poisoner_Init_Response,
		PoisonEntry = 600,
		Poisoner_DatabaseInfo = 700,		//database's host,user,password
	};
	op_code op;
	unsigned int size;
};