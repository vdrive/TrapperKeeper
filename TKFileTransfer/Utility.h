#pragma once

class Utility
{
public:
	Utility(void);
	~Utility(void);

	static byte CharToByte(char ch);
	static char ByteToChar(byte b);
	static void DecodeCompletionFromString(string &str,vector<byte> &v_completion);

	static byte PowerOf2(int power);

};
