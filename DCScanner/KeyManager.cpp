#include "StdAfx.h"
#include "keymanager.h"

KeyManager::KeyManager(void)
{
}

KeyManager::~KeyManager(void)
{
}

char* KeyManager::CreateLock()
{
	char lock[80];
	char *ptr_lock;	//lock buffer
	ptr_lock=&lock[0];
	CString finalkey;
	int random_number;
	for(int i = 0; i < 80; i++)
	{
		random_number=rand();
		lock[i] = (char)(94.0 * random_number + 33);
	}
/*for (i=0; i<(int)strlen(lock); i++)
	{
		switch (lock[i])
		{
			case 0:
			{
				finalkey+="/%DCN000%/";
				break;
			}
			case 5:
			{
				finalkey+="/%DCN005%/";
				break;	 
			}
			case 36:
			{
				finalkey+="/%DCN036%/";
				break;
			}
			case 96:
			{
				finalkey+="/%DCN096%/";
				break;
			}
			case 124:
			{
				finalkey+="/%DCN124%/";
				break;
			}
			case 126:
			{
				finalkey+="/%DCN126%/";
				break;
			}
			default:
			{
				finalkey+=lock[i];
				break;
			}
		}
	}*/
	//finalkey=lock;
	//return finalkey;
	return ptr_lock;
}

char* KeyManager::CreatePK()
{
	char pk[16];	//pk buffer
	char *ptr_pk;
	ptr_pk=&pk[0];
	CString finalkey;
	int random_number;
	for(int i = 0; i < 16; i++)
	{
		random_number=rand();
		pk[i] = (char)(94.0 * random_number + 33);
	}
	/*for (i=0; i<(int)strlen(pk); i++)
	{
		switch (pk[i])
		{
			case 0:
			{
				finalkey+="/%DCN000%/";
				break;
			}
			case 5:
			{
				finalkey+="/%DCN005%/";
				break;	 
			}
			case 36:
			{
				finalkey+="/%DCN036%/";
				break;
			}
			case 96:
			{
				finalkey+="/%DCN096%/";
				break;
			}
			case 124:
			{
				finalkey+="/%DCN124%/";
				break;
			}
			case 126:
			{
				finalkey+="/%DCN126%/";
				break;
			}
			default:
			{
				finalkey+=pk[i];
				break;
			}
		}
	}*/
	return ptr_pk;
}

CString KeyManager::LocktoKey(CString lock1)
{
	char * lock=lock1.GetBuffer(lock1.GetLength());
	int len = lock1.GetLength();//(int)strlen(lock);
	char * key = new char[len];

	// Special case char 0
	key[0] = lock[0]^lock[len-1]^lock[len-2]^5;

	// Xor the current position with the previous position
	for (int i=1; i<len; i++)
	{
		key[i] = lock[i]^lock[i-1];
	}

	// Nibble Swap
	for (i = 0; i<len; i++)
	{
		key[i] = ((key[i]<<4) & 240) | ((key[i]>>4) & 15);
	}

	CString finalkey;
	
	// Make the final key by substituting the 0, 5, 36, 96, 124, and 126 characters
	for (i=0; i<len; i++)
	{
		switch (key[i])
		{
			case 0:
			{
				finalkey+="/%DCN000%/";
				break;
			}
			case 5:
			{
				finalkey+="/%DCN005%/";
				break;	 
			}
			case 36:
			{
				finalkey+="/%DCN036%/";
				break;
			}
			case 96:
			{
				finalkey+="/%DCN096%/";
				break;
			}
			case 124:
			{
				finalkey+="/%DCN124%/";
				break;
			}
			case 126:
			{
				finalkey+="/%DCN126%/";
				break;
			}
			default:
			{
				finalkey+=key[i];
				break;
			}
		}
	}
	delete [] key;
	return(finalkey);
}


