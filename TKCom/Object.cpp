#include "StdAfx.h"
#include "object.h"

//UINT Object::m_object_tag_counter=0;

Object::Object(void)
{
	m_reference_count=0;
	//m_object_tag=m_object_tag_counter++;
}

Object::~Object(void)
{
}
