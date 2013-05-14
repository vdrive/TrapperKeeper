#include "StdAfx.h"
#include "clientsocket.h"
#include "ClientConnection.h"
#include "..\BTScraperDll\PeerList.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "ConnectionSockets.h"

CClientSocket::CClientSocket(void)
{

	m_data_size = (1024*16);
	m_index = 1;
	m_connected = false;
	m_cur_piece = 0;
	m_cur_pos = 0;
	p_my_bitfield = NULL;
	p_remote_bitfield = NULL;
	m_socket_header_received = false;
	m_handshake_received = false;
	m_offer_piece = 0;
	m_is_seed = false;
	m_sharing = false;
	m_sharing_increment = 1;

}

CClientSocket::CClientSocket(int datasize)
{

	m_data_size = datasize;
	m_index = 1;
	m_connected = false;
	m_cur_piece = 0;
	m_cur_pos = 0;
	p_my_bitfield = NULL;
	p_remote_bitfield = NULL;
	m_socket_header_received = false;
	m_handshake_received = false;
	m_offer_piece = 0;
	m_is_seed = false;
	m_sharing = false;
	m_sharing_increment = 1;

}


CClientSocket::CClientSocket(PeerList * peer, TorrentFile * torrent)
{

	m_peer = *peer;

	strcpy(m_torrent_name,torrent->GetName().c_str());
	m_torrent_num_pieces = torrent->GetNumPieces();
	m_torrent_piecelength = torrent->GetPieceLength();
	m_data_size = (1024*16);
	m_connected = false;
	m_cur_piece = 0;
	m_cur_pos = 0;
	p_my_bitfield = NULL;
	p_remote_bitfield = NULL;
	m_socket_header_received = false;
	m_handshake_received = false;
	m_offer_piece = 0;
	m_is_seed = false;
	m_sharing = false;

}

CClientSocket::~CClientSocket(void)
{

	if (p_my_bitfield != NULL)
	{
		delete [] p_my_bitfield;
	}

	if (p_remote_bitfield != NULL)
	{
		delete [] p_remote_bitfield;
	}
}

void CClientSocket::InitParent(ConnectionSockets * parent)
{
	p_parent = parent;


}

bool CClientSocket::OnReceive(int error_code)
{



	if(error_code!=0)
	{
		Close();
		return false;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return false;
	}

	// ---

	int nRSDResult;

	// Your ReceiveSocketData() or ReceiveSomeSocketData() code goes here
	if (m_handshake_received == false)
	{
		nRSDResult = ReceiveSocketData(1);
	}
	else
	{
		nRSDResult = ReceiveSocketData(4);
	}
	// ---

	// Check for more data
	/*
	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			OnReceive(0);	// recursive call
		}
	}
	*/
	// Try to receive a header

  // IF the header receive did not fail
  if( nRSDResult == 0 )
  {
   // Check if there was a missed receive
   char buf[1];
   memset(buf,0,sizeof(buf));
   WSABUF wsabuf;
   wsabuf.buf=buf;
   wsabuf.len=1;

   DWORD num_read = 0;
   DWORD flags = MSG_PEEK;
   int nPeekResult = WSARecv( m_hSocket, &wsabuf, 1, &num_read, &flags, NULL, NULL );

   if( nPeekResult != SOCKET_ERROR && num_read != 0)
   {
    return true;
   }
  }
 


	return false;
}


void CClientSocket::OnClose(int error_code)
{
	m_socket_header_received = false;
	m_handshake_received = false;
	m_connected = false;
	TEventSocket::OnClose(error_code);
}

void CClientSocket::SetIp(char * ip)
{
	strcpy(m_ip,ip);
}



