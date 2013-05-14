/*-----------------------------------------------------------------------------
Class Name:  MDRandomNameGenerator
Purpose:  The purpose of this class is  to Generator a Random name
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDRANDOMNAMEGENERATOR_H
#define MDRANDOMNAMEGENERATOR_H
/*-------------------------------INCLUDE---------------------------------------*/
#include <string>
//#include <windows.h>
#include <vector>
using namespace std;
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDRandomNameGenerator
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDRandomNameGenerator();
  ~MDRandomNameGenerator();
  bool GetRandomName(char* destName);
  bool UnitTest();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
private:
  bool ReadInCensusFile(char* fileName,vector<string> *names);
  bool InitData();
  bool AddFunk(char* destName);
  vector<string> m_firstNames;
  vector<string> m_lastNames;

/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
};
#endif