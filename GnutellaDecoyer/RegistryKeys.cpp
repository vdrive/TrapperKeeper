// RegistryKeys.cpp

#include "stdafx.h"
#include "RegistryKeys.h"

//
//
//
void RegistryKeys::SetModuleCounts(int min,int max)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GnutellaDecoyer\\Module Count Limits";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Minimum",0,REG_DWORD,(unsigned char *)&min,cbData);
	RegSetValueEx(hKey,"Maximum",0,REG_DWORD,(unsigned char *)&max,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void RegistryKeys::GetModuleCounts(int *min,int *max)
{
	// If this is the first time running, or the key doesn't exist, return default values
	*min=10;
	*max=10;
	return;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\GnutellaDecoyer\\Module Count Limits";

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
			if(strcmp(szName,"Minimum")==0)
			{
				*min=val;
			}
			else if(strcmp(szName,"Maximum")==0)
			{
				*max=val;
			}

			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);
}
