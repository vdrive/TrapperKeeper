// RegistryKeys.h
#pragma once
class RegistryKeys
{
public:
	static void SetDividers(int normal,int singles);
	static void GetDividers(int& normal,int& singles);
	static void SetPioletDivider(int normal);
	static void GetPioletDivider(int& normal);
	static void SetBearShareDivider(int normal);
	static void GetBearShareDivider(int& normal);
	static void SetGiftSwarmerDivider(int normal);
	static void GetGiftSwarmerDivider(int& normal);
	static void SetGiftSpooferDivider(int normal);
	static void GetGiftSpooferDivider(int& normal);
};