#include "StdAfx.h"
#include ".\alu.h"
#include "ProgramMemory.h"
#include "Processor.h"


#pragma warning( disable : 4311 )
//
//
//
ALU::ALU(void)
{
	p_parent=NULL;
}

//
//
//
ALU::~ALU(void)
{
}

//
//
//
void ALU::InitParent(Processor *parent)
{
	p_parent=parent;
}

//
//
//
void ALU::SetZeroSignFlags(unsigned char *val)
{
	// Assume they are set, and then reset if needed
	p_parent->Zero(true);
	p_parent->Sign(true);

	if(val != 0)
	{
		p_parent->Zero(false);
	}
	if(((DWORD_PTR)val & 0x80000000) == 0)
	{
		p_parent->Sign(false);
	}
}

//
//
//
void ALU::SetZeroSignFlags(unsigned short int val)
{
	// Assume they are set, and then reset if needed
	p_parent->Zero(true);
	p_parent->Sign(true);

	if(val != 0)
	{
		p_parent->Zero(false);
	}
	if((val & 0x8000) == 0)
	{
		p_parent->Sign(false);
	}
}

//
//
//
void ALU::SetZeroSignFlags(unsigned char val)
{
	// Assume they are set, and then reset if needed
	p_parent->Zero(true);
	p_parent->Sign(true);

	if(val != 0)
	{
		p_parent->Zero(false);
	}
	if((val & 0x80) == 0)
	{
		p_parent->Sign(false);
	}
}

//
//
//
void ALU::SetCarryOverflowFlags(bool add,unsigned char *op1,unsigned char *op2,unsigned char *res)
{
	// Clear them, then reset them if necessary
	ClearCarryOverflow();

	// Create unsigned copies
	unsigned int uop1=(unsigned int)op1;
	unsigned int uop2=(unsigned int)op2;
	unsigned int ures=(unsigned int)res;

	if(add)
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Pos(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Neg(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}

		// Unsigned (Carry)
		if((ures < uop1) || (ures < uop2))
		{
			p_parent->Carry(true);
		}
	}
	else	// subtract
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Neg(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Pos(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}		

// *&* TEMP KLUDGE - begin
		bool one=false;
		bool two=false;
		if(uop1 < uop2)
		{
			one=true;
		}
		else if((res > op1) && (res > op2))
		{
			two=true;
		}

		if(one != two)
		{
			int poop=3;
		}
// *&* TEMP KLUDGE - end

		// Unsigned (Borrow)
		if(uop1 < uop2)
		{
			p_parent->Carry(true);
		}
	}
}

//
//
//
void ALU::SetCarryOverflowFlags(bool add,unsigned short int op1,unsigned short int op2,unsigned short int res)
{
	// Clear them, then reset them if necessary
	ClearCarryOverflow();

	// Create unsigned copies
	unsigned int uop1=(unsigned int)op1;
	unsigned int uop2=(unsigned int)op2;
	unsigned int ures=(unsigned int)res;

	if(add)
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Pos(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Neg(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}

		// Unsigned (Carry)
		if((ures < uop1) || (ures < uop2))
		{
			p_parent->Carry(true);
		}
	}
	else	// subtract
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Neg(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Pos(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}		

		// *&* TEMP KLUDGE - begin
		bool one=false;
		bool two=false;
		if(uop1 < uop2)
		{
			one=true;
		}
		else if((res > op1) && (res > op2))
		{
			two=true;
		}

		if(one != two)
		{
			int poop=3;
		}
		// *&* TEMP KLUDGE - end

		// Unsigned (Borrow)
		if(uop1 < uop2)
		{
			p_parent->Carry(true);
		}
	}
}

//
//
//
void ALU::SetCarryOverflowFlags(bool add,unsigned char op1,unsigned char op2,unsigned char res)
{
	// Clear them, then reset them if necessary
	ClearCarryOverflow();

	// Create unsigned copies
	unsigned int uop1=(unsigned int)op1;
	unsigned int uop2=(unsigned int)op2;
	unsigned int ures=(unsigned int)res;

	if(add)
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Pos(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Neg(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}

		// Unsigned (Carry)
		if((ures < uop1) || (ures < uop2))
		{
			p_parent->Carry(true);
		}
	}
	else	// subtract
	{
		// Signed (Overflow/Underflow)
		if(Pos(op1) && Neg(op2))
		{
			if(Neg(res))
			{
				p_parent->Overflow(true);	// Overflow
			}
		}
		else if(Neg(op1) && Pos(op2))
		{
			if(Pos(res))
			{
				p_parent->Overflow(true);	// Underflow
			}
		}		

		// *&* TEMP KLUDGE - begin
		bool one=false;
		bool two=false;
		if(uop1 < uop2)
		{
			one=true;
		}
		else if((res > op1) && (res > op2))
		{
			two=true;
		}

		if(one != two)
		{
			int poop=3;
		}
		// *&* TEMP KLUDGE - end

		// Unsigned (Borrow)
		if(uop1 < uop2)
		{
			p_parent->Carry(true);
		}
	}
}