//Obsolete, not calling SomeSocketDataReceived anymore since I always know what I want
void CClientSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{

	byte * dataptr = (byte *)data;
	if (data_len <= 4)
	{
		// if data len is 4 (or less) it's a keep alive message
		SendKeepAlive();
	}
	if (data_len > 4)
	{
		// The string length of the BitTorrent Protocol is 19
		if (*dataptr == 0x13)
		{
			SendBitfield();
			return;
		}
		dataptr +=4;
		// 1 is Unchoke
		if (*dataptr == 0x01)
		{
			// When they unchoke us let them know we want their data
			SendInterested();
		}
		// 4 is a Have message, the client is sending a piece index
		if (*dataptr == 0x04)
		{
			dataptr++;
			int * piecenum = (int *)dataptr;
			SendSpecificRequest(*piecenum,0);
			// TODO  record all the messages to find out what nobody has and request those from seeds
		}
		// 5 is a bitfield
		else if (*dataptr == 0x05)
		{
		//	BitfieldReceived(data);
			SendInterested();
		}
		// 6 is a request
		else if(*dataptr == 0x06)
		{
			SendChoke();
		}
		// 7 is a piece
		else if (*dataptr == 0x07)
		{
			// Get the size
			int* intptr = (int*)data;
			unsigned char * tempbuf = new unsigned char[17];
			memcpy(tempbuf,data,17);
			int dataneeded = *intptr;
			dataneeded = SwapBytes(dataneeded);
			// Get the Piece Number
			dataptr++;
			// if the piece they sent isn't part of the current piece, it's a new piece
			intptr = (int *)dataptr;
			int temppiece = *intptr;
			if (temppiece != m_cur_piece)
			{
				m_cur_pos = 0;
			}
			m_cur_piece = temppiece;
			// Get the position
			intptr ++; // (int *)data[9];
			int temppos = *intptr;
			temppos = SwapBytes(temppos);
			m_cur_pos += temppos;
			delete [] tempbuf;
			//p_parent->ReportDataRequest(dataneeded, m_ip);
			if (dataneeded < 65539+9)
			{
				ReceiveSocketData(dataneeded);
			}

		}
		else 
		{
		}
	}
}


