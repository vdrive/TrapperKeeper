// Record.cpp: implementation of the Record class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Record.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Record::Record()
{

}


//
//
//

bool Record::WriteToFile(CFile * file)
{

	switch (m_type)
	{
		case (Record::DWORD):
		{
			file->Write(&m_tag,sizeof(int));
			file->Write(&m_size,sizeof(int));
			file->Write(&m_intentry1,sizeof(int));	
			break;
		}
		case (Record::TWODWORD):
		{
			file->Write(&m_tag,sizeof(int));
			file->Write(&m_size,sizeof(int));
			file->Write(&m_intentry1,sizeof(int));
			file->Write(&m_intentry2,sizeof(int));
			break;
		}
		case (Record::STRING):
		{
			file->Write(&m_tag,sizeof(int));
			file->Write(&m_size,sizeof(int));
			file->Write(m_stringentry,(UINT)strlen(m_stringentry)+1);
			break;
		}
		
		case (Record::HASH):
		{
			file->Write(&m_tag,sizeof(int));
			file->Write(&m_size,sizeof(int));
			file->Write(m_stringentry,20);
			break;
		}
		
	}

	return true;
}


//
//
//

//Set Size dependant on the type of Record.
void Record::CalculateSize()
{
	switch (m_type)
	{
		case (Record::DWORD):
		{	
			//m_size = (sizeof(int)*3);
			m_size = (sizeof(int));
			break;
		}
		case (Record::TWODWORD):
		{
			//m_size = (sizeof(int)*4);
			m_size = (sizeof(int)*2);
			break;
		}
		case (Record::STRING):
		{
			//m_size = ((sizeof(int)*2) + strlen(m_stringentry)+1);
			m_size = ((int)strlen(m_stringentry)+1);
			break;
		}
		//  Removing the hash special case and making it the same as a string
		case (Record::HASH):
		{
			//m_size = ((sizeof(int)*2) + strlen(m_hash)+1);
			m_size = (20);
			break;
		}
		
	}
}

//
//
//

int Record::GetSize()
{
	return m_size;
}

void Record::Clear()
{
	m_type = Record::EMPTY;
	m_tag = 0;
	m_size = 0;
	m_stringentry[256+1] = NULL;
	m_intentry1 = 0;
	m_intentry2 = 0;
	//m_hash[20+1] = NULL;
}


// Functions to set the type and data dependant on the type of data inserted
void Record::Insert(int tag, int num)
{
	m_tag = tag;
	m_type = Record::DWORD;
	m_intentry1 = num;
	CalculateSize();
}

void Record::Insert(int tag, int num, int num2)
{
	m_tag = tag;
	m_type = Record::TWODWORD;
	m_intentry1 = num;
	m_intentry2 = num2;
	CalculateSize();
}

void Record::Insert(int tag,char * entry)
{
	m_tag = tag;
	m_type = Record::STRING;
	if (tag == 3)
	{
		m_type = Record::HASH;
	}
	strcpy(m_stringentry, entry);
	CalculateSize();
}