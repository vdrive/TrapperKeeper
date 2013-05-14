#pragma once

class Register
{
public:
	Register(void);
	~Register(void);

	struct four_bytes
	{
		unsigned char m_ll;
		unsigned char m_lh;
		unsigned char m_hl;
		unsigned char m_hh;
	};

	struct two_shorts
	{
		unsigned short int m_x;
		unsigned short int m_y;
	};

	union reg
	{
		four_bytes m_four_bytes;
		two_shorts m_two_shorts;
		unsigned char * ex;
	};
/*
	// Read
	unsigned int ex();
	unsigned int x();
	unsigned int l();

	// Write
	void ex(unsigned int val);
	void x(unsigned short int val);
	void l(unsigned char val);

private:
	unsigned int m_register;
*/
};
