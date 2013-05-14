#include "StdAfx.h"
#include "utility.h"

Utility::Utility(void)
{
}

Utility::~Utility(void)
{
}

byte Utility::CharToByte(char ch){
	switch(ch){
		case 'A': return 0xA;break;
		case 'B': return 0xB;break;
		case 'C': return 0xC;break;
		case 'D': return 0xD;break;
		case 'E': return 0xE;break;
		case 'F': return 0xF;break;
		case '0': return 0x0;break;
		case '1': return 0x1;break;
		case '2': return 0x2;break;
		case '3': return 0x3;break;
		case '4': return 0x4;break;
		case '5': return 0x5;break;
		case '6': return 0x6;break;
		case '7': return 0x7;break;
		case '8': return 0x8;break;
		case '9': return 0x9;break;
		default: return 0x0;break;
	}
}

char Utility::ByteToChar(byte b){
	switch(b){
		case 0xA: return 'A';break;
		case 0xB: return 'B';break;
		case 0xC: return 'C';break;
		case 0xD: return 'D';break;
		case 0xE: return 'E';break;
		case 0xF: return 'F';break;
		case 0x0: return '0';break;
		case 0x1: return '1';break;
		case 0x2: return '2';break;
		case 0x3: return '3';break;
		case 0x4: return '4';break;
		case 0x5: return '5';break;
		case 0x6: return '6';break;
		case 0x7: return '7';break;
		case 0x8: return '8';break;
		case 0x9: return '9';break;
		default: return '0';break;
	}
}

void Utility::DecodeCompletionFromString(string &str,vector<byte> &v_completion)
{
	v_completion.clear();

	for(int i=0;i<(int)str.size();i+=2){
		char ch1=str[i];
		char ch2='0';
		if(i+1<(int)str.size())
			ch2=str[i+1];
		byte the_val1=Utility::CharToByte(ch1);
		byte the_val2=Utility::CharToByte(ch2);

		byte the_val=(the_val1<<4)|(the_val2);
		v_completion.push_back(the_val);
	}
}

byte Utility::PowerOf2(int power)
{
	switch(power){
		case 0: return 1;
		case 1: return 2;
		case 2: return 4;
		case 3: return 8;
		case 4: return 16;
		case 5: return 32;
		case 6: return 64;
		case 7: return 128;
	}
	return 1;
}