#include "stdafx.h"
#include "Strings.h"

	
//
//
//
Strings::Strings()
{
	Clear();
}
	
//
//
//
Strings::~Strings()
{

}

	
//
//
//
void Strings::Clear()
{
	v_strings.clear();
}
	
//
///
//
int Strings::GetBufferLength()
{
	int i;
	int len = 0;
	len+=sizeof(unsigned int); //number of strings

	for(i=0; i<(int)v_strings.size(); i++)
	{
		len+=(int)v_strings[i].size()+1; //+1 for NULL
	}

	return len;
}
	
//
//
//
int Strings::WriteToBuffer(char *buf)
{
	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	*((unsigned int *)ptr)=(UINT)v_strings.size();
	ptr+=sizeof(unsigned int);

	for(int i=0;i<(int)v_strings.size();i++)
	{
		strcpy(ptr,v_strings[i].c_str());
		ptr+=v_strings[i].size()+1;	// +1 for NULL
	}

	return GetBufferLength();
}
	
//
//
//
int Strings::ReadFromBuffer(char *buf)
{
	char *ptr=buf;



	unsigned int num_strings=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	for(UINT i=0;i<num_strings;i++)
	{
		string new_string = ptr;
		v_strings.push_back(new_string);	
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	return GetBufferLength();
}


