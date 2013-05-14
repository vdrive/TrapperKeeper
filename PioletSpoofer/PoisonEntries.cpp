#include "StdAfx.h"
#include "poisonentries.h"
#include "PoisonEntry.h"

PoisonEntries::PoisonEntries(void)
{
	Clear();
}

//
//
//
PoisonEntries::~PoisonEntries(void)
{
	v_poison_entries.clear();
}

//
//
//
void PoisonEntries::Clear()
{
	v_poison_entries.clear();
}

//
//
//
int PoisonEntries::GetBufferLength()
{
	int i;
	int len=0;

	len+=sizeof(int);
	for(i=0;i<(int)v_poison_entries.size();i++)
	{
		len+=(int)v_poison_entries[i].GetBufferLength();
	}

	return len;
}

//
//
//
int PoisonEntries::WriteToBuffer(char *buf)
{

	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((int *)ptr)=(int)v_poison_entries.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_poison_entries.size();i++)
	{
		ptr+=v_poison_entries[i].WriteToBuffer(ptr);
	}
	return (int)(ptr-buf);
}

//
//
//
int PoisonEntries::ReadFromBuffer(char *buf)
{
	int i;
	int len;

	Clear();

	char *ptr=buf;
	len=*((int *)ptr);
	ptr+=sizeof(int);

	for(i=0;i<len;i++)
	{
		PoisonEntry entry;
		ptr += entry.ReadFromBuffer(ptr);
		v_poison_entries.push_back(entry);
	}
	return (int)(ptr-buf);
}