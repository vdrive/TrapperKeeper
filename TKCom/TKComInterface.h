#pragma once
#include "..\SamplePlugIn\Interface.h"
//#include "AppID.h"

//Author:  Ty Heath
//Created:  3.4.2003
//Last Modified:  3.11.2003
//PURPOSE:
//Gives you the plugin author easy to use and reliable TrapperKeeper to TrapperKeeper communication ability.

//TRAPPER KEEPER COM INTERFACE SYSTEM.

//READ THESE NOTES BEFORE USING THIS CLASS!
//1.  Use objects of type TKComInterface.   Call SendReliableData and SendUnreliableData to send data to a destination (ip or name).
//2.  None of the methods of this class are blocking.  (they will return instantly)
//3.  Each call is completely asynchronous. You can send one thing after another and they may arrive at the destination in any order.  If you want data to arrive coherently then send it all in one call to Send.  This is a result of note #4.
//4.  Internally the com system dynamically opens more socket connections to the same dest if one connection has significant data (more than 64k) queued up.  This prevents plugins from hogging a particular link to somewhere and making other plugins wait in line forever.
//5.  Use the op_code parameter in the constructor to coordinate your communication needs.  If you have only one TKComInterface type object in your plugin, then it should be your plugin's app id.  Please note that if you send a message from an interface object with a particular op_code, all interface objects at the dest with the same op_code will recieve the message.
//6.  Use SendUnreliableData if it is not critical that your destination receive this data (i.e. routine i am here status messages).  It sends the data as a UDP packet and is a bit more efficient than SendReliableData.
//6.  Trapper Keeper's com service kicks ass.  If you disagree, you are wrong.


//ignore these they are internally used
#define TKCOMBUFFERKEY 213
#define TKCOMMESSAGECODE 2
#define TKCOMREGISTERCODE 4
#define TKCOMDEREGISTERCODE 8
#define TKCOMMESSAGERELIABLECODE (16|TKCOMMESSAGECODE)
#define TKCOMMESSAGEUNRELIABLECODE (32|TKCOMMESSAGECODE)

//use this class to send data to a remote trapper keeper via the COM plugin
class TKComInterface : protected Interface
{
private: //private members for internal management
	static AppID m_com_app;
	bool mb_registered;

	//called by the interface class parent.  It is private so that noone can override it in a derived class.
	bool InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data);

public:
	TKComInterface(void);
	virtual ~TKComInterface(void);

	//*********** INTERFACE METHODS ***************//
	//Call to send Reliable (TCP/IP) data to dest.
	//dest can be either a name or a specific ip.  If the name server plugin is not added, it must be a specific ip.
	//data_length can safely be up to 10,000,000 bytes.  (compression will likely make it much smaller, but you cannot rely on this)
	//SendReliableData will return false if this app has queued up too much data that hasn't yet been sent out.	
	bool SendReliableData(char *dest, void* data, UINT data_length); //call to send a buffer of data to whomever the socket is connected to.  

	//dest can be either a name or a specific ip.  If the name server plugin is not added, it must be a specific ip.
	//You can not safely send unreliable data over 500 bytes in length (compression will likely make it much smaller, but you cannot rely on this)
	//The data will arrive (this is UDP so that isn't guaranteed anyway) all at once at the destination in the form of a DataReceived call
	//SendUnreliableData will return false if this app has queued up too much data that hasn't yet been sent out.
	bool SendUnreliableData(char *dest, void* data, UINT data_length); //call to send a buffer of data to whomeever the socket is connected to.  


protected:
	UINT m_op_code;
	//override this to handle data received com events for the op_code that this interface object is assigned to
	virtual void DataReceived(char *source_ip, void *data, UINT data_length){}

public:
	//Nothing will work until Register is called.  It should be called anytime during or after your Dll's DllStart() method has been called by TrapperKeeper.  The reason is that the Com Service may not yet be loaded if you call it before then.
	//The op_code should be a unique number depending on what you want to do.  You should usually set this to your plugins app id.  
	//A com interface object at the dest will recieve the message if it has the same opcode, no matter which plugin it is in.  Note:  the dest can be the local computer (127.0.0.1) in which case this com object and all other objects at this address with the same opcode will receive an echo.
	void Register(Dll* pDll, UINT op_code);
};