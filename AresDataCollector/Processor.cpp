#include "StdAfx.h"
#include ".\processor.h"

#include "..\tkcom\sha1.h"

#pragma warning (disable : 4312)
#pragma warning (disable : 4311)
#pragma warning (disable : 4800)

//
//
//
Processor::Processor(void)
{
	p_parent=NULL;
	p_stack=NULL;

	m_pm.InitParent(this);
	m_alu.InitParent(this);
/***
	// Read in the 0x38 message sixteen
	CStdioFile file;
	if(file.Open("input.txt",CFile::typeText|CFile::modeRead|CFile::shareDenyNone)==FALSE)
	{
		MessageBox(NULL,"Error opening file input.txt","Error",MB_OK);
		return;
	}

	unsigned char sixteen[16];
	memset(sixteen,0,sizeof(sixteen));
	unsigned int index=0;
	char buf[1024];
	while((file.ReadString(buf,sizeof(buf))!=NULL) && (index < sizeof(sixteen)))
	{
		// Whack any comments
		if(strchr(buf,';')!=NULL)
		{
			*strchr(buf,';')='\0';
		}

		// Trim whitespace
		CString cstring=buf;
		cstring.Trim();
		strcpy(buf,cstring);

		// See if it was all whitespace
		if(strlen(buf)==0)
		{
			continue;
		}

		// Point at the last character and then rewind until we hit a space or we rewind all the way
		char *ptr=&buf[strlen(buf)-1];
		while(ptr != buf)
		{
			// We are pointing at whitepsace, go back one, and break
			if(*ptr == ' ')
			{
				ptr++;
				break;
			}
			else
			{
				ptr--;
			}
		}

		// We should be pointing at the number
		sixteen[index++]=(unsigned char)strtoul(ptr,NULL,16);
	}

	// Turn the sixteen from the 0x38 message into the 512 to be processed
	memset(m_data,0,sizeof(m_data));
	Sixteen2FiveTwelve(sixteen,m_data);
***/
	// Allocate and initialize the stack
//	m_stack_size=1*1024*1024;	// 1 MB
//	m_stack_size=1*1024;		// 1 KB
//	p_stack=new unsigned char[m_stack_size];
//	memset(p_stack,0,m_stack_size);
//	memset(p_stack,0xcabba6e5,m_stack_size);	// set uninitialized stack to cabbages

	InitRegisters();
}

//
//
//
Processor::~Processor(void)
{
	// Free the stack
	if(p_stack!=NULL)
	{
		delete [] p_stack;
	}
}

//
//
//
void Processor::InitParent(CAresDlg *parent)
{
	p_parent=parent;
}

//
//
//
void Processor::InitRegisters()
{
//	memset(m_stack,0x69,sizeof(m_stack));
	m_stack_len=10*1024;	// 10 KB (was 4 KB but needed to be increased for SN processing)
	if(p_stack!=NULL)
	{
		delete [] p_stack;
	}
	p_stack=new unsigned char[m_stack_len];
	memset(p_stack,0x69,m_stack_len);

	// Init the registers
	m_eax.ex=m_data;
	m_ebx.ex=m_data;
//	m_ecx.ex=(unsigned char *)0xdeadbeef;
	m_ecx.ex=0;
	m_edx.ex=m_data;
	
	m_esi.ex=m_data;
//	m_edi.ex=(unsigned char *)0xdeadbeef;
	m_edi.ex=(unsigned char *)0x67;
	
//	m_ebp.ex=p_stack + (m_stack_size*1/3);	// set the stack pointer to point 1/3 down the stack
//	m_esp.ex=p_stack + (m_stack_size*2/3);	// set the stack pointer to point 2/3 down the stack
/*
	m_ebp.ex=m_stack + (sizeof(m_stack)*1/3);	// set the stack pointer to point 1/3 down the stack
	m_esp.ex=m_stack + (sizeof(m_stack)*2/3);	// set the stack pointer to point 2/3 down the stack
*/
	m_esp.ex=p_stack + m_stack_len/2;	// put the stack ptr in the middle of the stack
	m_ebp.ex=(DWORD_PTR)m_esp.ex+(unsigned char *)(0x297fe68-0x0297fe34);

	m_eip.ex=(unsigned char *)0xdeadbeef;	// init instruction pointer to not initialized
	m_efl.ex=0;								// init flags to all not set

	m_lines_executed=0;

	m_ran_to=0;
}

