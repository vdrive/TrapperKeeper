#pragma once

#include "ProgramMemory.h"
#include "Operand.h"

class Operation
{
public:
	unsigned int m_address;
	unsigned int m_next_address;
	ProgramMemory::op_code m_op_code;
	Operand m_operand1;
	Operand m_operand2;

	Operation(void);
	~Operation(void);
	
	void WriteToBuf(void *buf);
};
