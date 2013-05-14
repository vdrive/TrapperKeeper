// Checksum.h

#ifndef CHECKSUM_H
#define CHECKSUM_H

class Checksum
{
public:
	// Public Member Functions
	static unsigned short int Calc(unsigned char *buf,unsigned short int buf_len);
};

#endif // CHECKSUM_H