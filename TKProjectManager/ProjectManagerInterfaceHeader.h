#pragma once

struct ProjectManagerInterfaceHeader
{
	enum op_code
	{
		Init = 1,
		HaveProjectsChanged,
		IsReady,
		GetProjectsCopy
	};
	op_code op;
	unsigned int size;
};