//
//
//
void Processor::SetEIP(unsigned char *addr)
{
	// Set the instruction pointer
	m_eip.ex=addr;
}

//
//
//
void Processor::StartProcessor()
{
	// Run
	Run(false);

	// Update the GUI
	UpdateGUI(false);
}

//
//
//
void Processor::Step()
{
	// Step
	Run(true);

	// Update the GUI
	UpdateGUI(true);
}

//
//
//
void Processor::Run(bool step)
{
	ProgramMemory::op_code op;
	Operand op1,op2;

	// Set the address to fetch to be equal to the instruction pointer

	while(1)
	{
		if(m_eip.ex == (unsigned char *)0x755238)
		{
			int poop=3;
			
			if(m_ran_to==0)
			{
				m_ran_to++;
//				return;
			}
		}
/*
		if(m_ran_to==1)
		{
			if(m_eip.ex == (unsigned char *)0x587244)
			{
				m_ran_to++;
				return;
			}
		}
*/
#ifdef SUPERNODE
		if(m_eip.ex == (unsigned char *)0x579314)
		{
			Log("Done ... Reached 0x579314");
/*
			// Sha the data, just to see if it is it. Yup, that's it! 
			unsigned char sha1[20];
			memset(sha1,0,sizeof(sha1));

			unsigned char tmp[0x202];
			memset(tmp,0,sizeof(tmp));
			memcpy(&tmp[1],m_data,sizeof(m_data));
			tmp[sizeof(tmp)-1]=0xff;

			CryptoPP::SHA1 sha;
			sha.CalculateDigest(sha1,tmp,sizeof(tmp));
*/
			break;
		}
#else
        // Check to see if we are done
		if(m_eip.ex == (unsigned char *)0x75525b)
		{
			Log("Done ... Reached 0x75525b");

/*
			// Sha the data, just to see if it is it. Yup, that's it! 
			unsigned char sha1[20];
			memset(sha1,0,sizeof(sha1));

			CryptoPP::SHA1 sha;
			sha.CalculateDigest(sha1,m_data,sizeof(m_data));
*/
			break;
		}
#endif

		if(m_lines_executed == 382904)
		{
			int poop=3;
//			return;
		}

		// Fetch the line of code
		unsigned char *ret=m_pm.Fetch(m_eip.ex,op,op1,op2);

		// Check to see if the address was not found
		if(ret == 0)
		{
			char msg[1024];
			sprintf(msg,"Fetch(0x%08x) returned 0",(unsigned int)m_eip.ex);
			Log(msg);
			break;
		}
		else
		{
			// Else the address was found
			m_eip.ex=ret;
			m_lines_executed++;
		}

		// If it is a jump or call or ret
		switch(op)
		{
			case ProgramMemory::op_code::call:
			{
				Push(m_eip.ex);			// where we are going to return to
				m_eip.ex=op1.m_const;	// make the call
				break;
			}
			case ProgramMemory::op_code::jge:
			{
				// Jump if greater than or equal
				if(Sign() == Overflow())
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::jl:
			{
				// Jump if less than
				if(Sign() != Overflow())
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::jbe:
			{
				// Jump if below or equal
				if(Carry() || Zero())
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::jle:
			{
				// Jump if less than or equal
				if(Zero() || (Sign() != Overflow()))
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::jmp:
			{
				m_eip.ex=op1.m_const;	// just jump
				break;
			}
			case ProgramMemory::op_code::jnb:
			{
				// Jump if not below
				if(!Carry())
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::jnz:
			{
				// Jump if zero is not set
				if(!Zero())
				{
					m_eip.ex=op1.m_const;	// jump
				}
				else
				{
					// Get the next line of code
					m_eip.ex=m_eip.ex;
				}

				break;
			}
			case ProgramMemory::op_code::retn:
			{
				m_eip.ex=Pop();		// return from the call

				// *&* TEMP KLUDGE - begin
				if(op1.m_const == (unsigned char *)0xdeadbeef)
				{
					op1.m_const=0;
				}
				// *&* TEMP KLUDGE - end

				// "Pop" any extra bytes if needed
				m_esp.ex=(unsigned char *)((DWORD_PTR)m_esp.ex+(DWORD_PTR)op1.m_const);

				break;
			}
			default:	// ALU op code
			{
				// Figure out which var they are talking about here
				unsigned char **ex1=NULL;
				unsigned char **ex2=NULL;
				unsigned short int *x1=NULL;
				unsigned short int *x2=NULL;
				unsigned char *h1=NULL;
				unsigned char *h2=NULL;
				unsigned char *l1=NULL;
				unsigned char *l2=NULL;

				// *&* TEMP KLUDGE - begin
				if(op2.m_reg == Operand::na && op2.m_const == (unsigned char *)0xdeadbeef &&
					op != ProgramMemory::op_code::push && 
					op != ProgramMemory::op_code::pop &&
					op != ProgramMemory::op_code::inc)
				{
					int poop=3;
				}

				if(op1.m_const == (unsigned char *)0xdeadbeef)
				{
					op1.m_const=0;
				}
				if(op2.m_const == (unsigned char *)0xdeadbeef)
				{
					op2.m_const=0;
				}
				// *&* TEMP KLUDGE - end

				// If the op regs are not NA, figure out which reg they are pointing at
				if(op1.m_reg != Operand::reg::na)
				{
					FindReg(op1,&ex1,&x1,&h1,&l1);
				}
				if(op2.m_reg != Operand::reg::na)
				{
					FindReg(op2,&ex2,&x2,&h2,&l2);
				}

				// If there is a dereference, then just pass the pointer in, else pass in a reference to the pointer
				unsigned char **dex1=NULL;
				unsigned char **dex2=NULL;
				unsigned short int *dx1=NULL;
				unsigned short int *dx2=NULL;
				unsigned char *dl1=NULL;
				unsigned char *dl2=NULL;

				if(op1.m_dereference)
				{
					switch(op1.m_width)
					{
						case Operand::ex:
						{
							dex1=(unsigned char **)(*ex1);
							dex1=(unsigned char **)((DWORD_PTR)dex1+(DWORD_PTR)op1.m_const);
							break;
						}
						case Operand::x:
						{
							unsigned char **tmp=(unsigned char **)(*ex1);
							tmp=(unsigned char **)((DWORD_PTR)tmp+(DWORD_PTR)op1.m_const);
							dx1=(unsigned short *)tmp;
							break;
						}
						case Operand::l:
						{
							unsigned char **tmp=(unsigned char **)(*ex1);
							tmp=(unsigned char **)((DWORD_PTR)tmp+(DWORD_PTR)op1.m_const);
							dl1=(unsigned char *)tmp;
							break;
						}
						default:
						{
							Log("Missing op1 dereference");
							return;

							break;
						}
					}
				}
				if(op2.m_dereference)
				{
					switch(op2.m_width)
					{
						case Operand::ex:
						{
							dex2=(unsigned char **)(*ex2);
							dex2=(unsigned char **)((DWORD_PTR)dex2+(DWORD_PTR)op2.m_const);
							break;
						}
						case Operand::x:
						{
							unsigned char **tmp=(unsigned char **)(*ex2);
							tmp=(unsigned char **)((DWORD_PTR)tmp+(DWORD_PTR)op2.m_const);
							dx2=(unsigned short *)tmp;
							break;
						}
						case Operand::l:
						{
							unsigned char **tmp=(unsigned char **)(*ex2);
							tmp=(unsigned char **)((DWORD_PTR)tmp+(DWORD_PTR)op2.m_const);
							dl2=(unsigned char *)tmp;

							break;
						}
						default:
						{
							Log("Missing op2 dereference");
							return;

							break;
						}
					}
				}

				bool error=false;

				// Call the appropriate process function

				// First check the 3 operand op codes
				if(op == ProgramMemory::op_code::shld || op == ProgramMemory::op_code::shldcl || 
					op == ProgramMemory::op_code::shrd || op == ProgramMemory::op_code::shrdcl)
				{
					// Assume it is sh*dcl and set the third operand byte to cl
					unsigned char *l3=&m_ecx.m_four_bytes.m_ll;
					
					// If the third operand is actually a passed in constant
					unsigned char c=(unsigned char)op2.m_const;
					if(op == ProgramMemory::op_code::shld || op == ProgramMemory::op_code::shrd)
					{
						l3=&c;						
					}

					// Make sure that all the appropriate registers are set
					if(ex1 == NULL || ex2 == NULL || dex2 != NULL)
					{
						Log("Unknown sh*d operand combination");
						error=true;
					}
					else if(dex1!=NULL)	// only op1 can be dereferenced (I think)
					{
						error=m_alu.Process(op,dex1,ex2,l3);
					}
					else
					{
						error=m_alu.Process(op,ex1,ex2,l3);
					}
				}
                else if(dex1!=NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,dex1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,dex1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,dex1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,dex1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,dex1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,dex1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,dex1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,dex1,&op2.m_const);
					}
				}
				else if(dx1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,dx1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,dx1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,dx1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,dx1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,dx1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,dx1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,dx1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,dx1,&op2.m_const);
					}
				}
				else if(dl1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,dl1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,dl1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,dl1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,dl1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,dl1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,dl1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,dl1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,dl1,&op2.m_const);
					}
				}
				else if(ex1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,ex1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,ex1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,ex1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,ex1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,ex1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,ex1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,ex1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,ex1,&op2.m_const);
					}
				}
				else if(x1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,x1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,x1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,x1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,x1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,x1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,x1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,x1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,x1,&op2.m_const);
					}
				}
				else if(h1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,h1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,h1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,h1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,h1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,h1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,h1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,h1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,h1,&op2.m_const);
					}
				}
				else if(l1 != NULL)
				{
					if(dex2 != NULL)
					{
						error=m_alu.Process(op,l1,dex2);
					}
					else if(dx2 != NULL)
					{
						error=m_alu.Process(op,l1,dx2);
					}
					else if(dl2 != NULL)
					{
						error=m_alu.Process(op,l1,dl2);
					}
					else if(ex2 != NULL)
					{
						error=m_alu.Process(op,l1,ex2);
					}
					else if(x2 != NULL)
					{
						error=m_alu.Process(op,l1,x2);
					}
					else if(h2 !=NULL)
					{
						error=m_alu.Process(op,l1,h2);
					}
					else if(l2 != NULL)
					{
						error=m_alu.Process(op,l1,l2);
					}
					else	// constant or no op2
					{
						error=m_alu.Process(op,l1,&op2.m_const);
					}
				}
				else
				{
					Log("ERROR? : op1 is a constant for an ALU call");
				}

				if(error)
				{
					return;
				}

				break;
			}
		}

		// If we are stepping, then break out
		if(step)
		{
			break;
		}
	}
}

