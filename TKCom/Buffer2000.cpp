// Buffer2000.cpp: implementation of the Buffer2000 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Buffer2000.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Buffer2000::Buffer2000()
{
	buffer=NULL;
	Clear();
}

Buffer2000::~Buffer2000()
{
	Clear();
}

Buffer2000::Buffer2000(UINT initial_capacity)
{
	buffer=NULL;
	Clear();
	VerifyCapacity(initial_capacity);
}

void Buffer2000::Clear()
{
	if(buffer) delete [] buffer;
	length=0;
	buffer=NULL;
	bsize=0;
}

void Buffer2000::SetTo(const byte *data,UINT len)
{
	Clear();
	EnsureCapacity(len);
	if(len==0)
		return;
	memcpy(buffer,data,len);
	length+=len;
	buffer[length]=(byte)0;
}

void Buffer2000::WriteBytes(const byte *data,UINT len)
{
	if(!data || len<=0)
		return;
	VerifyCapacity(length+len);
	memcpy(buffer+length,data,len);
	length+=len;
	buffer[length]=(byte)0;
}

void Buffer2000::WriteByte(byte b)
{
	VerifyCapacity(length+1);
	buffer[length++]=b;
	buffer[length]=(byte)0;
}

void Buffer2000::WriteByte(byte b,UINT offset)
{
	VerifyCapacity(max(length,offset+1));
	buffer[offset]=b;
	length=max(offset+1,length);
	buffer[length]=(byte)0;
}


Buffer2000::Buffer2000(const byte *data,UINT len)
{
	buffer=NULL;
	SetTo(data,len);
}

void Buffer2000::WriteFloat(float val)
{
	VerifyCapacity(length+4);
	*(float*)(buffer+length)=val;
	length+=4;
	buffer[length]=(byte)0;
}


void Buffer2000::WriteDWord(DWORD val)
{
	VerifyCapacity(length+4);
	*(DWORD*)(buffer+length)=val;
	length+=4;
	buffer[length]=(byte)0;
}

//use this function with caution (i.e. It is possible for you to leave parts of the buffer uninitialized).
void Buffer2000::WriteDWord(UINT offset, DWORD val)
{
	VerifyCapacity(max(length,offset+4));
	*(DWORD*)(buffer+offset)=val;
	length=max(offset+4,length);
	buffer[length]=(byte)0;
}

void Buffer2000::WriteWord(WORD val)
{
	VerifyCapacity(length+2);
	*(WORD*)(buffer+length)=val;
	length+=2;
	buffer[length]=(byte)0;
}

//use this function with caution (i.e. It is possible for you to leave parts of the buffer uninitialized).
void Buffer2000::WriteWord(UINT offset, WORD val)
{
	VerifyCapacity(max(length,offset+2));
	*(WORD*)(buffer+offset)=val;
	length=max(offset+2,length);
	buffer[length]=(byte)0;
}

void Buffer2000::WriteString(const char *str)
{
	if(str==NULL){
		return;
	}
	UINT slen=(UINT)strlen(str);
	VerifyCapacity(length+slen);
	strcpy((char*)(buffer+length),str);  //strcpy automatically inserts our NULL char for us
	length+=slen;
}

//a slightly modified version of ensurecapicity that will grow the array by more than necessary to make the append operations fairly effecient.
void Buffer2000::VerifyCapacity(UINT size){
	if(size+1<bsize || size==0)  //is it necessary to grow?
		return;
	bsize=((size+1)<<1);

	byte *temp_buf=new byte[bsize];
	if(length>0){
		memcpy(temp_buf,buffer,length);
	}
	temp_buf[length]=(byte)0; //append a null char just for good protocol
	if(buffer) delete []buffer;
	buffer=temp_buf;
}

void Buffer2000::EnsureCapacity(UINT size)
{
	if(size+1<bsize || size==0)  //is it necessary to grow?
		return;
	bsize=size+1;

	byte *temp_buf=new byte[bsize];
	if(length>0){
		memcpy(temp_buf,buffer,length);
	}
	temp_buf[length]=(byte)0; //append a null char just for good protocol
	if(buffer) delete []buffer;
	buffer=temp_buf;
}

void Buffer2000::RemoveRange(UINT offset, UINT count)
{
	if(offset>length || count<1)
		return;

	count=min(count,length-offset);
	bsize=((length-count+1)<<1);
	byte *temp_buffer=new byte[bsize];  //we are going to make a new array so we can trim it to a respectable size
	memcpy(temp_buffer,buffer,offset);  //copy start of buffer to the new one
	memcpy(temp_buffer+offset,buffer+offset+count,length-(offset+count)); //copy the end of the old buffer into the new buffer
	length-=count;
	temp_buffer[length]=(char)0;
	delete []buffer;
	buffer=temp_buffer;
}

DWORD Buffer2000::GetDWord(UINT offset)
{
	if(length<4 || length-4<offset){  //we are dealing with unsigned values... thus it is necessary to check if length<4 first.  Or it will wrap.
		ASSERT(0);  //invalid range, the coder needs ta fix their stuff.
		return 0;
	}

	return *((DWORD*)(buffer+offset));
}

WORD Buffer2000::GetWord(UINT offset)
{
	if(length<2 || length-2<offset){  //we are dealing with unsigned values... thus it is necessary to check if length<4 first.  Or it will wrap.
		ASSERT(0);  //invalid range, the coder needs ta fix their stuff.
		return 0;
	}

	return *((WORD*)(buffer+offset));
}

byte Buffer2000::GetByte(UINT offset)
{
	if(length<1 || length-1<offset){  //we are dealing with unsigned values... thus it is necessary to check if length<4 first.  Or it will wrap.
		ASSERT(0);  //invalid range, the coder needs ta fix their stuff.
		return 0;
	}

	return *((byte*)(buffer+offset));
}

const char* Buffer2000::GetString(UINT offset)
{
	if(length<1 || length-1<offset){  //we are dealing with unsigned values... thus it is necessary to check if length<4 first.  Or it will wrap.
		ASSERT(0);  //invalid range, the coder needs ta fix their stuff.
		return 0;
	}

	return (const char*)(buffer+offset);
}

//wide character format
void Buffer2000::WriteStringW(const char* str)
{
	if(str==NULL){
		return;
	}

	int slen=(int)strlen(str);
	for(int i=0;i<slen;i++)
		WriteWord(str[i]);
}
