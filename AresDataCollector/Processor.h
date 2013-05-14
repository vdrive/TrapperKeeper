#pragma once

#include "Register.h"
#include "ProgramMemory.h"
#include "ALU.h"
#include "Operand.h"

#include <stack>
using namespace std;

class CAresDlg;

class Processor
{
public:
	Processor(void);
	~Processor(void);
	void InitParent(CAresDlg *parent);
	void SetEIP(unsigned char *addr);

	void Log(char *msg);

	void Push(unsigned char *val);
	unsigned char *Pop();

	void StartProcessor();	// runs
	void Step();	// steps

	void UpdateGUI(bool clear);

	void MoveEbxToEax();

	int m_ran_to;

	void ProcessData(unsigned char *sixteen_in,unsigned char *twenty_out);
	void ProcessSNData(unsigned char *eighty_in,unsigned char *twenty_out);
	void InitRegisters();

	//
	// Flags
	//

	// Mask Constants
	#define FLAG_CARRY		0x0001
	#define FLAG_PARITY		0x0004
	#define FLAG_AUX_CARRY	0x0010
	#define FLAG_ZERO		0x0040
	#define FLAG_SIGN		0x0080
	
	#define FLAG_TRAP		0x0100
	#define FLAG_INTERRUPT	0x0200
	#define FLAG_DIRECTION	0x0400
	#define FLAG_OVERFLOW	0x0800

	// Reading
	bool Carry();
	bool Parity();
	bool AuxCarry();
	bool Zero();
	bool Sign();

	bool Trap();
	bool Interrupt();
	bool Direction();
	bool Overflow();

	// Writing
	void Carry(bool val);
	void Parity(bool val);
	void AuxCarry(bool val);
	void Zero(bool val);
	void Sign(bool val);

	void Trap(bool val);
	void Interrupt(bool val);
	void Direction(bool val);
	void Overflow(bool val);

private:
	CAresDlg *p_parent;

	// Program Memory
	ProgramMemory m_pm;

	void FindReg(Operand &op,unsigned char ***ex,unsigned short int **x,unsigned char **h,unsigned char **l);

	// Registers
	Register::reg m_eax,m_ebx,m_ecx,m_edx,m_esi,m_edi,m_ebp,m_esp,m_eip,m_efl;
	Register::reg m_esi_ebp;	// when the op includes esi+ebp

	// ALU
	ALU m_alu;
		
	// Stack
//	stack<unsigned char *> m_stack;
	unsigned char *p_stack;
	unsigned int m_stack_len;
//	unsigned char m_stack[1024];	// 1 KB stack

	void Run(bool step);

	unsigned char m_data[512];

	void Sixteen2FiveTwelve(unsigned char *sixteen,unsigned char *fivetwelve);
	void Eighty2FiveTwelve(unsigned char *eighty,unsigned char *fivetwelve);

	unsigned int m_lines_executed;
};