//
//
//
void Processor::Push(unsigned char *val)
{
//	m_stack.push(val);

	m_esp.ex-=4;

	// Check stack bounds
	if(m_esp.ex >= p_stack+m_stack_len)
	{
		MessageBox(NULL,"Attempting to push beyond upper stack boundary","Stack Error",MB_OK);
		return;
	}
	else if(m_esp.ex < p_stack)
	{
		MessageBox(NULL,"Attempting to push beyond lower stack boundary","Stack Error",MB_OK);
		return;
	}

	memcpy(m_esp.ex,&val,4);
}

//
//
//
unsigned char *Processor::Pop()
{
//	unsigned char *ret=m_stack.top();
//	m_stack.pop();
//	return ret;

	unsigned char *ret=NULL;

	// Check stack bounds
	if(m_esp.ex >= p_stack+m_stack_len)
	{
		MessageBox(NULL,"Attempting to pop from beyond upper stack boundary","Stack Error",MB_OK);
		return 0;
	}
	else if(m_esp.ex < p_stack)
	{
		MessageBox(NULL,"Attempting to pop from beyond lower stack boundary","Stack Error",MB_OK);
		return 0;
	}

	memcpy(&ret,m_esp.ex,4);

	if(ret == (unsigned char *)0xcabba6e5)
	{
		int poop=3;
	}

	m_esp.ex+=4;

	// Check for uninitialized
	if(ret == (unsigned char *)0x69696969)
	{
		int poop=3;
	}

	return ret;
}

