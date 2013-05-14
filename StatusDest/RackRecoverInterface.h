#pragma once
#define RACKRECOVERYPORT 17240
#define RACKMONITORPORT 17241
#define RACKRESTARTCOMMAND 5555
#define RACKPING 5556
#define RACKPONG 5557
#define RACKDLLS 5558
#define RACKPROCESSES 5559
#define RACKTRAPPERKEEPERALIVE 5560
#define RACKDELETEFILE 5561

class RackRecoverInterface
{
public:

	RackRecoverInterface(void);
	~RackRecoverInterface(void);
	void RestartRack(const char* ip);
	void DeleteFileOnRack(const char* ip, const char* full_file_name);
};
