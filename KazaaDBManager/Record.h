// Record.h: interface for the Entry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(Record_H)
#define Record_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Record
{
	enum record_type
	{
		EMPTY,
		DWORD,
		TWODWORD,
		STRING,
		HASH
	};

public:
	Record();
	void Clear();
	bool WriteToFile(CFile * File);
	int GetSize();
	void Insert(int tag,int num);
	void Insert(int tag,int num, int num2);
	void Insert(int tag,char * entry);


	record_type m_type;
	int m_tag;
	char m_stringentry[4096+1];
	int m_intentry1;
	int m_intentry2;

private:
	void CalculateSize();

	int m_size;
};

#endif // !defined Record
