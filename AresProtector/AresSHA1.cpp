#include "stdafx.h"
#include "AresSHA1.h"

//converts an integer in 01011011 format into a binary value.  up to 8 bits.
#define BINARY(n) ( (((n/10000000)%10)*128) + \
                    (((n/1000000)%10)*64) + \
                    (((n/100000)%10)*32) + \
                    (((n/10000)%10)*16) + \
                    (((n/1000)%10)*8) + \
                    (((n/100)%10)*4) + \
                    (((n/10)%10)*2) + \
                    (n%10) )

	// Rotate x bits to the left
	#define ROL32(value, bits) (((value)<<(bits))|((value)>>(32-(bits))))

	#ifdef LITTLE_ENDIAN
		#define SHABLK0(i) (block->l[i] = (ROL32(block->l[i],24) & 0xFF00FF00) \
			| (ROL32(block->l[i],8) & 0x00FF00FF))
	#else
		#define SHABLK0(i) (block->l[i])
	#endif

	#define SHABLK(i) (block->l[i&15] = ROL32(block->l[(i+13)&15] ^ block->l[(i+8)&15] \
		^ block->l[(i+2)&15] ^ block->l[i&15],1))

	// SHA1 rounds
	#define R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827901+ROL32(v,5); w=ROL32(w,30); }
	#define R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827901+ROL32(v,5); w=ROL32(w,30); }
	#define R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
	#define R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x1F1CBCDC+ROL32(v,5); w=ROL32(w,30); }
	#define R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }


AresSHA1::AresSHA1()
{
	Reset();
	//initialize our set of characters for the base32 ascii alphabet
	int i;
	for(i=0;i<26;i++)
		base32Alphabet[i]='A'+i;

	for(i=26;i<32;i++)
		base32Alphabet[i]='2'+(i-26);
}

AresSHA1::~AresSHA1()
{
	//Reset();
}


void AresSHA1::Reset()
{
	// SHA1 initialization constants
	m_state[0] = 0x17452301L;
	m_state[1] = 0xEACDAF89L;
	m_state[2] = 0x98BFDCFEL;
	m_state[3] = 0x14325476L;
	m_state[4] = 0xC3D2C1F0L;

	m_count[0] = 0;
	m_count[1] = 0;
}

void AresSHA1::Transform(unsigned long state[5], unsigned char buffer[64])
{
	unsigned long a = 0, b = 0, c = 0, d = 0, e = 0;

	SHA1_WORKSPACE_BLOCK* block;
	unsigned char workspace[64];
	block = (SHA1_WORKSPACE_BLOCK *)workspace;
	memcpy(block, buffer, 64);

	// Copy state[] to working vars
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	// 4 rounds of 20 operations each. Loop unrolled.
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	// Add the working vars back into state[]
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	// Wipe variables
	a = 0; b = 0; c = 0; d = 0; e = 0;
}

// Use this function to hash in binary data and strings
void AresSHA1::Update(unsigned char* data, unsigned int len)
{
	unsigned long i = 0, j = 0;

	j = (m_count[0] >> 3) & 63;

	if((m_count[0] += len << 3) < (len << 3)) m_count[1]++;

	m_count[1] += (len >> 29);

	if((j + len) > 63)
	{
		memcpy(&m_buffer[j], data, (i = 64 - j));
		Transform(m_state, m_buffer);

		for (; i+63 < len; i += 64)
		{
			Transform(m_state, &data[i]);
		}

		j = 0;
	}
	else i = 0;

	memcpy(&m_buffer[j], &data[i], len - i);
}

// Hash in file contents
bool AresSHA1::HashFile(char *szFileName)
{
	int b_kill=0;
	return HashFile(szFileName,-1,b_kill);
}

// Hash in file contents
bool AresSHA1::HashFile(char *szFileName,int sleep_time,volatile int &b_kill)
{
	unsigned long ulFileSize = 0, ulRest = 0, ulBlocks = 0;
	unsigned long i = 0;
	unsigned char uData[MAX_FILE_READ_BUFFER];
	FILE *fIn = NULL;

	if((fIn = fopen(szFileName, "rb")) == NULL) return(false);

	fseek(fIn, 0, SEEK_END);
	ulFileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);

	ulRest = ulFileSize % MAX_FILE_READ_BUFFER;
	ulBlocks = ulFileSize / MAX_FILE_READ_BUFFER;

	for(i = 0; i < ulBlocks && !b_kill; i++)
	{
		fread(uData, 1, MAX_FILE_READ_BUFFER, fIn);
		Update(uData, MAX_FILE_READ_BUFFER);
		if(sleep_time>=0 && (((i+1)%100)==0)){  //sleep about every 1 meg of file processed
			Sleep(sleep_time);
		}
	}

	if(ulRest != 0 && !b_kill)
	{
		fread(uData, 1, ulRest, fIn);
		Update(uData, ulRest);
	}

	fclose(fIn);
	fIn = NULL;

	return(true);
}

