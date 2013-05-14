#pragma once


struct SwarmerComHeader
{
	enum op_code
	{
		Swarmer_Init,
		Ping_Dests,
		Pong_Source,
		Project_Record,
		Records_Complete,
		File_Sent,
		
	};
	op_code op;
	unsigned int size;
};