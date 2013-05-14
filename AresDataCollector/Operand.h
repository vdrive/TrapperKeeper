#pragma once

class Operand
{
public:
	Operand(void);
	~Operand(void);

	void Reset();

	enum reg
	{
		na,
		eax,
		ebx,
		ecx,
		edx,
		esi,
		edi,
		ebp,
		esp,
		eip,
		efl,
		esi_ebp	// when the op includes esi+ebp
	};

	enum width
	{
		ex,
		x,
		h,
		l
	};

	reg m_reg;				// is it eax, ebx, ecx, etc.
	width m_width;			// is it eax, ax, ah, al, etc.
	bool m_dereference;		// is it [eax] (or [eax + const] if const is non-zero
	unsigned char *m_const;	// is there a constant involved either by itself or reg + const
};