//
//
//
void Processor::FindReg(Operand &op,unsigned char ***ex,unsigned short int **x,unsigned char **h,unsigned char **l)
{
	Operand::reg reg=op.m_reg;
	Operand::width width=op.m_width;

	switch(reg)
	{
		case Operand::reg::eax:
		{
			switch(width)
			{
				case Operand::width::ex:
				{
					*ex=&m_eax.ex;
					break;
				}
				case Operand::width::x:
				{
					// If it is x and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_eax.ex;
					}
					else
					{
						*x=&m_eax.m_two_shorts.m_x;
					}
					break;
				}
				case Operand::width::h:
				{
					*h=&m_eax.m_four_bytes.m_lh;
					break;
				}
				case Operand::width::l:
				{
					// If it is l and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_eax.ex;
					}
					else
					{
						*l=&m_eax.m_four_bytes.m_ll;
					}
					break;
				}
			}
			break;
		}
		case Operand::reg::ebx:
		{
			switch(width)
			{
				case Operand::width::ex:
				{
					*ex=&m_ebx.ex;
					break;
				}
				case Operand::width::x:
				{
					// If it is x and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_ebx.ex;
					}
					else
					{
						*x=&m_ebx.m_two_shorts.m_x;
					}
					break;
				}
				case Operand::width::h:
				{
					*h=&m_ebx.m_four_bytes.m_lh;
					break;
				}
				case Operand::width::l:
				{
					// If it is l and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_ebx.ex;
					}
					else
					{
						*l=&m_ebx.m_four_bytes.m_ll;
					}
					break;
				}
			}
			break;
		}
		case Operand::reg::ecx:
		{
			switch(width)
			{
				case Operand::width::ex:
				{
					*ex=&m_ecx.ex;
					break;
				}
				case Operand::width::x:
				{
					// If it is x and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_ecx.ex;
					}
					else
					{
						*x=&m_ecx.m_two_shorts.m_x;
					}
					break;
				}
				case Operand::width::h:
				{
					*h=&m_ecx.m_four_bytes.m_lh;
					break;
				}
				case Operand::width::l:
				{
					// If it is l and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_ecx.ex;
					}
					else
					{
						*l=&m_ecx.m_four_bytes.m_ll;
					}
					break;
				}
			}
			break;
		}
		case Operand::reg::edx:
		{
			switch(width)
			{
				case Operand::width::ex:
				{
					*ex=&m_edx.ex;
					break;
				}
				case Operand::width::x:
				{
					// If it is x and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_edx.ex;
					}
					else
					{
						*x=&m_edx.m_two_shorts.m_x;
					}
					break;
				}
				case Operand::width::h:
				{
					*h=&m_edx.m_four_bytes.m_lh;
					break;
				}
				case Operand::width::l:
				{
					// If it is l and a dereference, then set ex in preparation for the byte dereference
					if(op.m_dereference)
					{
						*ex=&m_edx.ex;
					}
					else
					{
						*l=&m_edx.m_four_bytes.m_ll;
					}
					break;
				}
			}
			break;
		}
		case Operand::reg::esi:
		{
			*ex=&m_esi.ex;
			break;
		}
		case Operand::reg::edi:
		{
			*ex=&m_edi.ex;
			break;
		}
		case Operand::reg::ebp:
		{
			*ex=&m_ebp.ex;
			break;
		}
		case Operand::reg::esp:
		{
			*ex=&m_esp.ex;
			break;
		}
		case Operand::reg::eip:
		{
			*ex=&m_eip.ex;
			break;
		}
		case Operand::reg::efl:
		{
			*ex=&m_efl.ex;
			break;
		}
		case Operand::reg::esi_ebp:	// when the op includes esi+ebp
		{
			m_esi_ebp.ex=(unsigned char *)((DWORD_PTR)m_esi.ex + (DWORD_PTR)m_ebp.ex);
			*ex=&m_esi_ebp.ex;
			break;
		}
		default:
		{
			// Should never get here
			Log("ERROR : FindReg() default case met");
			break;
		}
	}

	// If ex != NULL, then add the constant to it
	if(*ex != NULL)
	{
//		**ex += (DWORD_PTR)op.m_const;	*&* NO! This changes the actual register value, which is bad! *&*
	}
	else
	{
		if(op.m_const != 0)
		{
			Log("ERROR? : Processor::FindReg() op.m_const != 0 but ex == NULL");
		}
	}
}

