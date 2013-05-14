#include "StdAfx.h"

#include "supplyprocessorinterface.h"
#include "DllInterface.h"
#include "SupplyProcessorHeader.h"

SupplyProcessorInterface::SupplyProcessorInterface(void)
{
	m_app_id.m_app_id=3614;
	m_app_id.m_version = 0x00000001;
	m_app_id.m_app_name = "Supply Processor";
}

SupplyProcessorInterface::~SupplyProcessorInterface(void)
{
}


bool SupplyProcessorInterface::GetSupplyData(unsigned int project_id, int track_num, const char* network, 
											int num_want, vector<SupplyData> *data)
{
	SupplyProcessorHeader header;
	header.op = SupplyProcessorHeader::Request;
	header.size = sizeof(SupplyProcessorHeader);

	int len = header.size + 3 * sizeof(int) + strlen(network)+1;
	char * buf = new char[len];
	char *ptr=buf;

	memcpy(ptr, &header, header.size);
	ptr += header.size;
	*((unsigned int *)ptr) = project_id;
	ptr += sizeof(unsigned int);
	*((unsigned int *)ptr) = track_num;
	ptr += sizeof(unsigned int);
	*((unsigned int *)ptr) = num_want;
	ptr += sizeof(unsigned int);
	memcpy(ptr,network,strlen(network));
	ptr += strlen(network);
	*((char*)ptr)='\0';
	ptr++;

	bool retVal = SendData(buf, (char*)data);

	delete [] buf;
	return retVal;
}

bool SupplyProcessorInterface::SubmitSupplyData(SupplyData& supply) 
{
	vector<SupplyData> v_sd;
	v_sd.push_back(supply);
	return SubmitSupplyData(&v_sd);
}

bool SupplyProcessorInterface::SubmitSupplyData(vector<SupplyData> * v_supply)
{
	SupplyProcessorHeader header;
	header.op = SupplyProcessorHeader::Submit;
	header.size = sizeof(SupplyProcessorHeader);

	int len = header.size;
	char * buf = new char[len];
	char * ptr = buf;
	memcpy(ptr, &header, header.size);
	ptr += header.size;
	//ptr = (char*)(void*)v_supply;
	//bool retVal = SendData(buf,NULL);
	bool retVal = SendData(buf,v_supply);

	delete [] buf;
	return retVal;
}

bool SupplyProcessorInterface::CleanDatabase(const char * network_name, int days)
{
	SupplyProcessorHeader header;
	header.op = SupplyProcessorHeader::Clear;
	header.size = sizeof(SupplyProcessorHeader);

	int len = sizeof(int) + (int)strlen(network_name) + 1 + header.size;
	char * buf = new char[len];
	char * ptr = buf;
	memcpy(buf, &header, header.size);
	ptr += header.size;
	*((unsigned int *)ptr) = days;
	ptr+= sizeof(unsigned int);
	strcpy(ptr, network_name);
	bool retVal = SendData(buf, NULL);
	delete [] buf;
	return retVal;
}

bool SupplyProcessorInterface::SendData(const char* buf, void * result)
{
	return DllInterface::SendData(m_app_id, (void*)buf, result);
}

bool SupplyProcessorInterface::InterfaceReceivedData(AppID from_app_id,void* input_data, void* output_data)
{			
	return false;
}