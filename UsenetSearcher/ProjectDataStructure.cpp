//ProjectDataStructure.h

#include "stdafx.h"
#include "ProjectDataStructure.h"


	
//
//
//
int ProjectDataStructure::GetBufferLength()
{
	return 0;
}


//
//
//
int ProjectDataStructure::WriteToBuffer(char *buf)
{
	return GetBufferLength();	
}

//
//
//
ProjectChecksum ProjectDataStructure::CalculateChecksum()
{
	char *buf=new char[GetBufferLength()];
	WriteToBuffer(buf);

	ProjectChecksum checksum;
	checksum.Checksum(buf,GetBufferLength());
/*
	for(i=0;i<GetBufferLength();i++)
	{
		checksum+=buf[i];
	}
*/
	delete [] buf;

	return checksum;
}