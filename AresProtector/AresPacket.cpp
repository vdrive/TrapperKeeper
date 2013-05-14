#include "StdAfx.h"
#include "arespacket.h"
#include ".\arespacket.h"

AresPacket::AresPacket(void)
{
	m_created_time=CTime::GetCurrentTime();
	mp_decrypted_data=NULL;
	mp_encrypted_data=NULL;
	mp_uncompressed_data=NULL;
	m_event_type=ARES_SEND_EVENT;
	mb_error=false;
}

AresPacket::~AresPacket(void)
{
	if(mp_decrypted_data!=NULL){
		mp_decrypted_data->deref();
	}

	if(mp_encrypted_data!=NULL)
		mp_encrypted_data->deref();

	if(mp_uncompressed_data)
		mp_uncompressed_data->deref();
}

bool AresPacket::Read(Buffer2000 *input)
{
	if(input->GetLength()<3)
		return false;
	
	m_type=input->GetByte(2);
	unsigned short m_length=input->GetWord(0);

	//TRACE("AresPacket::Read() type=0x%X length=%u, received buffer length is %u\n",m_type,m_length,input->GetLength());
	if(m_length>(int)input->GetLength()-3)
		return false;

	m_event_type=ARES_RECEIVE_EVENT;
	if(mp_encrypted_data)
		mp_encrypted_data->deref();

	mp_encrypted_data=new Buffer2000(input->GetBufferPtr()+3,m_length);
	mp_encrypted_data->ref();
	input->RemoveRange(0,m_length+3);
	return true;
}

Buffer2000* AresPacket::GetEncryptedData()
{
	return mp_encrypted_data;
}

void AresPacket::SetDecryptedData(Buffer2000* p_decrypted_data)
{
	if(mp_decrypted_data)
		mp_decrypted_data->deref();
	mp_decrypted_data=p_decrypted_data;
	mp_decrypted_data->ref();
}

void AresPacket::SetEncryptedData(Buffer2000* p_encrypted_data)
{
	if(mp_encrypted_data)
		mp_encrypted_data->deref();
	mp_encrypted_data=p_encrypted_data;
	mp_encrypted_data->ref();
}


Buffer2000* AresPacket::GetDecryptedData(void)
{
	return mp_decrypted_data;
}

byte AresPacket::GetEventType(void)
{
	return m_event_type;
}

CString AresPacket::GetFormattedCreateTime(void)
{
	return m_created_time.Format("%I:%M:%S %p");
}


void AresPacket::SetUncompressedData(Buffer2000* data)
{
	if(mp_uncompressed_data)
		mp_uncompressed_data->deref();
	mp_uncompressed_data=data;
	mp_uncompressed_data->ref();
}

Buffer2000* AresPacket::GetUncompressedData(void)
{
	return mp_uncompressed_data;
}


#ifdef TKSOCKETSYSTEM
void AresPacket::SendData(UINT con_handle,TKSocketSystem *p_socket_system)
#else
void AresPacket::SendData(PeerSocket* p_socket)
#endif
{
	int len=GetLength();
	byte* buf=new byte[len+3];
	*(unsigned short*)buf=len;
	buf[2]=m_type;
	memcpy(buf+3,mp_encrypted_data->GetBufferPtr(),len);

#ifdef TKSOCKETSYSTEM
	p_socket_system->SendData(con_handle,buf,len+3);
#else
	p_socket->Send(buf,len+3);
#endif
	//p_socket->Send(buf,len+3);
	delete []buf;
}

void AresPacket::SetType(byte type)
{
	m_type=type;
}

CTime* AresPacket::GetCreatedTime(void)
{
	return &m_created_time;
}
