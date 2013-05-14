// RegistryKeys.cpp

#include "stdafx.h"
#include "RegistryKeys.h"

//
//
//
void RegistryKeys::SetDividers(int normal,int singles)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GnutellaSpoofer\\Load Balancing Divider";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Normal",0,REG_DWORD,(unsigned char *)&normal,cbData);
	RegSetValueEx(hKey,"Singles",0,REG_DWORD,(unsigned char *)&singles,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetDividers(int& normal,int& singles)
{
	// If this is the first time running, or the key doesn't exist, return default values
	normal=4;
	singles=2;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GnutellaSpoofer\\Load Balancing Divider";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Normal")==0)
			{
				normal=val;
			}
			else if(strcmp(szName,"Singles")==0)
			{
				singles=val;
			}

			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}

void RegistryKeys::SetPioletDivider(int normal)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletSpoofer\\Load Balancing Divider";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Normal",0,REG_DWORD,(unsigned char *)&normal,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetPioletDivider(int& normal)
{
	// If this is the first time running, or the key doesn't exist, return default values
	normal=4;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletSpoofer\\Load Balancing Divider";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Normal")==0)
			{
				normal=val;
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}

void RegistryKeys::SetBearShareDivider(int normal)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\BearShare\\Load Balancing Divider";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Normal",0,REG_DWORD,(unsigned char *)&normal,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetBearShareDivider(int& normal)
{
	// If this is the first time running, or the key doesn't exist, return default values
	normal=3;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\BearShare\\Load Balancing Divider";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Normal")==0)
			{
				normal=val;
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}

void RegistryKeys::SetGiftSwarmerDivider(int normal)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GiftSwarmer\\Load Balancing Divider";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Normal",0,REG_DWORD,(unsigned char *)&normal,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetGiftSwarmerDivider(int& normal)
{
	// If this is the first time running, or the key doesn't exist, return default values
	normal=3;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GiftSwarmer\\Load Balancing Divider";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Normal")==0)
			{
				normal=val;
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}

void RegistryKeys::SetGiftSpooferDivider(int normal)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GiftSpoofer\\Load Balancing Divider";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Normal",0,REG_DWORD,(unsigned char *)&normal,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetGiftSpooferDivider(int& normal)
{
	// If this is the first time running, or the key doesn't exist, return default values
	normal=3;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GiftSpoofer\\Load Balancing Divider";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Normal")==0)
			{
				normal=val;
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}