void CClientSocket::BitfieldReceived(char * data, int len)
{
	
	byte * dataptr = (byte *)data;
	dataptr ++;
//	p_remote_bitfield = new bool[len-1];
//	memcpy(&p_remote_bitfield,dataptr,len-1);

	for (int i = 0; i < len; i ++)
	{
		if (dataptr[i] == 0)
		return;
	}

	m_is_seed = true;

	// Record this later as part of trying to figure out what to download

}
void CClientSocket::SocketDataReceived(char *data,unsigned int len)
{
	//char msg[1024+1];
	//sprintf(msg,"SocketDataReceived from ip %s",m_ip); 
	//p_parent->LogMessage(msg);

	if(m_handshake_received == false)
	{
		if(m_socket_header_received == false)
		{
			unsigned char length = *((unsigned char*)data);
			if (length > 1<<16)
			{
				Close();
			}
			else
			{
				m_socket_header_received = true;
				//sprintf(msg,"It's a Handshake from ip %s",m_ip); 
				//p_parent->LogMessage(msg);
				ReceiveSocketData(length+8+20+20);
			}
		}
		else
		{
			//sprintf(msg,"It's the rest of a Handshake from ip %s",m_ip); 
			//p_parent->LogMessage(msg);
			m_socket_header_received = false;
			m_handshake_received = true;
			// Data is the handshake Data
			char * ptr = data;
			ptr +=(len-20);
			char buf[21];
			memcpy(buf,ptr,20);
			buf[20] = '\0';
			p_parent->ReportClientId(buf,m_ip);
		}
		return;
	}

	if (m_socket_header_received == false)
	{
		if (len != 4)
		{
			//sprintf(msg,"It's not the header from ip %s but it should be so closing",m_ip); 
			//p_parent->LogMessage(msg);
			Close();
			return;
		}

		unsigned int len = *((unsigned int*)data);
		len = SwapBytes(len);
		if (len == 0)
		{
		//	SendKeepAlive();
		}
		else
		{
			//sprintf(msg,"It's a header from ip %s ",m_ip); 
			//p_parent->LogMessage(msg);
			m_socket_header_received = true;
			ReceiveSocketData(len);
		}
	}
	else
	{
		m_socket_header_received = false;

		if (len == 1)
		{
			unsigned char dataptr = *((unsigned char*)data);
			if (dataptr == BT_UNCHOKE)
			{
				// When they unchoke us let them know we want their data
				//sprintf(msg,"Unchoke Message from  %s ",m_ip); 
				//p_parent->LogMessage(msg);
				SendInterested();
				SendRequest();
			}
			
			if (dataptr == BT_INTERESTED)
			{
				//sprintf(msg,"Interested message from  %s ",m_ip); 
				//p_parent->LogMessage(msg);
				SendUnChoke();

			}

		}
		else if (len == 5)
		{
			unsigned char dataptr = *((unsigned char*)data);
			//the client is sending a piece index
			if (dataptr == BT_HAVE)
			{
				//sprintf(msg,"Have message from  %s ",m_ip); 
				//p_parent->LogMessage(msg);
				data++;
				int * piecenum = (int *)data;
				SendSpecificRequest(*piecenum,0);
			// TODO  record all the messages to find out what nobody has and request those from seeds
			}
		}
		else
		{
			unsigned char dataptr = *((unsigned char*)data);
			if (dataptr == BT_BITFIELD)
			{
				//sprintf(msg,"Bitfield message from  %s ",m_ip); 
				//p_parent->LogMessage(msg);
				BitfieldReceived(data, len);
				SendInterested();
			}
			else if(dataptr == BT_REQUEST)
			{
				if(m_sharing == true)
				{
					//sprintf(msg,"Request message from  %s ",m_ip); 
					//p_parent->LogMessage(msg);
					RequestReceived(data);
				}
				else
				{
					SendChoke();
				}
			}
			else if (dataptr == BT_PIECE)
			{
				//sprintf(msg,"Piece Data message from  %s Yay!",m_ip); 
				//p_parent->LogMessage(msg);
				data++;
				int* intptr = (int*)data;
				// Get the Piece Index
				int temppiece = *intptr;
				bool piecefound = false;
				for (int j = 0; j < (int)v_piece_pos.size(); j++)
				{
						
					if (v_piece_pos[j].piece == temppiece)
					{
						m_cur_pos = v_piece_pos[j].position;
						v_piece_pos[j].position += len-9;
						piecefound = true;
						break;
					}

				}
				if (piecefound == false)
				{
					m_cur_pos = 0;
				}

				m_cur_piece = temppiece;
				// Get the Begin Position
				intptr ++; // (int *)data[9];
				int temppos = *intptr;
				temppos = SwapBytes(temppos);
				// Add the length to the current Position for the request
				m_cur_pos += len-9; // subtract the header size from the data size
				int requestpos = m_cur_pos;
				int displaypiece = SwapBytes(temppiece);
				char tempname[256+1];
				strcpy(tempname,m_torrent_name);
				p_parent->ReportDataReceived(len, requestpos,displaypiece,m_ip,tempname,m_is_seed);
				//requestpos = SwapBytes(requestpos);

				// If this piece wasn't in the vector
				if (piecefound == false)
				{
					PiecePosition newpiece;
					newpiece.piece = m_cur_piece;
					newpiece.position = requestpos;
					v_piece_pos.push_back(newpiece);
				}
				//If the piece is strictly less than the piece length ask for the next piece
				int piecelen = m_torrent_piecelength;
				if (requestpos < piecelen)
				{
					SendSpecificRequest(m_cur_piece,requestpos);
				}
				// otherwise the piece is finished so ask for a new one
				else
				{
					SendRequest();
				}
				//p_parent->ReportDataRequest(dataneeded, m_ip);

			}// end else dataptr = 7

		}// end else data_len

	}// end else socket_header
	/*
		byte * dataptr = (byte *)data;
	if (data_len > 4)
	{
		dataptr +=4;
		int temppos = SwapBytes(data_len);
		SendSpecificRequest(m_cur_piece,temppos);
	}
*/

	//sprintf(msg,"SocketDataReceived Finished from ip %s",m_ip); 
	//p_parent->LogMessage(msg);
}

int CClientSocket::ReceiveSocketData(unsigned int len)
{
	if (len > 1<<16)
	{
		Close();
		return(0);
	}
	else
	{
		if (this->IsSocket())
		{
			return TSocket::ReceiveSocketData(len);
		}
		else
		{
			return(0);
		}
	}
}

int CClientSocket::ReceiveSomeSocketData(unsigned int len)
{
	return TSocket::ReceiveSomeSocketData(len);
}
void CClientSocket::OnConnect(int error_code)
{

//	p_parent->ConnectedToServer();
	if (error_code == 0)
	{
		char msg[1024+1];
		sprintf(msg,"OnConnectStarted with ip %s",m_ip); 
		p_parent->LogMessage(msg);
		m_connected = true;
		unsigned char * message = p_parent->GetHandshakeMessage();
		if(this->IsSocket())
		{
			SendSocketData(message,68);
		}
		delete[] message;
		sprintf(msg,"OnConnectFinished with ip %s",m_ip); 
		p_parent->LogMessage(msg);
	}
}

