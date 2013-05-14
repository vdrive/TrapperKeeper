#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Buffer2000.h"

#ifdef TKSOCKETSYSTEM
#include "..\tkcom\tksocketsystem.h"
#else
#include "..\tkcom\PeerSocket.h"
#endif


#define ARES_INIT			0x33
#define ARES_NETWORK_STATS	0x01
#define ARES_QUERY_HIT		0x12

#define ARES_SEND_EVENT		0x01
#define ARES_RECEIVE_EVENT	0x02

class AresPacket : public Object
{
private:
	CTime m_created_time;
	byte m_type;

	byte m_event_type;

	Buffer2000 *mp_encrypted_data;
	Buffer2000 *mp_decrypted_data;
	Buffer2000 *mp_uncompressed_data;
	bool mb_error;
public:
	AresPacket(void);
	~AresPacket(void);

	unsigned short GetLength(){
		if(mp_encrypted_data!=NULL){
			return mp_encrypted_data->GetLength();
		}
		else if(mp_decrypted_data!=NULL){
			return mp_decrypted_data->GetLength();
		}
		else return 0;
	}
	int GetMessageType(){return (int)m_type;}
	void SetMessageType(int type){m_type=type;}
	bool IsBad(){return mb_error;}

	bool Read(Buffer2000 *input);
	Buffer2000* AresPacket::GetEncryptedData();
	void SetDecryptedData(Buffer2000* p_decrypted_data);
	void SetEncryptedData(Buffer2000* p_encrypted_data);
	Buffer2000* GetDecryptedData(void);
	byte GetEventType(void);
	CString GetFormattedCreateTime(void);
	void SetUncompressedData(Buffer2000* data);
	Buffer2000* GetUncompressedData(void);

#ifdef TKSOCKETSYSTEM
void SendData(UINT con_handle,TKSocketSystem *p_socket_system);
#else
void SendData(PeerSocket* p_socket);
#endif

	//void SendData(UINT con_handle,TKSocketSystem *p_socket_system);
	void SetType(byte type);
	CTime* GetCreatedTime(void);
};
