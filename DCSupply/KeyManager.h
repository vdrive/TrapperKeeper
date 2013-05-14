#pragma once

class KeyManager
{
public:
	KeyManager(void);
	~KeyManager(void);
	CString LocktoKey(CString lock);	//converts the lock into an acceptable key
	char* CreateLock();	//creates a lock to start data transfers
	char* CreatePK();
};