void CClientSocket::SendKeepAlive()
{
	p_parent->LogMessage("Send Keep Alive");
	byte len[4];
	int size = sizeof(len);
	unsigned char * message = new unsigned char[size];
	for (int i = 0; i < 4; i++)
	{
		len[i] = 0x00;
	}
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	if(this->IsSocket())
	{
		SendSocketData(message,size);
	}
	delete [] message;
	p_parent->LogMessage("End Send Keep Alive");
	
	//SendRequest();
}

void CClientSocket::SendChoke()
{
	byte len[4];
	byte id = 0x00;
	int size = sizeof(len)+ sizeof(id);
	unsigned char * message = new unsigned char[size];
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x01;
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	if(this->IsSocket())
	{
		SendSocketData(message,size);
	}
	delete [] message;
}

void CClientSocket::SendUnChoke()
{
	byte len[4];
	byte id = 0x01;
	int size = sizeof(len)+ sizeof(id);
	unsigned char * message = new unsigned char[size];
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x01;
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	if(this->IsSocket())
	{
		SendSocketData(message,size);
	}
	delete [] message;
}


void CClientSocket::SendHave()
{
	byte len[4];
	byte id = 0x04;
	int piece_index = SwapBytes(m_offer_piece);
	int size = sizeof(len)+ sizeof(id)+ sizeof(int);
	unsigned char * message = new unsigned char[size];
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x05;
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	memcpy(&message[pos],&piece_index,sizeof(int));
	pos += sizeof(int);
	if(this->IsSocket())
	{
		SendSocketData(message,size);
	}
	m_offer_piece +=m_sharing_increment;
	if (m_offer_piece > m_torrent_num_pieces)
	{
		//m_offer_piece = 0;
		m_offer_piece = rand()%100;
	}

	delete [] message;

	
}

void CClientSocket::SendInterested()
{
	char msg[1024+1];
	p_parent->LogMessage("Send Interested");
	byte len[4];
	byte id = 0x02;
	int size = sizeof(len)+ sizeof(id);
	sprintf(msg,"Interested Message size %d",size);
	p_parent->LogMessage(msg);
	unsigned char * message = new unsigned char[size];
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x01;
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	sprintf(msg,"Interested Message size %d",size);
	p_parent->LogMessage(msg);
	if(this->IsSocket())
	{
		sprintf(msg,"Sending Interested Message %s",message);
		p_parent->LogMessage(msg);
		SendSocketData(message,size);
		p_parent->LogMessage("Interested Message Sent deleting message");
	}

	delete [] message;
	p_parent->LogMessage("Interested Message deleted");
	p_parent->LogMessage("End Send Interested");
}

void CClientSocket::Handshake(PeerList * peer)
{
    
}

void CClientSocket::SendRequest()
{
	p_parent->LogMessage("Send Request");
	byte len[4];
	byte id = 0x06;
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x0D;
	int index = m_index;
	int begin = 0;
	int length = 1<<14;

	// Make everything Big Endian
	index = SwapBytes(index);
	length = SwapBytes(length);

	unsigned char * message = new unsigned char[256+1];
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	memcpy(&message[pos],&index,sizeof(int));
	pos += sizeof(int);
	memcpy(&message[pos],&begin,sizeof(int));
	pos += sizeof(int);
	memcpy(&message[pos],&length,sizeof(int));
	pos += sizeof(int);
	if(this->IsSocket())
	{
		SendSocketData(message,pos);
	}

	m_index ++;
	if (m_index > m_torrent_num_pieces)
	{
		m_index = 0;
	}
	delete [] message;
	p_parent->LogMessage("End Send Request");

}

void CClientSocket::SendSpecificRequest(int piece, int begin)
{

	byte len[4];
	byte id = 0x06;
	for (int i = 0; i < 3; i++)
	{
		len[i] = 0x00;
	}
	len[3] = 0x0D;
	int index = piece;
	int start = SwapBytes(begin);
	int length = 1<<14;

	// Make everything Big Endian
	length = SwapBytes(length);
	
	unsigned char * message = new unsigned char[256+1];
	int pos = 0;
	memcpy(&message[pos],&len,sizeof(len));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	memcpy(&message[pos],&index,sizeof(int));
	pos += sizeof(int);
	memcpy(&message[pos],&start,sizeof(int));
	pos += sizeof(int);
	memcpy(&message[pos],&length,sizeof(int));
	pos += sizeof(int);
	if(this->IsSocket())
	{
		SendSocketData(message,pos);
	}

	m_index ++;
	if (m_index > m_torrent_num_pieces)
	{
		m_index = 0;
	}
	delete [] message;

}

