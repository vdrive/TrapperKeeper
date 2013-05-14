/*-------------------------------INCLUDE---------------------------------------*/
//#include "windows.h"
#include <stdio.h>
#include "MDConsoleOutput.h"
#include <string>
#include <iostream>
/*---------------------------NAMESPACE-----------------------------------------*/
using namespace std;
/*----------------------------PREPROCESSOR-------------------------------------*/
#define WHITE_BACKGROUND BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED
#define DEFAULT_SEVERITY 0
/*-----------------------------------------------------------------------------
Function Name:MDOutput(char* outputString)
Arugments:  char* - the string we want to output

Returns:  bool - true if we were succesful
Access:   public
Purpose:  to give more flexible access to the output to the console screen.  By
          default, anything with a sevirty will have a white background, while typical
          output will have a black background.
          Also note, if #DEBUG is not defined, nothing will happen
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::MDPrintln(char* output) 
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf);
  Write(buf,DEFAULT_SEVERITY);
#endif 

  return result;
}
/*-----------------------------------------------------------------------------
Function Name:MDOutput(char*, int value)
Arugments:  char* - the string we want to output
            int - for the value we want concated to the end of our debug
Returns:  bool - true if we were succesful
Access:   public
Purpose:  to give more flexible access to the output to the console screen.  By
          default, anything with a sevirty will have a white background, while typical
          output will have a black background.
          Also note, if #DEBUG is not defined, nothing will happen
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::MDPrintln(char* output, int value)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf,value);
  Write(buf,DEFAULT_SEVERITY);
#endif 

  return result;
}
bool MDConsoleOutput::MDPrintln(char* output, DWORD value)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf,value);
  Write(buf,DEFAULT_SEVERITY);
#endif 

  return result;
}
/*-----------------------------------------------------------------------------
Function Name:MDOutput( int severity, char*)
Arugments:  char* - the string we want to output
            int - the severity of the output
          
            1 - FOREGROUND_BLUE      text color contains blue.
            2 - FOREGROUND_GREEN     text color contains green.
            3 - FOREGROUND_RED       text color contains red.
            7 - FOREGROUND_WHITE    text color white.
Returns:  bool - true if we were succesful
Access:   public
Purpose:  to give more flexible access to the output to the console screen.  By
          default, anything with a sevirty will have a white background, while typical
          output will have a black background.
          Also note, if #DEBUG is not defined, nothing will happen
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::MDPrintln(int severity,char* output)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf);
  Write(buf,severity);
#endif 

  return result;
}
/*-----------------------------------------------------------------------------
Function Name:MDOutput( int severity, char* output,int value)
Arugments:  int - the severity of the output
            1 - FOREGROUND_BLUE      text color contains blue.
            2 - FOREGROUND_GREEN     text color contains green.
            3 - FOREGROUND_RED       text color contains red.
            7 - FOREGROUND_WHITE    text color white.

            char* - the string we want to output
            int - value that we wanted concated

Returns:  bool - true if we were succesful
Access:   public
Purpose:  to give more flexible access to the output to the console screen.  By
          default, anything with a sevirty will have a white background, while typical
          output will have a black background.
          Also note, if #DEBUG is not defined, nothing will happen
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::MDPrintln(int severity,char* output, int value)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf,value);
  Write(buf,severity);
#endif//end of debug

  return result;
}
bool MDConsoleOutput::MDPrintln(int severity,char* output, DWORD value)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output,buf,value);
  Write(buf,severity);
#endif//end of debug

  return result;
}

bool MDConsoleOutput::MDPrintln(char* output1,char* output2)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output1,output2,buf );
  Write(buf,DEFAULT_SEVERITY);
#endif//end of debug

  return result;
}
/*-----------------------------------------------------------------------------
Function Name:MDOutput( int severity, char* output,char* output)
Arugments:  int - the severity of the output
            1 - FOREGROUND_BLUE      text color contains blue.
            2 - FOREGROUND_GREEN     text color contains green.
            3 - FOREGROUND_RED       text color contains red.
            7 - FOREGROUND_WHITE    text color white.

            char* - the string we want to output
            char* - the string we want to output

Returns:  bool - true if we were succesful
Access:   public
Purpose:  to give more flexible access to the output to the console screen.  By
          default, anything with a sevirty will have a white background, while typical
          output will have a black background.
          Also note, if #DEBUG is not defined, nothing will happen
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::MDPrintln(int severity,char* output1,char* output2)
{
  bool result = false;

#ifdef DEBUG
  char buf[100];
  Format(output1,output2,buf);
  Write(buf,severity);
#endif//end of debug

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Format(char* lineToFormat1, char* lineToFormat2, char* Output)
Arugments:  char* lineToFormat1  - line we want to format
            char* lineToFormat2  - line we want to format
            char* Output        - destenation string 
Returns: 
Access:  private
Purpose: to format our string
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::Format(char* lineToFormat1, char* lineToFormat2,char* Output)
{
  bool result=false;

  string toOutput1(lineToFormat1);
  string toOutput2(lineToFormat2);

  string toOutput;
  toOutput = toOutput1 + toOutput2 + "\n";
 
  strcpy(Output,toOutput.data());
  result = true;
  return result;
}

/*-----------------------------------------------------------------------------
Function Name: Format(char* lineToFormat, char* Output, int value)
Arugments:  char* lineToFormat  - line we want to format
            char* Output        - destenation string
            int value           - value that we want to concatinate
Returns: 
Access:  private
Purpose: to format our string
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::Format(char* lineToFormat, char* Output, int value)
{
  bool result=false;

  string toOutput(lineToFormat);

  char buf[10];
  _itoa( value, buf, 10 );

  string intValue(buf);
  toOutput += intValue;
  toOutput += "\n";

  strcpy(Output,toOutput.data());
  result = true;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Format(char* lineToFormat, char* Output, int value)
Arugments:  char* lineToFormat  - line we want to format
            char* Output        - destenation string
            int value           - value that we want to concatinate
Returns: 
Access:  private
Purpose: to format our string
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::Format(char* lineToFormat, char* Output, DWORD value)
{
  bool result=false;

  string toOutput(lineToFormat);

  char buf[10];
  _itoa( value, buf, 10 );

  string intValue(buf);
  toOutput += intValue;
  toOutput += "\n";

  strcpy(Output,toOutput.data());
  result = true;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Format(char* lineToFormat, char* Output)
Arugments:  char* lineToFormat  - line we want to format
            char* Output        - destenation string
Returns: 
Access:  private
Purpose: to format our string
Created: 3/17/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::Format(char* lineToFormat, char* Output)
{
  bool result=false;
  string toOutput(lineToFormat);

  toOutput += "\n";

  strcpy(Output,toOutput.data());
  result = true;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:Write(char* output, int severity)
Arugments:  char* output - string that we want to write to the console
            int severity - basically the color of the text to write
Returns:  bool - true if we are successful
Access:   private
Purpose:  to encapsulate our handle access
Created:  3/17/04
Modified:
-------------------------------------------------------------------------------*/
bool MDConsoleOutput::Write(char* output, int severity)
{
  bool result = false;

  //Handle for our output
  HANDLE hStdout;
  DWORD cWritten;
  DWORD wOldColorAttrs; 
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 

  // Get handles to STDOUT. 
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 

  if ((hStdout != INVALID_HANDLE_VALUE) && (output != NULL))
  {
    //get initial settings
    GetConsoleScreenBufferInfo(hStdout, &csbiInfo); 
    wOldColorAttrs = csbiInfo.wAttributes; 

    switch(severity)
    {
    case 1:
      {
        SetConsoleTextAttribute(hStdout,FOREGROUND_BLUE | WHITE_BACKGROUND);
      }
      break;
    case 2:
      {
        SetConsoleTextAttribute(hStdout,FOREGROUND_GREEN | WHITE_BACKGROUND);
      }
      break;
    case 3:
      {
        SetConsoleTextAttribute(hStdout,FOREGROUND_RED | WHITE_BACKGROUND);
      }
      break;
    default:
      break;      
    }//end of switch

    //Do the write
    result = WriteConsole(hStdout,output,strlen(output),&cWritten,0);
 
    //Set it back to normal
    SetConsoleTextAttribute(hStdout,csbiInfo.wAttributes);

  }//end of (hStdout != INVALID_HANDLE_VALUE)

  return result;
}