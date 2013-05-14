
#ifndef BENCODE_H
 #define BENCODE_H

#include <string>
#include "BtStructs.h"
//#include "unsigned short.H"


using namespace std;

string EncodeInt( const  int x );
// string EncodeLong( const  Long x );
string EncodeString( const  string x );
//string EncodeList( const  List &x );
string EncodeDicti( const  AtomDicti x );
//string Encode(   *pAtom );

// the decode functions allocate memory, so be SURE to delete it

int DecodeInt(unsigned short* buf, int *pos);
// Long *DecodeLong( const string &x, unsigned long iStart = 0 );
long DecodeLong(char * buf, int *pos);
// string DecodeString( const string &x, unsigned long iStart = 0 );
string DecodeString(unsigned short * buf,int *pos);
// List *DecodeList( const string &x, unsigned long iStart = 0 );
AtomList DecodeList(unsigned short * buf, int *pos);
// AtomDicti *DecodeDicti( const string &x, unsigned long iStart = 0 );
AtomDicti DecodeDicti( char * buf, int *pos);
//  *Decode( const string &x, unsigned long iStart = 0 );
 // *DecodeFile( const char *szFile );

vector<unsigned short*> DecodePieces(unsigned short * buf, int *pos); // custom for bit torrents

#endif