void CClientSocket::SendBitfield()
{

	//length is 1 + the bitfield
	
	int len = 1+ (m_torrent_num_pieces/8);
	byte id = 0x05;
	// Swap the int to be big endian
	int bytelen = SwapBytes(len);	// The total message is the bitfield + the id and len field
	unsigned char * message = new unsigned char[len + 1 +4];
	int pos = 0;
	memcpy(&message[pos],&bytelen,sizeof(bytelen));
	pos += sizeof(len);
	memcpy(&message[pos],&id,sizeof(id));
	pos += sizeof(id);
	memcpy(&message[pos],&p_my_bitfield,(len-1));
	pos += len-1;
	if(this->IsSocket())
	{
		SendSocketData(message,pos);
	}
	delete [] message;

}

bool CClientSocket::IsConnected()
{
	//just for now till I decide how to tell

	return m_connected;
}

void CClientSocket::SetTempConnect()
{
	m_connected = true;

}

void CClientSocket::SetDisconnected()
{
	m_connected = false;

}

int CClientSocket::Close()
{
	m_socket_header_received = false;
	m_handshake_received = false;
	m_connected = false;
	return TEventSocket::Close();

}
void CClientSocket::Attach(SOCKET hsocket)
{

	if(IsSocket())
	{

		Close();

	}

	m_hSocket=hsocket;

	m_data_size = (1024*16);
	m_index = 1;
	m_connected = false;
	m_cur_piece = 0;
	m_cur_pos = 0;
	p_my_bitfield = NULL;
	p_remote_bitfield = NULL;
	m_socket_header_received = false;
	m_handshake_received = false;
	m_offer_piece = 0;

	EventSelect(FD_ALL_EVENTS);

}

bool CClientSocket::InitTorrent(TorrentFile torrent)
{

	char message[1024+1];
	int * thisaddr = (int*)(this);
	sprintf(message,"Init Torrent, this is %d",*thisaddr);
	p_parent->LogMessage(message);
	if (torrent.GetName()!= "(null)")
	{
		sprintf(message,"Torrent is %s",torrent.GetName().c_str());
		p_parent->LogMessage(message);
		//m_torrent = torrent;
		strcpy(m_torrent_name,torrent.GetName().c_str());
		m_torrent_num_pieces = torrent.GetNumPieces();
		m_torrent_piecelength = torrent.GetPieceLength();
		int len = m_torrent_num_pieces;

		if (len < 32*1024)
		{
			sprintf(message,"Message Length OK, len = %d",len);
			p_parent->LogMessage(message);
			p_my_bitfield = new bool[len];
			for (int i = 0; i < len; i ++)
			{
				if (i%20 == 0)
				{
					p_my_bitfield[i] = 1;
				}
				else
				{
					p_my_bitfield[i] = 0;
				}
			}
		}
		else
		{
			sprintf(message,"Message Length is bad bailing out, len = %d",len);
			p_parent->LogMessage(message);
			return false;
		}
		return true;
	}
	else 
	{
		p_parent->LogMessage("Torrent File is NULL bailing out");
		return false;
	}
}

int CClientSocket::SwapBytes(int value)
{
	int ret = ((value>>24)&0xff) | ((value>>8)&0xff00) | ((value<<8)&0xff0000) | ((value<<24)&0xff000000);
	return ret;
}

int CClientSocket::RequestSocketInfo()
{

	return (int)(v_send_data_buffers.size());

}

void CClientSocket::SendRandomData(int index, int begin, int length)
{

	int i_index = SwapBytes(index);
	int i_begin = SwapBytes(begin);
	int i_length = SwapBytes(length);
	
	unsigned char * buf;

	// send just a little bit less than a full piece
		i_length -= 10;
	if (i_length < 1<<16)
	{
		buf = new unsigned char[i_length+13];
	}
	else
	{
		return;
	}



	int len = 9+i_length;
	len = SwapBytes(len);
	byte id = 0x07;

	int pos = 0;
	memcpy(&buf[pos],&len,sizeof(int));
	pos += sizeof(int);
	memcpy(&buf[pos],&id,sizeof(byte));
	pos += sizeof(byte);
	memcpy(&buf[pos],&index,sizeof(int));
	pos += sizeof(int);
	memcpy(&buf[pos],&begin,sizeof(int));

	pos += sizeof(int);
	for(int i=pos;i<i_length+pos;i++)
	{
		buf[i]=rand()%256;
	}
	pos += i_length;
	if(this->IsSocket())
	{
		SendSocketData(buf,pos);
		char tempname[256+1];
		strcpy(tempname,m_torrent_name);
		p_parent->ReportDataSent(i_length,i_index,m_ip,tempname);
	}

	delete [] buf;
}



