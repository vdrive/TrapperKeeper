// GnutellaOpCode.h
#pragma once

#define CONNECTION_STRING_0_4		"GNUTELLA CONNECT/0.4\n\n"
#define CONNECTION_REPLY_STRING_0_4	"GNUTELLA OK\n\n"

//#define CONNECTION_STRING_0_6		"GNUTELLA CONNECT/0.6\r\nUser-Agent: BearShare 2.4.4\r\nPong-Caching: 0.1\r\nHops-Flow: 1.0\r\n\r\n"
#define CONNECTION_STRING_0_6		"GNUTELLA CONNECT/0.6\r\n\r\n"
#define CONNECTION_REPLY_STRING_0_6	"GNUTELLA/0.6 200 OK\r\n\r\n"

class GnutellaOpCode
{
public:
	enum op
	{
		Ping=0x00,
		Pong=0x01,
		QRPTableUpdate=0x30,
		Push=0x40,
		Query=0x80,
		QueryHit=0x81
	};
};