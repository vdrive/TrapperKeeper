// Buffer2000.h: interface for the Buffer2000 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Object.h"


//A buffer class built for efficient append operations.
//This class maintains a NULL char at the end of the data for convenience and safety in outputing the buffer as a string.  It doesn't include this NULL char in its length.
//The equals(=) and copy constructor (Buffer2000(const &ref)) are not overridden to protect others (in the event that they someday use this class) from their own inefficient programming practices.
class Buffer2000 : public Object  
{
private:
	UINT bsize;
	byte *buffer;
	UINT length;
    inline void VerifyCapacity(UINT size);

public:

	Buffer2000();

	//Use this contructor if you have some idea of how much data the buffer will hold.  This will reduce reallocations and copies required to grow the buffer.
	Buffer2000(UINT initial_capacity);

	virtual ~Buffer2000();	
	Buffer2000(const byte *data,UINT len);
	void WriteBytes(const byte *data,UINT len);
	void WriteByte(byte b);
	void WriteByte(byte b,UINT offset);
	void SetTo(const byte *data,UINT len);
	void Clear();

	//if you want a null character appended at the end of the string, call Write(byte b) where b is 0, after calling this function
	void WriteString(const char *str);

	void WriteDWord(DWORD val);
	void WriteWord(WORD val);
	void WriteFloat(float val);

	//Use this function with caution (It is perfectly safe to use in any way, but it makes it possible for you to leave parts of the buffer uninitialized).
	void WriteDWord(UINT offset, DWORD val);
	void WriteWord(UINT offset, WORD val);

	//Call this if you have some idea of how much data the buffer will hold.  This will reduce reallocations and copies required to grow the buffer.
	void EnsureCapacity(UINT size);

	//Call to remove a range of data from the buffer
	void RemoveRange(UINT offset, UINT count);

	//Rapid access functions (avoids typical function callstack inefficiency)
	inline const byte* GetBufferPtr(void){return buffer;}
	inline byte* GetMutableBufferPtr(void){return buffer;}
	inline UINT GetLength(){return length;}
	
	DWORD GetDWord(UINT offset);
	WORD GetWord(UINT offset);
	byte GetByte(UINT offset);
	const char* GetString(UINT offset);

	void WriteStringW(const char* str);
};