#pragma once
#include "..\tkcom\object.h"

class ProcessorJob : public Object
{
public:
	ProcessorJob(byte *data_in);
	~ProcessorJob(void);

#ifdef SUPERNODE
	byte m_eighty_in[0x80];  //80 in hex = 128 bytes
#else
	byte m_sixteen_in[16];
#endif

	byte m_twenty_out[20];
	bool mb_done;
};
