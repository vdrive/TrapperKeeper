#include "StdAfx.h"
#include "processorjob.h"

ProcessorJob::ProcessorJob(byte *data_in)
{
#ifdef SUPERNODE
	memcpy(m_eighty_in,data_in,0x80);
#else
	memcpy(m_sixteen_in,data_in,16);
#endif

	memset(m_twenty_out,0,20);
	mb_done=false;
}

ProcessorJob::~ProcessorJob(void)
{
}