//
//
//
bool ALU::Pos(unsigned char *val)
{
	if(((DWORD_PTR)val & 0x80000000) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ALU::Pos(unsigned short int val)
{
	if((val & 0x8000) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ALU::Pos(unsigned char val)
{
	if((val & 0x80) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ALU::Neg(unsigned char *val)
{
	return !Pos(val);
}

//
//
//
bool ALU::Neg(unsigned short int val)
{
	return !Pos(val);
}

//
//
//
bool ALU::Neg(unsigned char val)
{
	return !Pos(val);
}

bool ALU::Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char **op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::push:
		{
			p_parent->Push(*op1);
			break;
		}
		case ProgramMemory::op_code::add:
		{
			unsigned char *res=(unsigned char *)((DWORD_PTR)*op1+(DWORD_PTR)*op2);
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::mov:
		{
			*op1=*op2;
			break;
		}
		case ProgramMemory::op_code::xor:
		{
			*op1=(unsigned char *)((DWORD_PTR)*op1 ^ (DWORD_PTR)*op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::cmp:
		{
			unsigned char *ptr=(unsigned char *)(*op1-*op2);
			SetZeroSignFlags(ptr);
			SetCarryOverflowFlags(false,*op1,*op2,ptr);
			break;
		}
		case ProgramMemory::op_code::or:
		{
			*op1=(unsigned char *)((DWORD_PTR)*op1 | (DWORD_PTR)*op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::pop:
		{
			*op1 = p_parent->Pop();
			break;
		}
		case ProgramMemory::op_code::sub:
		{
			unsigned char *res = (unsigned char *)(*op1 - *op2);
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::inc:
		{
			unsigned char *res=*op1+1;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,(unsigned char *)1,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::and:
		{
			*op1 = (unsigned char *)((DWORD_PTR)*op1 & (DWORD_PTR)*op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::sbb:
		{
			unsigned char *src=*op2;

			// If the carry flag is set, add one to the source (op2) value
			if(p_parent->Carry())
			{
				src++;
			}

			unsigned char *res=(unsigned char *)((DWORD_PTR)*op1-(DWORD_PTR)src);
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,src,res);
			*op1=res;
            break;
		}
		case ProgramMemory::op_code::adc:
		{
			unsigned char *src=*op2;

			// If the carry flag is set, add one to the source (op2) value
			if(p_parent->Carry())
			{
				src++;
			}

			unsigned char *res=(unsigned char *)((DWORD_PTR)*op1+(DWORD_PTR)src);
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,src,res);
			*op1=res;

			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown d d");
			error=true;
			break;
		}
	}

	return error;
}
bool ALU::Process(ProgramMemory::op_code op,unsigned char **op1,unsigned short int *op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::mov:
		{
			unsigned short int *ptr=(unsigned short int *)op1;
			*ptr=(unsigned short int)*op2;
			break;
		}
		case ProgramMemory::op_code::sub:
		{
/*
			unsigned char *res = *op1-*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned char *)*op2,res);
			*op1=res;
*/
			unsigned short int *ptr=(unsigned short int *)op1;
			unsigned short int res=*ptr-*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*ptr,*op2,res);
			*ptr=res;

			break;
		}
		case ProgramMemory::op_code::or:
		{
/*
			*op1 = (unsigned char *)((DWORD_PTR)*op1 | *op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
*/
			unsigned short int *ptr=(unsigned short int *)op1;
			*ptr = *ptr | *op2;
			SetZeroSignFlags(*ptr);
			ClearCarryOverflow();

			break;
		}
		case ProgramMemory::op_code::xor:
		{
/*
			*op1 = (unsigned char *)((DWORD_PTR)*op1 ^ *op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
*/
			unsigned short int *ptr=(unsigned short int *)op1;
			*ptr = *ptr ^ *op2;
			SetZeroSignFlags(*ptr);
			ClearCarryOverflow();

			break;
		}
		case ProgramMemory::op_code::add:
		{
/*
			unsigned char *res = *op1 + *op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,(unsigned char *)*op2,res);
			*op1=res;
*/
			unsigned short int *ptr=(unsigned short int *)op1;
			unsigned short int res=*ptr+*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*ptr,*op2,res);
			*ptr=res;

			break;
		}
		case ProgramMemory::op_code::movzx:
		{
			*op1 = (unsigned char *)*op2;
			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown d w");
			error=true;
			break;
		}
	}

	return error;
}
bool ALU::Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char *op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::sub:
		{
/*
			unsigned char *res=*op1-*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned char *)*op2,res);
			*op1=res;
*/
			unsigned char *ptr=(unsigned char *)op1;
			unsigned char res=*ptr-*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*ptr,*op2,res);
			*ptr=res;

			break;
		}
		case ProgramMemory::op_code::or:
		{
/*
			*op1=(unsigned char *)((DWORD_PTR)*op1 | *op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
*/
			unsigned char *ptr=(unsigned char *)op1;
			*ptr = *ptr | *op2;
			SetZeroSignFlags(*ptr);
			ClearCarryOverflow();

			break;
		}
		case ProgramMemory::op_code::mov:
		{
			DWORD_PTR ptr=(DWORD_PTR)*op1;
            ptr &= 0xFFFFFF00;
			ptr |= *op2;
			*op1=(unsigned char *)ptr;
			break;
		}
		case ProgramMemory::op_code::xor:
		{
/*
			*op1 = (unsigned char *)((DWORD_PTR)*op1 ^ *op2);
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
*/
			unsigned char *ptr=(unsigned char *)op1;
			*ptr = *ptr ^ *op2;
			SetZeroSignFlags(*ptr);
			ClearCarryOverflow();

			break;
		}
		case ProgramMemory::op_code::add:
		{
/*
			unsigned char *res = *op1 + *op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,(unsigned char *)*op2,res);
			*op1 = res;
*/
			unsigned char *ptr=(unsigned char *)op1;
			unsigned char res=*ptr+*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*ptr,*op2,res);
			*ptr=res;

			break;
		}
		case ProgramMemory::op_code::movzx:
		{
			*op1 = (unsigned char *)*op2;
			break;
		}
		case ProgramMemory::op_code::shl:
		{
			DWORD_PTR ptr=(DWORD_PTR)*op1;

			// Check for overflow
			if(*op2 == 1)
			{
				if((ptr >> 31) == 1)
				{
					p_parent->Overflow(true);
				}
				else
				{
					p_parent->Overflow(false);
				}
			}
			else if(*op2 > 0 && *op2 <= 32)	// check for carry
			{
				DWORD_PTR mask=1;
				mask <<= (*op2-1);
				if((mask & ptr) == mask)
				{
					p_parent->Carry(true);
				}
				else
				{
					p_parent->Carry(false);
				}
			}

			// Do the shift and write it back out to the dest
			ptr <<= *op2;
			*op1=(unsigned char *)ptr;

			SetZeroSignFlags(*op1);

			break;
		}
		case ProgramMemory::op_code::shr:
		{
			DWORD_PTR ptr=(DWORD_PTR)*op1;

			// Check for overflow
			if(*op2 == 1)
			{
				if(((ptr >> 31) & 1) != ((ptr >> 30) & 1))
				{
					p_parent->Overflow(true);
				}
				else
				{
					p_parent->Overflow(false);
				}
			}
			else if(*op2 > 0 && *op2 <= 32)	// check for carry
			{
				DWORD_PTR mask=1;
				mask <<= (32-*op2);
				if((mask & ptr) == mask)
				{
					p_parent->Carry(true);
				}
				else
				{
					p_parent->Carry(false);
				}
			}

			// Do the shift and write it back out to the dest
			ptr >>= *op2;
			*op1=(unsigned char *)ptr;

			SetZeroSignFlags(*op1);

			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown d b");
			error=true;
			break;
		}
	}

	return error;
}

bool ALU::Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned char **op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::mov:
		{
			*op1=(unsigned short int)*op2;
			break;
		}
		case ProgramMemory::op_code::cmp:
		{
			unsigned short int res=*op1-(unsigned short int)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned short int)*op2,res);
			break;
		}
		case ProgramMemory::op_code::add:
		{
			unsigned short int res=*op1+(unsigned short int)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,(unsigned short int)*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::sub:
		{
			unsigned short int res=*op1 - (unsigned short int)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned short int)*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::or:
		{
			*op1 |= (unsigned short int)*op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::xor:
		{
			*op1 ^= (unsigned short int)*op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown w d");
			error=true;
			break;
		}
	}

	return error;
}
bool ALU::Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned short int *op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::xor:
		{
			*op1 ^= *op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::sub:
		{
			unsigned short int res=*op1 - *op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::or:
		{
			*op1 |= *op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::mov:
		{
			*op1 = *op2;
			break;
		}
		case ProgramMemory::op_code::add:
		{
			unsigned short int res=*op1 + *op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(true,*op1,*op2,res);
			*op1=res;
			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown w w");
			error=true;
			break;
		}
	}

	return error;
}
bool ALU::Process(ProgramMemory::op_code op,unsigned short int *op1,unsigned char *op2)
{
	p_parent->Log("w b");

	return true;
}

bool ALU::Process(ProgramMemory::op_code op,unsigned char *op1,unsigned char **op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::mov:
		{
			*op1=(unsigned char)*op2;
			break;
		}
		case ProgramMemory::op_code::cmp:
		{
			unsigned char res = *op1 - (unsigned char)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned char)*op2,res);
			break;
		}
		case ProgramMemory::op_code::xor:
		{
			*op1 = *op1 ^ (unsigned char)*op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::add:
		{
			unsigned char res = *op1 + (unsigned char)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned char)*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::and:
		{
			*op1 = *op1 & (unsigned char)*op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::or:
		{
			*op1 |= (unsigned char)*op2;
			SetZeroSignFlags(*op1);
			ClearCarryOverflow();
			break;
		}
		case ProgramMemory::op_code::sub:
		{
			unsigned char res=*op1 - (unsigned char)*op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,(unsigned char)*op2,res);
			*op1=res;
			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown b d");
			error=true;
			break;
		}
	}

	return error;
}

bool ALU::Process(ProgramMemory::op_code op,unsigned char *op1,unsigned short int *op2)
{
	p_parent->Log("b w");

	return true;
}
bool ALU::Process(ProgramMemory::op_code op,unsigned char *op1,unsigned char *op2)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::sub:
		{
			unsigned char res=*op1 - *op2;
			SetZeroSignFlags(res);
			SetCarryOverflowFlags(false,*op1,*op2,res);
			*op1=res;
			break;
		}
		case ProgramMemory::op_code::mov:
		{
			*op1=*op2;
			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown b b");
			error=true;
			break;
		}
	}

	return error;
}

