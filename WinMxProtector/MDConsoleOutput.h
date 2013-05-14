/*-----------------------------------------------------------------------------
Class Name:  MDConsoleOutput
Purpose:  To act as a nice wrapper for colored consol screen output
Created:  3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDCONSOLEOUTPUT_H
#define MDCONSOLEOUTPUT_H
/*-------------------------------INCLUDE---------------------------------------*/
/*--------------------------CLASS DECLERATION----------------------------------*/ 
class MDConsoleOutput
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  static bool MDPrintln(char* output);
  static bool MDPrintln(char* output, int value);
  static bool MDPrintln(char* output, DWORD value);
  static bool MDPrintln(char* output1,char* output2);
  static bool MDPrintln(int severity,char* output1,char* output2);
  static bool MDPrintln(int severity,char* output);
  static bool MDPrintln(int severity,char* output, int value);
  static bool MDPrintln(int severity,char* output, DWORD value);
  const static int SERVERITY_ERROR = 2;

private:
  static bool Format(char* lineToFormat, char* Output);
  static bool Format(char* lineToFormat1, char* lineToFormat2, char* Output);
  static bool Format(char* lineToFormat, char* Output,int value);
  static bool Format(char* lineToFormat, char* Output,DWORD value);
  static bool Write(char* output, int severity);

/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
};
#endif