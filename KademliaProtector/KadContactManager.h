#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "KadContact.h"
#include <afxmt.h>
#include "UInt128.h"

class KadContactManager : public ThreadedObject
{
	class SaveContactThread : public ThreadedObject
	{
		Vector mv_contacts[256];
	public:
		UINT Run();
		SaveContactThread(Vector *v_contacts);
	};

	Vector mv_contacts[256];
	Vector mv_new_contacts;
	CCriticalSection m_lock;
	char* m_status;

	int m_main_boot_strap_index;
	int m_second_boot_strap_index;

	SaveContactThread *mp_save_contact_thread;

public:
	KadContactManager(void);
	~KadContactManager(void);
	void AddContacts(Vector &v_contacts);
	
private:
	void Update(void);
	void SaveContactList(void);
public:
	void StartUp(void);
	void Shutdown(void);
	void GetNextBootStrapContact(Vector &v_tmp);
	UINT GetContactCount(void);
	UINT GetNewCachedContactCount(void);
	void GetContactsForKey(CUInt128& key, Vector& v_contacts);
	char* GetContactLoadStatus(void);
protected:
	UINT Run(void);
};