//
//
//
void Processor::Log(char *msg)
{
//***	p_parent->Log(msg);
}

//
//
//
void Processor::UpdateGUI(bool clear)
{
/***
	char buf[1024];

	sprintf(buf,"%08x",m_eax.ex);
	p_parent->m_eax_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_ebx.ex);
	p_parent->m_ebx_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_ecx.ex);
	p_parent->m_ecx_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_edx.ex);
	p_parent->m_edx_edit.SetWindowText(buf);


	sprintf(buf,"%08x",m_esi.ex);
	p_parent->m_esi_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_edi.ex);
	p_parent->m_edi_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_ebp.ex);
	p_parent->m_ebp_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_esp.ex);
	p_parent->m_esp_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_eip.ex);
	p_parent->m_eip_edit.SetWindowText(buf);

	sprintf(buf,"%08x",m_efl.ex);
	p_parent->m_efl_edit.SetWindowText(buf);

	sprintf(buf,"%u Lines Executed",m_lines_executed);
//	p_parent->SetWindowText(buf);
	p_parent->GetDlgItem(IDC_Lines_Static)->SetWindowText(buf);

#ifdef _DEBUG
	// If clear is set, wipe the list
	if(clear)
	{
		p_parent->m_list.ResetContent();
	}

	// Write the data out
	for(int i=0;i<sizeof(m_data);i++)
	{
		sprintf(buf,"%08x - %02x",&m_data[i],m_data[i]);
		p_parent->m_list.AddString(buf);
	}
#endif
***/
}

