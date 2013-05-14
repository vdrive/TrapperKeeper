#pragma once
#include "AresPacket.h"

class AresUtilityClass
{
public:
	static unsigned char sm_data1[512];
	static unsigned char sm_data2[512];
	static unsigned char sm_data3[512];
	static bool smb_inited_data;

	AresUtilityClass(void);
	~AresUtilityClass(void);
	
	static void DecryptFirstMessage(AresPacket &packet,unsigned int ip, unsigned short &m_state_short,byte &m_state_byte,unsigned short remote_port, bool b_0x38_message);
	static void CalculateRemoteGUIDSha1(byte* remote_guid, byte* sha1,unsigned short state_short,byte state_byte);
	static unsigned short int FourPissAlgorithm(unsigned short four_shit_short,unsigned char four_shit_byte,unsigned short the_short);
	static unsigned short int CalculateTwoBytesFromBuffer(unsigned char *buf,unsigned int len);
	static unsigned char CalcStateByte(unsigned char input,unsigned int &state);
	static unsigned short int CalcStateShort(unsigned short int input,unsigned int &state);
	static void EncryptMessage(unsigned char *buf,unsigned int len,unsigned short int pes);
	static unsigned short int GetEncryptionShort(byte state_byte);
	static void DecryptMessage(AresPacket &packet,unsigned short &m_fss,byte &m_fsb);
	static unsigned short int FourShitAlgorithm(unsigned char first_byte,unsigned short &m_fss,byte &m_fsb);

	static unsigned short int DecryptFTPacket(unsigned char *buf,unsigned int len,unsigned short int decrypt_code);
	static unsigned short int EncryptFTPacket(unsigned char *buf,unsigned int len,unsigned short int encrypt_code);
	static unsigned short GetConst591da8(int offset);

	static CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen);

	static byte ConvertCharToInt(char ch)
	{
		switch(ch){
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'A': return 10;
			case 'a': return 10;
			case 'B': return 11;
			case 'b': return 11;
			case 'C': return 12;
			case 'c': return 12;
			case 'D': return 13;
			case 'd': return 13;
			case 'E': return 14;
			case 'e': return 14;
			case 'F': return 15;
			case 'f': return 15;
		}
		return 0;
	}

	static void EncodeHashFromString(byte *hash,int hash_len,const char *str)
	{
		for(int j=0;j<hash_len;j++){
			char ch1=str[j*2];
			char ch2=str[j*2+1];
			byte val1=ConvertCharToInt(ch1);
			byte val2=ConvertCharToInt(ch2);
			byte hash_val=((val1&0xf)<<4)|(val2&0xf);
			hash[j]=hash_val;
		}				

	}


private:
	static void InitData();
	
	static unsigned int Bound(unsigned int val);
	static void Loop123(unsigned char *data, unsigned char *const_data, unsigned int len);
	static void Loop4(unsigned char *data,unsigned char *edi,unsigned char *ecx);
	
public:
	static void PrintByteArray(byte* data, unsigned int len);
	static unsigned short CalculateIPCode(unsigned int ip);
	static void EncryptFirstMessage(unsigned short& state_short, byte& state_byte, unsigned short my_port,AresPacket &packet);
	static bool IsHashFakeByByte(byte* hash);
	static bool IsHashFakeByString(const char* hash);
};