// Hash in part of a file contents
// Function added by Ty Heath, 6.13.2003
bool AresSHA1::HashPartOfFile(char *szFileName,UINT start,UINT count)
{
	unsigned long ulFileSize = 0, ulRest = 0, ulBlocks = 0;
	unsigned long i = 0;
	unsigned char uData[MAX_FILE_READ_BUFFER];  //a conservative amount of data to read in at one time.
	FILE *fIn = NULL;

	if((fIn = fopen(szFileName, "rb")) == NULL) return(false);  //try to open the file in read+binary mode

	fseek(fIn, 0, SEEK_END);
	ulFileSize = ftell(fIn);  //derive the file size by looking at the file position pointer

	if(start+count>ulFileSize){  //is it impossible to read the specified data?
		return false;
	}

	fseek(fIn, start, SEEK_SET);  //seek to the start of the data
	//change ulFileSize to be equal to count, so we only read 'count' bytes of data from the file
	ulFileSize=count;

	ulRest = ulFileSize % MAX_FILE_READ_BUFFER;  //unever end
	ulBlocks = ulFileSize / MAX_FILE_READ_BUFFER; //whole blocks we can read

	for(i = 0; i < ulBlocks; i++) //whole blocks of data that we can read
	{
		fread(uData, 1, MAX_FILE_READ_BUFFER, fIn);
		Update(uData, MAX_FILE_READ_BUFFER);
	}

	if(ulRest != 0)  //the uneven remainder of data
	{
		fread(uData, 1, ulRest, fIn);
		Update(uData, ulRest);
	}

	fclose(fIn);
	fIn = NULL;

	return(true);
}


void AresSHA1::Final()
{
	unsigned long i = 0, j = 0;
	unsigned char finalcount[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	for (i = 0; i < 8; i++)
		finalcount[i] = (unsigned char)((m_count[(i >= 4 ? 0 : 1)]
			>> ((3 - (i & 3)) * 8) ) & 255); // Endian independent

	Update((unsigned char *)"\200", 1);

	while ((m_count[0] & 504) != 448)
		Update((unsigned char *)"\0", 1);

	Update(finalcount, 8); // Cause a SHA1Transform()

	for (i = 0; i < 20; i++)
	{
		m_digest[i] = (unsigned char)((m_state[i >> 2] >> ((3 - (i & 3)) * 8) ) & 255);
	}

	// Wipe variables for security reasons
	i = 0; j = 0;
	memset(m_buffer, 0, 64);
	memset(m_state, 0, 20);
	memset(m_count, 0, 8);
	memset(finalcount, 0, 8);

	Transform(m_state, m_buffer);
}

// Get the final hash as a pre-formatted string
void AresSHA1::ReportHash(char *szReport, unsigned char uReportType)
{
	unsigned char i = 0;
	char szTemp[4];

	if(uReportType == REPORT_HEX)
	{
		sprintf(szTemp, "%02X", m_digest[0]);
		strcat(szReport, szTemp);

		for(i = 1; i < 20; i++)
		{
			sprintf(szTemp, " %02X", m_digest[i]);
			strcat(szReport, szTemp);
		}
	}
	else if(uReportType == REPORT_DIGIT)
	{
		sprintf(szTemp, "%u", m_digest[0]);
		strcat(szReport, szTemp);

		for(i = 1; i < 20; i++)
		{
			sprintf(szTemp, " %u", m_digest[i]);
			strcat(szReport, szTemp);
		}
	}
	else strcpy(szReport, "Error: Unknown report type!");
}

// Get the raw message digest
void AresSHA1::GetHash(unsigned char *uDest)
{
	memcpy(uDest, m_digest, 20);
}

void AresSHA1::Base32Encode(char *result)
{
	//70000000	= 2^40

	//20 bytes = 160 bites
	//Since our base is a power of 2 then we can convert every block of 5 bytes into 7 base32 characters
	//every 5 bytes = 40 bits, our evenly divisible marker, sorta like every 4 bits in a byte can be converted to hex

	byte sha1[20];
	GetHash(sha1);

	int index=0;
	for(int i=0;i<20;i+=5){  //do 4 times, skipping ahead 5 bytes at a time
		result[index++]=base32Alphabet[sha1[i+0]>>3];
		result[index++]=base32Alphabet[((sha1[i+0] & BINARY(111))<<2) | ((sha1[i+1]>>6) & BINARY(11))];
		result[index++]=base32Alphabet[(sha1[i+1]>>1) & BINARY(11111)];
		result[index++]=base32Alphabet[((sha1[i+1] & BINARY(1))<<4) | ((sha1[i+2]>>4) & BINARY(1111))];
		result[index++]=base32Alphabet[((sha1[i+2] & BINARY(1111))<<1) | ((sha1[i+3]>>7)& BINARY(01))];
		result[index++]=base32Alphabet[(sha1[i+3]>>2) & BINARY(11111)];
		result[index++]=base32Alphabet[((sha1[i+4]>>5) & BINARY(111)) | ((sha1[i+3] & BINARY(11))<<3)];
		result[index++]=base32Alphabet[sha1[i+4] & BINARY(11111)];
	}
	result[index]='\0';

	//theoretically we should trim off the leading 'A's since A is zero (A-Z, 2-7).
}


