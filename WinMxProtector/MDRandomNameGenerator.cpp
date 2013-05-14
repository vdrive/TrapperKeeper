/*----------------------------PREPROCESSOR-------------------------------------*/
#pragma warning(disable:4786)
/*-------------------------------INCLUDE---------------------------------------*/
#include <fstream>
#include "MDRandomNameGenerator.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include "MDConsoleOutput.h"

int g_nameToggle = 0;
/*-----------------------------------------------------------------------------
Function Name: MDRandomNameGenerator()
Arugments: 
Returns: 
Access:   Public
Purpose:  Constructor
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDRandomNameGenerator::MDRandomNameGenerator()
{
  InitData();
};
/*-----------------------------------------------------------------------------
Function Name:~MDRandomNameGenerator()
Arugments: 
Returns: 
Access:   Public
Purpose:  Destructor
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDRandomNameGenerator::~MDRandomNameGenerator(){};
/*-----------------------------------------------------------------------------
Function Name:GetRandomName(char* destName)
Arugments: 
Returns: 
Access:   Public
Purpose:  to get a random name for someone
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDRandomNameGenerator::GetRandomName(char* destName)
{
  bool result = false;
  
  //check to make sure we've read in names...else fail 
  if((m_firstNames.size() != 0 ) && (m_lastNames.size() != 0))
  {
    //get the names from our vectors
    const char* firstname = (m_firstNames.at(rand()  % m_firstNames.size()).data());
    const char* lastname = (m_lastNames.at(rand()  % m_lastNames.size()).data());
    
    //put together our new name
    strcpy(destName,firstname); //firstname
    strcat(destName," ");       //toss a space in there  
    strcat(destName,lastname);  //last name

    //every now and then make the name funky
    g_nameToggle++;
    if(g_nameToggle>100) g_nameToggle = 0;

    if((g_nameToggle % 2) == 0)
    {
      AddFunk(destName);
    }
    result = true;
  }

  return result;
};
/*-----------------------------------------------------------------------------
Function Name:AddFunk(char* destName)
Arugments: 
Returns: 
Access:   Public
Purpose:  To Spice up the name a little bit
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDRandomNameGenerator::AddFunk(char* destName)
{
  bool result = false;
  int length = (int)strlen(destName);

  for(int index=1;index<length;index++)
  {
    destName[index] = tolower(destName[index]);
  }

  int modvalues = 12;
  char Charactors[] = {'~','`','!','@','#','$','%','^','-','+','=','_'}; 

  int jumpVal = rand() % length - 3;
  if(jumpVal <= 0) jumpVal = 5;
  for(int i=0;i<length-jumpVal;i+=jumpVal)
  {
    int next = rand() % modvalues;
    if(next >= length) break;
    destName[i] = Charactors[rand() % modvalues];
  }
  result = true;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:ReadInCensusFile(char* fileName,vector<string> *names)
Arugments: 
Returns: 
Access:   Public
Purpose:  to read our data file and file up the appropriate vector
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDRandomNameGenerator::ReadInCensusFile(char* fileName,vector<string> *names)
{
  bool result = false;
 // names->empty();
  if(fileName!=NULL)
  {
    ifstream infile;
    infile.open(fileName);
    if (infile) 
    {
      for (string someName; infile >> someName; ) 
      {
        //test to see if this is a num  
        const char *ptr1 = someName.data();
        double numtest = atof(ptr1);
        if( (!(numtest > 0)) && (strcmp(ptr1,"0.000") != 0) )
        {
          names->push_back(someName);
        }
      }
      infile.close();
      result = true;
    }
    else
    {
      //MDConsoleOutput::MDPrintln("We did not find the file:",fileName);
    }
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:InitData()
Arugments:void
Returns:  bool - true if we are succesfull
Access:   Public
Purpose:  to fill up our arrays with names
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDRandomNameGenerator::InitData()
{
  //MDConsoleOutput::MDPrintln("Initing data");
  bool result = false;
  //seed our generator
  srand((int)time(0));
  m_firstNames.empty();
  m_lastNames.empty();
  //collect our data
  ReadInCensusFile("malefirstNames.txt",&m_firstNames);
  ReadInCensusFile("femalefirstNames.txt",&m_firstNames);
  ReadInCensusFile("lastNames.txt",&m_lastNames);
  result = true;
  //MDConsoleOutput::MDPrintln("Done initing data");
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:InitData()
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
bool MDRandomNameGenerator::UnitTest()
{
  bool result = false;
  vector<string> names;
  ReadInCensusFile("malefirstNames.txt",&names);
  for(int i=0;i<(int)names.size();i++)
  {
    const string d = names.at(i);
    //MDConsoleOutput::Println("",d.data());
  }
  return result;
}
 