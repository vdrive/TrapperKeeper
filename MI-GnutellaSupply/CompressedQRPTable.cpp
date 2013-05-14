// CompressedQRPTable.cpp

#include "stdafx.h"
#include "CompressedQRPTable.h"

//
//
//
CompressedQRPTable::CompressedQRPTable()
{
	p_data=NULL;

	Clear();
}

//
//
//
CompressedQRPTable::~CompressedQRPTable()
{
	Clear();
}

//
//
//
CompressedQRPTable::CompressedQRPTable(CompressedQRPTable &table)
{
	p_data=NULL;

	SetData(table.GetData(),table.GetLen());
}

//
//
//
void CompressedQRPTable::Clear()
{
	if(p_data!=NULL)
	{
		delete [] p_data;
		p_data=NULL;
	}
	
	m_data_len=0;
}

//
//
//
void CompressedQRPTable::SetData(char *data,unsigned int data_len)
{
	Clear();

	p_data=new char[data_len];
	memcpy(p_data,data,data_len);
	m_data_len=data_len;
}

//
//
//
char *CompressedQRPTable::GetData()
{
	return p_data;
}

//
//
//
unsigned int CompressedQRPTable::GetLen()
{
	return m_data_len;
}

//
//
//
CompressedQRPTable& CompressedQRPTable::operator=(CompressedQRPTable &table)
{
	SetData(table.GetData(),table.GetLen());
	return *this;
}