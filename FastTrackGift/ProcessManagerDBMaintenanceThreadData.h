#pragma once

class ProcessManagerDBMaintenanceThreadData
{
public:
	ProcessManagerDBMaintenanceThreadData(void);
	~ProcessManagerDBMaintenanceThreadData(void);

	enum vals
	{
		NumberOfEvents=3
	};

	HANDLE m_events[NumberOfEvents];
};
