// DatabaseItem.cpp

#include "stdafx.h"
#include "DatabaseItem.h"

//
//
//
DatabaseItem::DatabaseItem()
{
	m_id=0;
}

//
//
//
DatabaseItem::~DatabaseItem()
{
	// Free memory
	unsigned int i;
	for(i=0;i<v_columns.size();i++)
	{
		delete [] v_columns[i];
	}
}

//
// copy constructor
//
DatabaseItem::DatabaseItem(const DatabaseItem &item)
{
	unsigned int i;

	m_id=item.m_id;

	for(i=0;i<item.v_columns.size();i++)
	{
		char *ptr=new char[strlen(item.v_columns[i])+1];
		strcpy(ptr,item.v_columns[i]);
		v_columns.push_back(ptr);
	}
}

//
// assignment operator
//
DatabaseItem& DatabaseItem::operator=(const DatabaseItem &item)
{
	unsigned int i;

	m_id=item.m_id;

	// Free memory (if needed)
	for(i=0;i<v_columns.size();i++)
	{
		delete [] v_columns[i];
	}
	v_columns.clear();	

	// Copy columns
	for(i=0;i<item.v_columns.size();i++)
	{
		char *ptr=new char[strlen(item.v_columns[i])+1];
		strcpy(ptr,item.v_columns[i]);
		v_columns.push_back(ptr);
	}

	return *this;
}
/*
//
// less than operator
//
bool DatabaseItem::operator<(const DatabaseItem &item) const
{
	unsigned int ip1,ip2;
	sscanf(v_columns[0],"%u",&ip1);
	sscanf(item.v_columns[0],"%u",&ip2);

	if(ip1<ip2)
	{
		return true;
	}
	else
	{
		return false;
	}
}
*/
// Reading Data

//
//
//
unsigned int DatabaseItem::ID()
{
	return m_id;
}

//
//
//
char *DatabaseItem::Column(unsigned int col)
{
	return v_columns[col];
}

// Writing Data

//
//
//
void DatabaseItem::ID(unsigned int id)
{
	m_id=id;
}

//
//
//
void DatabaseItem::AddColumn(char *buf)
{
	char *ptr=new char[strlen(buf)+1];
	strcpy(ptr,buf);
	v_columns.push_back(ptr);
}
