#pragma once

struct SupplyProcessorHeader
{
	enum op_code {
		Submit=1,
		Request,
		Clear
	};
	op_code op;
	unsigned int size;

};
