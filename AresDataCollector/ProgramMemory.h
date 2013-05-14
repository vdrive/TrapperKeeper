#pragma once

#include "Operand.h"

// *&* TEMP KLUDGE - begin
#include <vector>
using namespace std;
// *&* TEMP KLUDGE - end
class Processor;
class Operation;

class ProgramMemory
{
public:
	enum op_code
	{
		push,
		add,
		mov,
		cmp,
		jbe,
		call,
		jmp,
		xor,
		sub,
		or,
		movzx,
		jnz,
		jle,
		jnb,
		sbb,
		jge,
		pop,
		and,
		retn,
		jl,
		adc,
		inc,
		shl,
		shld,	// the 3rd operand is the 2nd operand's constant, and the 2nd operand is a non-dereferenced reg
		shldcl,	// shld with cl as the 3rd operand
		shr,
		shrd,	// the 3rd operand is the 2nd operand's constant, and the 2nd operand is a non-dereferenced reg
		shrdcl	// shrd with cl as the 3rd operand
	};

	ProgramMemory(void);
	~ProgramMemory(void);
	void InitParent(Processor *parent);

	unsigned char *Fetch(unsigned char *addr,op_code &op,Operand &op1,Operand &op2);	// returns next address

private:
	Processor *p_parent;

	Operation **m_op_list;

	int m_last_index;
	int m_start_address;
	int m_end_address;
	int m_num_ops;

	// *&* TEMP KLUDGE - begin
	struct line
	{
		op_code m_op;
		Operand m_op1;
		Operand m_op2;
	};
	vector<line> v_lines;
	// *&* TEMP KLUDGE - end
};