//
//
//
bool ALU::Process(ProgramMemory::op_code op,unsigned char **op1,unsigned char **op2,unsigned char *op3)
{
	bool error=false;

	switch(op)
	{
		case ProgramMemory::op_code::shld:
		case ProgramMemory::op_code::shldcl:
		{
			UINT64 wide;
			wide=(DWORD_PTR)*op1;
			wide <<= 32;
			wide |= (DWORD_PTR)*op2;

			// Check for overflow
			if(*op3 == 1)
			{
				if(((wide >> 63) & 1) != ((wide >> 62) & 1))
				{
					p_parent->Overflow(true);
				}
				else
				{
					p_parent->Overflow(false);
				}
			}
			else if(*op3 > 0 && *op3 <= 64)	// check for carry
			{
				UINT64 mask=1;
				mask <<= (64-*op3);
				if((mask & wide) == mask)
				{
					p_parent->Carry(true);
				}
				else
				{
					p_parent->Carry(false);
				}
			}
			
			// Do the shift
			wide <<= *op3;

			// Copy back the operand results
//			*op2=(unsigned char *)wide;	*&* don't write out into a source, duh
			wide >>= 32;
			*op1=(unsigned char *)wide;

			SetZeroSignFlags(*op1);

			break;
		}
		case ProgramMemory::op_code::shrd:
		case ProgramMemory::op_code::shrdcl:
		{
			UINT64 wide;
			wide=(DWORD_PTR)*op2;
			wide <<= 32;
			wide |= (DWORD_PTR)*op1;

			// Check for overflow
			if(*op3 == 1)
			{
				if(((wide >> 31) & 1) != ((wide >> 32) & 1))
				{
					p_parent->Overflow(true);
				}
				else
				{
					p_parent->Overflow(false);
				}
			}
			else if(*op3 > 0 && *op3 <= 64)	// check for carry
			{
				UINT64 mask=1;
				mask <<= (*op3-1);
				if((mask & wide) == mask)
				{
					p_parent->Carry(true);
				}
				else
				{
					p_parent->Carry(false);
				}
			}
			
			// Do the shift
			wide >>= *op3;

			// Copy back the operand results
			*op1=(unsigned char *)wide;
			wide >>= 32;
//			*op2=(unsigned char *)wide;	*&* don't write out into a source, duh

			SetZeroSignFlags(*op1);

			break;
		}
		default:
		{
			p_parent->Log("ERROR : Unknown d d b");
			error=true;
			break;
		}
	}

	return error;
}

//
//
//
void ALU::ClearCarryOverflow()
{
	p_parent->Carry(false);
	p_parent->Overflow(false);
}