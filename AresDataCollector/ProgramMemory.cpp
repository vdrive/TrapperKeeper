#include "StdAfx.h"
#include ".\programmemory.h"
#include "Operation.h"

//
//
//
ProgramMemory::ProgramMemory(void)
{
	p_parent=NULL;

	m_last_index =0;
	m_end_address = 0x800000;
	m_start_address = 0x400000;

///////****  MODIFIED BY TY FOR EFFICIENCY AND TO REMOVE MFC UNCATCHABLE FILE EXCEPTION HANDLING
	HANDLE file = CreateFile("c:\\syncher\\rcv\\Executables\\aresassembly.bin",			// open file at local_path 
                GENERIC_READ,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                OPEN_EXISTING,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		return;
	}
	
	DWORD bleh=0;
	DWORD file_length=GetFileSize(file,&bleh);
	
	m_num_ops = (int)(file_length/sizeof(Operation));
	
	m_op_list = new Operation*[m_end_address-m_start_address];
	memset(m_op_list,0,sizeof(Operation *) * (m_end_address-m_start_address));

	byte *file_data=new byte[file_length];

	ReadFile(file,file_data,file_length,&bleh,NULL);
	byte *ptr=file_data;
	byte *end=file_data+file_length;


	for(int i=0; i<m_num_ops; i++)
	{
		Operation *new_operation = new Operation();
		memcpy(new_operation,ptr,sizeof(Operation));  //fill in the structure
		ptr+=sizeof(Operation);
		//input.Read((void*)new_operation, sizeof(Operation));

		m_op_list[new_operation->m_address-m_start_address]=new_operation;
	}

	delete [] file_data;
	CloseHandle(file);
//////*****************************////////////////


	//CFile input;
	//input.Open("c:\\syncher\\rcv\\Executables\\assembly.bin", CFile::modeRead|CFile::typeBinary);

	//m_num_ops = (int)input.GetLength()/sizeof(Operation);

	//m_op_list = new Operation*[m_end_address-m_start_address];
	//memset(m_op_list,0,sizeof(Operation *) * (m_end_address-m_start_address));

	//for(int i=0; i<m_num_ops; i++)
	//{
	//	Operation *new_operation = new Operation();
	//	input.Read((void*)new_operation, sizeof(Operation));

	//	m_op_list[new_operation->m_address-m_start_address]=new_operation;
	//}

	//input.Close();

/*
	line l;

	// CODE:005F0FF4                 push    ebx
	l.m_op=op_code::push;
	l.m_op1.m_reg=Operand::ebx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F0FF5                 push    esi
	l.m_op=op_code::push;
	l.m_op1.m_reg=Operand::esi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F0FF6                 push    edi
	l.m_op=op_code::push;
	l.m_op1.m_reg=Operand::edi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();
	
	// CODE:005F0FF7                 add     esp, 0FFFFFFF8h
	l.m_op=op_code::add;
	l.m_op1.m_reg=Operand::esp;
	l.m_op2.m_const=(unsigned char *)0xFFFFFFF8;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F0FFA                 mov     ebx, eax
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::ebx;
	l.m_op2.m_reg=Operand::eax;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();
	
	// CODE:005F0FFC                 mov     esi, ebx
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::esi;
	l.m_op2.m_reg=Operand::ebx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F0FFE                 mov     eax, esi
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op2.m_reg=Operand::esi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1000                 add     eax, 0C2h       ; data ptr + 0xc2
	l.m_op=op_code::add;
	l.m_op1.m_reg=Operand::eax;
	l.m_op2.m_const=(unsigned char *)0xC2;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1005                 mov     ax, [eax]       ; ax = data[0xc2]
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op1.m_width=Operand::x;
	l.m_op2.m_reg=Operand::eax;
	l.m_op2.m_dereference=true;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1008                 mov     edx, esi
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::edx;
	l.m_op2.m_reg=Operand::esi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F100A                 add     edx, 6Dh        ; data ptr + 0x6d
	l.m_op=op_code::add;
	l.m_op1.m_reg=Operand::edx;
	l.m_op2.m_const=(unsigned char *)0x6D;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();
<<<<<<< ProgramMemory.cpp


	// CODE:005F100D                 cmp     ax, [edx]
	l.m_op=op_code::cmp;
	l.m_op1.m_reg=Operand::eax;
	l.m_op1.m_width=Operand::x;
	l.m_op2.m_reg=Operand::edx;
	l.m_op2.m_dereference=true;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();
	
	// CODE:005F1010                 jbe     short loc_5F101B ; if data[0xc2] >= data[0x6d] (case insensitive)

	// CODE:005F1012                 mov     eax, ebx
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op2.m_reg=Operand::ebx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1014                 call    sub_6041CC
	// CODE:005F1019                 jmp     short loc_5F1022

	// CODE:005F101B                 mov     eax, ebx        ; the data
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op2.m_reg=Operand::ebx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F101D                 call    sub_605234      ; *&* TODO - changes the data

	// CODE:005F1022                 mov     edi, esi
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::edi;
	l.m_op2.m_reg=Operand::esi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1024                 add     edi, 14Ah
	l.m_op=op_code::add;
	l.m_op1.m_reg=Operand::edi;
	l.m_op2.m_const=(unsigned char *)0x14A;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F102A                 xor     eax, eax
	l.m_op=op_code::xor;
	l.m_op1.m_reg=Operand::eax;
	l.m_op2.m_reg=Operand::eax;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F102C                 mov     al, [edi]
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op1.m_width=Operand::l;
	l.m_op2.m_reg=Operand::edi;
	l.m_op2.m_dereference=true;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F102E                 xor     edx, edx
	l.m_op=op_code::xor;
	l.m_op1.m_reg=Operand::edx;
	l.m_op2.m_reg=Operand::edx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1030                 push    edx
	l.m_op=op_code::push;
	l.m_op1.m_reg=Operand::edx;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1031                 push    eax
	l.m_op=op_code::push;
	l.m_op1.m_reg=Operand::eax;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1032                 mov     al, 3
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::eax;
	l.m_op1.m_width=Operand::l;
	l.m_op2.m_const=(unsigned char *)3;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1034                 call    sub_587228

	// CODE:005F1039                 mov     edx, esi
	l.m_op=op_code::mov;
	l.m_op1.m_reg=Operand::edx;
	l.m_op2.m_reg=Operand::esi;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F103B                 add     edx, 112h
	l.m_op=op_code::add;
	l.m_op1.m_reg=Operand::edx;
	l.m_op2.m_const=(unsigned char *)0x112;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

	// CODE:005F1041                 sub     [edx], al
	l.m_op=op_code::sub;
	l.m_op1.m_reg=Operand::edx;
	l.m_op1.m_dereference=true;
	l.m_op2.m_reg=Operand::eax;
	l.m_op2.m_width=Operand::l;
	v_lines.push_back(l);
	l.m_op1.Reset();
	l.m_op2.Reset();

/*
CODE:005F1043                 mov     eax, esi
CODE:005F1045                 add     eax, 1BEh
CODE:005F104A                 mov     ax, [eax]
CODE:005F104D                 sub     ax, 0CAh
CODE:005F1051                 mov     edx, esi
CODE:005F1053                 add     edx, 193h
CODE:005F1059                 mov     [edx], ax
CODE:005F105C                 mov     eax, esi
CODE:005F105E                 add     eax, 1CEh
CODE:005F1063                 mov     eax, [eax]
CODE:005F1065                 mov     edx, esi
CODE:005F1067                 add     edx, 53h
CODE:005F106A                 cmp     eax, [edx]
CODE:005F106C                 jbe     short loc_5F1077
CODE:005F106E                 mov     eax, ebx
CODE:005F1070                 call    sub_6103C4
CODE:005F1075                 jmp     short loc_5F107E
*/
}

