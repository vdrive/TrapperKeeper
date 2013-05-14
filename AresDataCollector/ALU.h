#pragma once

#include "ProgramMemory.h"

class Processor;

class ALU
{
public:
	ALU(void);
	~ALU(void);
	void InitParent(Processor *parent);

	void SetZeroSignFlags(unsigned char *val);
	void SetZeroSignFlags(unsigned short int val);
	void SetZeroSignFlags(unsigned char val);
	void SetCarryOverflowFlags(bool add,unsigned char *op1,unsigned char *op2,unsigned char *res);
	void SetCarryOverflowFlags(bool add,unsigned short int op1,unsigned short int op2,unsigned short int res);
	void SetCarryOverflowFlags(bool add,unsigned char op1,unsigned char op2,unsigned char res);

	bool Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char **op2);
	bool Process(ProgramMemory::op_code op,unsigned char **op1,unsigned short int *op2);
	bool Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char *op2);

	bool Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned char **op2);
	bool Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned short int *op2);
	bool Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned char *op2);
	
	bool Process(ProgramMemory::op_code op,unsigned char *op1,unsigned char **op2);
	bool Process(ProgramMemory::op_code op,unsigned char *op1,unsigned short int *op2);
	bool Process(ProgramMemory::op_code op,unsigned char *op1,unsigned char *op2);

	bool Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char **op2,unsigned char *op3);

private:
	Processor *p_parent;

	void ClearCarryOverflow();

	bool Pos(unsigned char *val);
	bool Pos(unsigned short int val);
	bool Pos(unsigned char val);
	bool Neg(unsigned char *val);
	bool Neg(unsigned short int val);
	bool Neg(unsigned char val);
};
