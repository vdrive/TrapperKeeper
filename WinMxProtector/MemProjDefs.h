//#include <windows.h>
//#include <Psapi.h>
//#include "stdio.h"
//#include "ProcessNameID.h"
 
#ifndef STARTADDRESS
#define STARTADDRESS 280300//STARTADDRESS 280300
#endif

#ifndef MEMORYSCANSIZE 
#define MEMORYSCANSIZE 280330-STARTADDRESS
#endif

#ifndef MAXNAMELENGTH 
#define MAXNAMELENGTH 100
#endif

#ifndef MAXNUMOFPROCS 
#define MAXNUMOFPROCS 1000
#endif