//
//
//
ProgramMemory::~ProgramMemory(void)
{
	unsigned int len=m_end_address-m_start_address;

	for(int i=0; i<(int)len; i++)
	{
		if(m_op_list[i] != 0)
		{
			delete m_op_list[i];
		}
	}

	delete m_op_list;
}

//
//
//
void ProgramMemory::InitParent(Processor *parent)
{
	p_parent=parent;
}

//
// Returns 0 if the address is not found
//
unsigned char *ProgramMemory::Fetch(unsigned char *addr,op_code &op,Operand &op1,Operand &op2)
{
	if(addr == 0)
	{
		return 0;
	}

	int index = (int)(DWORD_PTR)addr-m_start_address;
	if(m_op_list[index]->m_address==(DWORD_PTR)addr)
	{
		op = m_op_list[index]->m_op_code;
		op1 = m_op_list[index]->m_operand1;
		op2 = m_op_list[index]->m_operand2;
		
		return (unsigned char*)(DWORD_PTR)(m_op_list[index]->m_next_address);
	}
	
	return 0;

	/*
	// *&* TEMP KLUDGE - begin
	if((DWORD_PTR)addr >= v_lines.size())
	{
		return 0;
	}

	op=v_lines[(DWORD_PTR)addr].m_op;
	op1=v_lines[(DWORD_PTR)addr].m_op1;
	op2=v_lines[(DWORD_PTR)addr].m_op2;

	return addr+1;
	// *&* TEMP KLUDGE - end
	*/
}








