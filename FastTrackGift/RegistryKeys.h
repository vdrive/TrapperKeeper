// RegistryKeys.h
#pragma once
class RegistryKeys
{
public:
	static void SetModuleCounts(int min,int max);
	static void GetModuleCounts(int *min,int *max);
	static void SetSupplyIntervalMultiplier(int multi);
	static void GetSupplyIntervalMultiplier(int *multi);
};