#pragma once
#include "object.h"
#include <afxmt.h>
//AUTHOR:  Ty Heath
//DATE:  3.5.2003
//PURPOSE: 
//This class is a data object that represents a data buffer as supplied by a plugin to be sent and received.
//It handles encryption and compression as well.  It also maintains a list of destinations that it is remaining to be broadcasted to.
//#define COMDATABUFFERSAMPLE 30

class DataBuffer : public Object
{
private:
	//string m_source; //used in source tracking for plugins (the 'char* source_ip' parameter in TKComInterface::DataReceived()).  This gets set by a TKSocket when it recieves data and creates this object.
	CCriticalSection m_dest_lock;
	byte* m_data;  //pointer to data.  deallocated at destruction.  it contains all relevant information such as op_code, length, etc.
	bool mb_sealed;
	//byte m_data_sample[COMDATABUFFERSAMPLE]; //a 30 byte data sample
public:

	//UINT m_data_length; //length of data
	//UINT m_op_code; //what op_code this data represents.  essentially its type.  This is determined by plugins when they register an interface object.

	//Used for initialization when a plugin wants to send data.
	DataBuffer( byte *data , UINT data_length , UINT op_code );

	DataBuffer();
	~DataBuffer(void);

	//used by TKSocket to check if enough data has been received to recreate this object.
	int ReadFromBuffer( const char* source_ip , byte* buffer , UINT buffer_length ); 

	//Gets the amount of space in bytes that a call to WriteToBuffer would require.
	UINT GetDataLength();  

	UINT GetOpCode(void);
	const char* GetSource(void);
	byte* GetBuffer(void);
};