void CClientSocket::SendValidData(int index, int begin, int length)
{

	// i_XXX is the integer form of the big endian value
	int i_index = SwapBytes(index);
	int i_begin = SwapBytes(begin);
	int i_length = SwapBytes(length);
	
	unsigned char * buf;

	// send just a little bit less than a full piece
	//	i_length -= 10;
	if (i_length < 1<<16)
	{
		buf = new unsigned char[i_length+13];
	}
	else
	{
		return;
	}



	int len = 9+i_length;
	len = SwapBytes(len);
	byte id = 0x07;

	int pos = 0;
	memcpy(&buf[pos],&len,sizeof(int));
	pos += sizeof(int);
	memcpy(&buf[pos],&id,sizeof(byte));
	pos += sizeof(byte);
	memcpy(&buf[pos],&index,sizeof(int));
	pos += sizeof(int);
	memcpy(&buf[pos],&begin,sizeof(int));

	pos += sizeof(int);
	/*
	for(int i=pos;i<i_length+pos;i++)
	{
		buf[i]=rand()%256;
	}
	*/
	CFile infile;
	char filename[256+1]; 
	strcpy(filename,"C:\\torrentshared\\");
	strcat(filename,m_torrent_name);

	char * ptr = strrchr(filename,'.');
	if (ptr != NULL)
	{
		*ptr = '\0';
	}
	strcat(filename,".btf");

	// making our own extension here for downloaded torrent files
	if (infile.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD Damnerror = ::GetLastError();
		delete [] buf;
		SendRandomData(index,begin,length);  // this is not one we're sharing so send random
		return;
	}
	
	int temppos = (m_torrent_piecelength * i_index) + i_begin;

	infile.Seek(((m_torrent_piecelength * i_index)+i_begin),CFile::begin);
	infile.Read(&buf[pos],i_length);
	pos += i_length;

	infile.Close();
	if(this->IsSocket())
	{
		SendSocketData(buf,pos);
		// Send Data Message to Dialog
		char tempname[256+1];
		strcpy(tempname,m_torrent_name);
		p_parent->ReportDataSent(i_length,i_index,m_ip,tempname);
	}

	delete [] buf;
}
/*
void CClientSocket::RequestReceived(char * data)
{
	char * dataptr;
	dataptr = data;
	dataptr +=1;
	int *index = (int*)dataptr;
	dataptr +=4;
	int *begin = (int*)dataptr;
	dataptr +=4;
	int *length = (int*)dataptr;
	int i_begin = SwapBytes(*begin);
	if (i_begin < (16*1024))
	{
		char * msg = new char[256+1];
		int i_length = SwapBytes(*length);
		SendRandomData(*index,*begin,*length);
		sprintf(msg,"Sending %d bytes of Data to %s, for piece %d, beginning at %d",i_length,m_ip,SwapBytes(*index),i_begin);
		p_parent->ReportToDialog(msg);
	}
	else
	{
		SendChoke();
	}

}
*/
void CClientSocket::RequestReceived(char * data)
{
	char * dataptr;
	dataptr = data;
	dataptr +=1;
	int *index = (int*)dataptr;
	dataptr +=4;
	int *begin = (int*)dataptr;
	dataptr +=4;
	int *length = (int*)dataptr;
	int i_begin = SwapBytes(*begin);

		char * msg = new char[256+1];
		int i_length = SwapBytes(*length);
		SendValidData(*index,*begin,*length);
		sprintf(msg,"Sending %d bytes of Data to %s, for piece %d, beginning at %d",i_length,m_ip,SwapBytes(*index),i_begin);
		p_parent->ReportToDialog(msg);

}

void CClientSocket::SetAsSeed()
{
	m_is_seed = true;
}

void CClientSocket::SetSharing(bool state)
{

	m_sharing = state;

}

void CClientSocket::SetIncrement(int increment)
{
	m_sharing_increment = increment;
	int randnum = rand()%100;
	m_offer_piece = increment * randnum;

}

string CClientSocket::GetIpString()
{
	string returnstring = m_ip;
	return(returnstring);

}