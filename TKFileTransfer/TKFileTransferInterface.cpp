#include "StdAfx.h"
#include "tkfiletransferinterface.h"
#include "DllInterface.h"
#include "../DllLoader/AppID.h"

UINT TKFileTransferInterface::session_id_counter=1;
AppID TKFileTransferInterface::m_file_transfer_app;

TKFileTransferInterface::TKFileTransferInterface()
{
	mb_registered=false;
}

TKFileTransferInterface::~TKFileTransferInterface(void)
{
}

bool TKFileTransferInterface::InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data){
	if(!mb_registered || from_app_id.m_app_id!=m_file_transfer_app.m_app_id) //only care if it is from the file transfer service
		return false;

	IPInterface ip;
	if(ip.ReadFromBuffer((byte*)input_data)==0)
		return false;

	
	if(ip.v_ints.size()<1 || ip.v_strings.size()<3)
		return false;

	UINT session_id=(UINT)ip.v_ints[0];
	if(m_session_id!=session_id)
		return false;

	char* source=(char*)ip.v_strings[0].c_str();
	char* local_path=(char*)ip.v_strings[1].c_str();
	char* remote_path=(char*)ip.v_strings[2].c_str();
	char* reason="";
	if(ip.v_strings.size()>3)
		reason=(char*)ip.v_strings[3].c_str();
	

	switch(ip.m_type){
		case ip.RECEIVEDFILE:
			if(ip.v_strings.size()<3)
				return false;
			this->RecievedFile(source,local_path,remote_path);
			break;
		case ip.FAILED:
			if(ip.v_strings.size()<4)
				return false;
			this->GetFileFailed(source,local_path,remote_path,reason);
			break;
		case ip.PROGRESS:
			if(ip.v_floats.size()<1)
				return false;
			this->FileProgress(source,local_path,remote_path,ip.v_floats[0]);
			break;
		case ip.P2PRECEIVEDFILE:
			if(ip.v_ints.size()<2)
				return false;			
			this->P2PRecievedFile(local_path,remote_path,ip.v_ints[1]);
			break;
		default: return false;
	}
	

	return true;
}

bool TKFileTransferInterface::HasP2P(){
	IPInterface request;
	request.m_type=request.P2PEXISTS;
	byte buffer[512];
	request.WriteToBuffer(buffer);

	return DllInterface::SendData(m_file_transfer_app,buffer);	
}

//call this to get a file from the source and save it anywhere you want
bool TKFileTransferInterface::GetFile( const char *source, const char* local_path , const char* remote_path ){
	if(!mb_registered)
		return false;

	IPInterface request;
	request.m_type=request.GETFILE;

	request.v_strings.push_back(string(source));
	request.v_strings.push_back(string(local_path));
	request.v_strings.push_back(string(remote_path));
	request.v_ints.push_back(m_session_id);

	byte buffer[4096];
	request.WriteToBuffer(buffer);

	return DllInterface::SendData(m_file_transfer_app,buffer);
}

//call this to get a file from the source and save it anywhere you want
bool TKFileTransferInterface::P2PGetFile( const char *source, const char* local_path , const char* hash){
	if(!mb_registered)
		return false;

	IPInterface request;
	request.m_type=request.P2PGETFILE;

	request.v_strings.push_back(string(source));
	request.v_strings.push_back(string(local_path));
	request.v_strings.push_back(string(hash));
	request.v_ints.push_back(m_session_id);

	byte buffer[4096];
	request.WriteToBuffer(buffer);

	return DllInterface::SendData(m_file_transfer_app,buffer);
}

//Nothing will work until Register is called.  It should be called anytime during or after your Dll's DllStart() method has been called by TrapperKeeper
void TKFileTransferInterface::Register(Dll* pDll)
{
	if(mb_registered)
		return;

	mb_registered=true;
	Interface::Register(pDll);
	m_file_transfer_app.m_app_id=62;
	m_session_id=session_id_counter++;
}

/*
//this function is for the load balancer, it retrieves a map of the current distribute directory
bool TKFileTransferInterface::GetDistributeMap(TKSyncherMap *map){
	IPInterface request;
	request.m_type=request.GETDISTRIBUTEMAP;

	request.SetData((byte*)map,4);

	byte buffer[4096];
	request.WriteToBuffer(buffer);

	return DllInterface::SendData(m_file_transfer_app,buffer);
}
*/

