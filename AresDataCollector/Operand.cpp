#include "StdAfx.h"
#include ".\operand.h"

//
//
//
Operand::Operand(void)
{
	Reset();
}

//
//
//
Operand::~Operand(void)
{
}

//
// Reset the object to its default state
//
void Operand::Reset()
{
	m_reg=reg::na;			// default is no register
	m_width=width::ex;		// default is a full dword register
	m_dereference=false;	// default is no dereference
	m_const=0;				// default is const = 0
}