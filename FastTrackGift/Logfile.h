// Logfile.h
#include <string>
using namespace std;

class Logfile : public CStdioFile
{
public:
/*	
	enum logifle_type
	{
		LogfileTypeSpoof,
		LogfileTypeSupply,
		LogfileTypeDebug,
		LogfileTypeDist
	};
*/
	Logfile(/*int type,char *title,*/char *self);
	void Log(char *msg);

private:
	char m_filename[MAX_PATH+1];
	char m_path[MAX_PATH+1];
};
