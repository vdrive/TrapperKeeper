#include "StdAfx.h"
#include "..\tkcom\tkcominterface.h"
#include "DllInterface.h"

AppID TKComInterface::m_com_app;

//DEFAULT CONSTRUCTOR
//The op_code should be a unique number depending on what you want to do.  It controls how messages get routed to these individual com controls.
//A com interface object at the dest will recieve the message if it has the same opcode.  Note:  the dest can be the local computer in which case this com object and all other objects at this address with the same opcode will receive an echo.
TKComInterface::TKComInterface ()
{
	mb_registered=false;
	m_op_code=0;
}

TKComInterface::~TKComInterface ( )
{
	//send a deregistration notice telling COM that this dll no longer wishes to recieve com notifications about whatever this interfaces op_code is
	byte tb[6];
	tb[0]=TKCOMBUFFERKEY;
	tb[1]=TKCOMDEREGISTERCODE; //deregister
	*((UINT*)(tb+2))=m_op_code;

	DllInterface::SendData(m_com_app,tb);
}

bool TKComInterface::InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data){
//bool TKComInterface::InterfaceReceivedData(AppID& from, void* data, UINT& size){
	if(from_app_id.m_app_id!=m_com_app.m_app_id || !mb_registered)
		return false;

	//if(size<11) //2 +length(4)+op_code(4)+source(at least 1 byte)
	//	return false;

	byte* bdata=(byte*)input_data;

	if(bdata[0]!=TKCOMBUFFERKEY)
		return false;

	if( (bdata[1] & TKCOMMESSAGECODE)==0)
		return false;

	if( *((UINT*)(bdata+6))!=m_op_code)
		return false;

	UINT total_length=*((UINT*)(bdata+2));

	string src=(char*)(bdata+10);
	ASSERT(src.size()<300);
	UINT slen=(UINT)src.size()+1;

	//notify all derived objects that we have recieved data.
	DataReceived((char*)src.c_str(),bdata+10+slen,total_length-10-slen);

	return true;
}

//Call to send Reliable (TCP/IP) data to dest.
//dest can be either a name or a specific ip.  If the name server plugin is not added, it must be a specific ip.
//data_length can safely be up to 5000000
//SendReliableData will return false if this app has queued up too much data that hasn't yet been sent out.
bool TKComInterface::SendReliableData(char *dest, void* data, UINT data_length){
	if(!mb_registered)
		return false;

	UINT slen=(UINT)strlen(dest);
	UINT nlen=data_length+slen+10+1;

	byte *buffer=new byte[nlen];
	buffer[0]=TKCOMBUFFERKEY;
	buffer[1]=TKCOMMESSAGERELIABLECODE;

	//append the op_code
	*((UINT*)(buffer+2))=m_op_code;

	//append the op_code
	*((UINT*)(buffer+6))=data_length;

	//append the dest
	strcpy((char*)(buffer+10),dest); 

	//append the data
	memcpy(buffer+10+1+slen,data,data_length);

	bool stat=DllInterface::SendData(m_com_app,buffer,NULL);

	delete [] buffer;
	return stat;
}

//Call to send Unreliable (UDP) data to dest.
//call to send a buffer of data to whomeever the socket is connected to.  
//dest can be either a name or a specific ip.  If the name server plugin is not added, it must be a specific ip.
//data can not be over 502 bytes.
//SendUnreliableData will return false if this app has queued up too much data that hasn't yet been sent out.
bool TKComInterface::SendUnreliableData(char *dest, void* data, UINT data_length){ 
	if(!mb_registered)
		return false;

	UINT slen=(UINT)strlen(dest);
	UINT nlen=data_length+strlen(dest)+10+1;

	byte *buffer=new byte[nlen];
	buffer[0]=TKCOMBUFFERKEY;
	buffer[1]=TKCOMMESSAGEUNRELIABLECODE;

	//append the op_code
	*((UINT*)(buffer+2))=m_op_code;

	//append the op_code
	*((UINT*)(buffer+6))=data_length;

	//append the dest
	strcpy((char*)buffer+10,dest); 

	//append the data
	memcpy(buffer+10+1+slen,data,data_length);

	bool stat=DllInterface::SendData(m_com_app,buffer);

	delete [] buffer;
	return stat;
}

//Nothing will work until Register is called.  It should be called anytime during or after your Dll's DllStart() method has been called by TrapperKeeper
void TKComInterface::Register(Dll* pDll, UINT op_code)
{
	if(mb_registered){  //Is this already registered?
		return;  //there shall be no duplicate registering for a single interface object.
	}
	Interface::Register(pDll);

	mb_registered=true;
	m_op_code=op_code;
	m_com_app.m_app_id=61;

	//send a registration notice telling COM that this dll wishes to recieve com notifications about whatever this interfaces op_code is
	byte tb[6];
	tb[0]=TKCOMBUFFERKEY;
	tb[1]=TKCOMREGISTERCODE; //register
	*((UINT*)(tb+2))=op_code;

	DllInterface::SendData(m_com_app,tb);
}