//
//
//
void Processor::MoveEbxToEax()
{
	// *&* KLUDGE
	m_ebx.ex=m_data;	
	m_esi.ex=m_data;

	m_eax.ex=m_ebx.ex;
}





















//
// Flags
//

// Reading
bool Processor::Carry(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_CARRY);}
bool Processor::Parity(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_PARITY);}
bool Processor::AuxCarry(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_AUX_CARRY);}
bool Processor::Zero(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_ZERO);}
bool Processor::Sign(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_SIGN);}

bool Processor::Trap(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_TRAP);}
bool Processor::Interrupt(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_INTERRUPT);}
bool Processor::Direction(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_DIRECTION);}
bool Processor::Overflow(){return (bool)(m_efl.m_two_shorts.m_x & FLAG_OVERFLOW);}

// Writing
void Processor::Carry(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_CARRY;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_CARRY;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Parity(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_PARITY;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_PARITY;	// if we are supposed to clear it, then clear it
	}
}
void Processor::AuxCarry(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_AUX_CARRY;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_AUX_CARRY;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Zero(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_ZERO;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_ZERO;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Sign(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_SIGN;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_SIGN;	// if we are supposed to clear it, then clear it
	}
}

void Processor::Trap(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_TRAP;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_TRAP;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Interrupt(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_INTERRUPT;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_INTERRUPT;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Direction(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_DIRECTION;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_DIRECTION;	// if we are supposed to clear it, then clear it
	}
}
void Processor::Overflow(bool val)
{
	m_efl.m_two_shorts.m_x |= FLAG_OVERFLOW;	// set it
	if(!val)
	{
		m_efl.m_two_shorts.m_x ^= FLAG_OVERFLOW;	// if we are supposed to clear it, then clear it
	}
}

//
//
//
void Processor::Sixteen2FiveTwelve(unsigned char *sixteen,unsigned char *fivetwelve)
{
	// Take the sha1 of the sixteen
	unsigned char sha1[20];
	memset(sha1,0,sizeof(sha1));

	SHA1 sha;
	//CryptoPP::SHA1 sha;
	sha.Update(sixteen,16);
	sha.Final();

	sha.GetHash(sha1);
	//sha.CalculateDigest(sha1,sixteen,16);

	unsigned char data[1024];
	memset(data,0,sizeof(data));
	unsigned int len=0;

	// Copy the sha1 to the data buffer
	memcpy(data,sha1,sizeof(sha1));
	len=sizeof(sha1);

	unsigned char h=0x80;
	unsigned char f=0x80;

	while(len < 512)	// 0x200
	{
		unsigned char buf[1024];
		memset(buf,0,sizeof(buf));
		unsigned char *ptr=buf;

		// Copy incrementing header byte
		*ptr++=h++;

		// Copy existing data
		memcpy(ptr,data,len);
		ptr+=len;

		// Copy decrementing footer byte
		*ptr++=f--;


		SHA1 sha_tmp;
		//CryptoPP::SHA1 sha;
		sha_tmp.Update(buf,(UINT)(ptr-buf));
		sha_tmp.Final();

		sha_tmp.GetHash(sha1);
		// Take the sha1 of the data
		//sha.CalculateDigest(sha1,buf,ptr-buf);		

		// Append this new sha to the old sha
		memcpy(&data[len],sha1,sizeof(sha1));
		len+=sizeof(sha1);
	}

	memcpy(fivetwelve,data,512);
}

//
//
//
void Processor::ProcessData(unsigned char *sixteen_in,unsigned char *twenty_out)
{
	memset(m_data,0,sizeof(m_data));
	Sixteen2FiveTwelve(sixteen_in,m_data);

	// Process (bork) the data
	InitRegisters();
	SetEIP((unsigned char *)0x7551d1);
	StartProcessor();

	// Take sha of the borked 512
	unsigned char sha1[20];

	SHA1 sha;
	sha.Update(m_data,sizeof(m_data));
	sha.Final();
	sha.GetHash(sha1);

	//CryptoPP::SHA1 sha;
	//sha.CalculateDigest(sha1,m_data,sizeof(m_data));

	// sha1 is the 0x00 20
	memcpy(twenty_out,sha1,20);
}

//
//
//
void Processor::Eighty2FiveTwelve(unsigned char *eighty,unsigned char *fivetwelve)
{
	// Create the 0x200 (512) from the 0x80 (128)
	unsigned char buf[1024];
	unsigned char tmp[1024];

	// Copy the 0x80 to buf
	unsigned len=0x80;
	memset(buf,0,sizeof(buf));
	memcpy(buf,eighty,len);

	//CryptoPP::SHA1 sha;
	unsigned char sha1[20];

	// Calculate the sha of buf and append it to buf and repeat 20 times
	for(int i=0;i<20;i++)
	{
		memset(tmp,0,sizeof(tmp));
		tmp[0]=0x00;
		memcpy(&tmp[1],buf,len);
		tmp[len+1]=0xff;

		SHA1 sha;
		sha.Update(tmp,len+2);
		sha.Final();
		sha.GetHash(sha1);

		//sha.CalculateDigest(sha1,tmp,len+2);

		memcpy(&buf[len],sha1,sizeof(sha1));
		len+=sizeof(sha1);
	}

	// buf is now the 0x210 bytes, so trim it to 0x200 bytes
	memset(&buf[0x200],0,sizeof(buf)-0x200);
	len=0x200;

	// Copy the 512
	memcpy(fivetwelve,buf,len);
}

//
//
//
void Processor::ProcessSNData(unsigned char *eighty_in,unsigned char *twenty_out)
{
	memset(m_data,0,sizeof(m_data));
	Eighty2FiveTwelve(eighty_in,m_data);

	// Process (bork) the data
	InitRegisters();
	SetEIP((unsigned char *)0x57930f);
	StartProcessor();

	// Take sha of the borked 512 + a 1 byte header (0x00) and a 1 byte footer (0xFF)
	unsigned char sha1[20];
	memset(sha1,0,sizeof(sha1));

	unsigned char tmp[0x202];
	memset(tmp,0,sizeof(tmp));
	memcpy(&tmp[1],m_data,sizeof(m_data));
	tmp[sizeof(tmp)-1]=0xff;

	SHA1 sha;

	sha.Update(tmp,sizeof(tmp));
	sha.Final();
	sha.GetHash(sha1);

//	CryptoPP::SHA1 sha;
//	sha.CalculateDigest(sha1,tmp,sizeof(tmp));

	// copy the sha, since it is the 20 out
	memcpy(twenty_out,sha1,20);
}


















