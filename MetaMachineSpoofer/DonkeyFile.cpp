#include "StdAfx.h"
#include "donkeyfile.h"
#include <io.h>
#include "HashPart.h"
#include "SpoofingSystem.h"

// basic MD4 functions
#define MD4_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD4_G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

// rotates x left n bits
// 15-April-2003 Sony: use MSVC intrinsic to save a few cycles
#ifdef _MSC_VER
#pragma intrinsic(_rotl)
#define MD4_ROTATE_LEFT(x, n) _rotl((x), (n))
#else
#define MD4_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

// partial transformations
#define MD4_FF(a, b, c, d, x, s) \
{ \
  (a) += MD4_F((b), (c), (d)) + (x); \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_GG(a, b, c, d, x, s) \
{ \
  (a) += MD4_G((b), (c), (d)) + (x) + (UINT)0x5A827999; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_HH(a, b, c, d, x, s) \
{ \
  (a) += MD4_H((b), (c), (d)) + (x) + (UINT)0x6ED9EBA1; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

// constants for MD4Transform
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

//spoof
DonkeyFile::DonkeyFile(const char* file_name,byte* hash,int size)
{
	m_file_name=file_name;
	for(int i=0;i<16;i++){
		m_hash[i]=hash[i];	
	}
	m_shash=EncodeBase16(hash,16);
	m_size=size;
	mb_completable=true;
	mb_evil_file=false;
}

//swarm
DonkeyFile::DonkeyFile(const char* file_name,byte* hash,int size,byte* hashparts)
{
	m_file_name=file_name;
	for(int i=0;i<16;i++){
		m_hash[i]=hash[i];	
	}
	m_shash=EncodeBase16(hash,16);
	m_size=size;

	int num_parts=size/PARTSIZE;
	if(num_parts>0){
		num_parts++;
	}

	if(num_parts>0 && hashparts!=NULL){
		for(int i=0;i<num_parts;i++){
			mv_hash_set.Add(new HashPart((byte*)hashparts+i*16));
		}
	}
	mb_completable=true;
	mb_evil_file=false;
}

//real decoy
DonkeyFile::DonkeyFile(LPCTSTR in_directory, LPCTSTR in_filename)
{
	m_file_path=in_directory;
	m_file_name=in_filename;
	CreateFromDecoy(in_directory,in_filename);
	m_shash=EncodeBase16(m_hash,16);
	mb_completable=true;
	mb_evil_file=false;
}

//false decoy
DonkeyFile::DonkeyFile(LPCTSTR in_filename, UINT length,byte* signature,bool b_possibly_evil,bool b_force_evil)
{
	m_file_name=in_filename;
	memcpy(m_signature,signature,SIGNATURELENGTH);
	m_size=length;

	if(m_size<100000000){  //for small files, disable evil file capability
		b_possibly_evil=false;
		b_force_evil=false;
	}

	if(signature[0]&1){
		mb_completable=false;
	}
	else{
		mb_completable=true;
	}

	if(b_force_evil || (b_possibly_evil && (signature[1]&1) && m_size>100000000)){  //if is big
		mb_evil_file=true;
		mb_completable=true;  //evil files must be completable
	}
	else{  //make sure the evil flag is turned off
		mb_evil_file=false;
	}

	if(mb_evil_file)
		CreateEvilFileFromSignature();
	else
		CreateFromSignature();

	m_shash=EncodeBase16(m_hash,16);	
}

DonkeyFile::~DonkeyFile(void)
{
}

bool DonkeyFile::CreateFromDecoy(LPCTSTR in_directory, LPCTSTR in_filename)
{
//	SetPath(in_directory);
//	SetFileName(in_filename);

	TRACE("DonkeyFile::CreateFromDecoy(%s, %s)\n",in_directory,in_filename);
	// open file
	CString namebuffer;
	namebuffer.Format("%s\\%s", in_directory, in_filename);
	FILE* file = fopen(namebuffer, "rbS");
	if (!file){
		//theApp.emuledlg->AddLogLine(false, GetResString(IDS_ERR_FILEOPEN) + CString(_T(" - %s")), namebuffer, _T(""), strerror(errno));
		return false;
	}

	// set filesize
	if (_filelengthi64(file->_file)>=4294967296){
		fclose(file);
		return false; // not supported by network
	}
	m_size=(int)_filelength(file->_file);

	// we are reading the file data later in 8K blocks, adjust the internal file stream buffer accordingly
	setvbuf(file, NULL, _IOFBF, 1024*8*2);

	// create hashset
	UINT togo = m_size;
	for (UINT hashcount = 0; togo >= PARTSIZE; ) {
		byte* newhash = new byte[16];
		CreateHashFromInput(file,0,PARTSIZE,newhash,0);
		// SLUGFILLER: SafeHash
		TRACE("DonkeyFile::CreateFromDecoy(%s, %s) Adding Hash Part.\n",in_directory,in_filename);
		mv_hash_set.Add(new HashPart((byte*)newhash));
		delete []newhash;
		togo -= PARTSIZE;
		hashcount++;
		Sleep(50);
	}
	byte* lasthash = new byte[16];
	//md4clr(lasthash);
	CreateHashFromInput(file,0,togo,lasthash,0);
	if (!hashcount){
		TRACE("DonkeyFile::CreateFromDecoy(%s, %s) Has No Hash Parts.\n",in_directory,in_filename);
		md4cpy(m_hash, lasthash);
	} 
	else {
		TRACE("DonkeyFile::CreateFromDecoy(%s, %s) Adding Last Hash Part.\n",in_directory,in_filename);
		mv_hash_set.Add(new HashPart((byte*)lasthash));
		byte* buffer = new byte[mv_hash_set.Size()*16];
		for (int i = 0; i < (int)mv_hash_set.Size(); i++){
			md4cpy(buffer+(i*16), ((HashPart*)mv_hash_set.Get(i))->m_hash);
		}
		CreateHashFromInput(0,0,mv_hash_set.Size()*16,m_hash,buffer);
		//CreateHashFromString(buffer, hashlist.GetCount()*16, m_abyFileHash);
		delete[] buffer;
	}

	fclose(file);
	file = NULL;
	delete []lasthash;
	return true;	
}

//from emule
void DonkeyFile::MD4Transform(UINT Hash[4], UINT x[16])
{
  UINT a = Hash[0];
  UINT b = Hash[1];
  UINT c = Hash[2];
  UINT d = Hash[3];

  /* Round 1 */
  MD4_FF(a, b, c, d, x[ 0], S11); // 01
  MD4_FF(d, a, b, c, x[ 1], S12); // 02
  MD4_FF(c, d, a, b, x[ 2], S13); // 03
  MD4_FF(b, c, d, a, x[ 3], S14); // 04
  MD4_FF(a, b, c, d, x[ 4], S11); // 05
  MD4_FF(d, a, b, c, x[ 5], S12); // 06
  MD4_FF(c, d, a, b, x[ 6], S13); // 07
  MD4_FF(b, c, d, a, x[ 7], S14); // 08
  MD4_FF(a, b, c, d, x[ 8], S11); // 09
  MD4_FF(d, a, b, c, x[ 9], S12); // 10
  MD4_FF(c, d, a, b, x[10], S13); // 11
  MD4_FF(b, c, d, a, x[11], S14); // 12
  MD4_FF(a, b, c, d, x[12], S11); // 13
  MD4_FF(d, a, b, c, x[13], S12); // 14
  MD4_FF(c, d, a, b, x[14], S13); // 15
  MD4_FF(b, c, d, a, x[15], S14); // 16

  /* Round 2 */
  MD4_GG(a, b, c, d, x[ 0], S21); // 17
  MD4_GG(d, a, b, c, x[ 4], S22); // 18
  MD4_GG(c, d, a, b, x[ 8], S23); // 19
  MD4_GG(b, c, d, a, x[12], S24); // 20
  MD4_GG(a, b, c, d, x[ 1], S21); // 21
  MD4_GG(d, a, b, c, x[ 5], S22); // 22
  MD4_GG(c, d, a, b, x[ 9], S23); // 23
  MD4_GG(b, c, d, a, x[13], S24); // 24
  MD4_GG(a, b, c, d, x[ 2], S21); // 25
  MD4_GG(d, a, b, c, x[ 6], S22); // 26
  MD4_GG(c, d, a, b, x[10], S23); // 27
  MD4_GG(b, c, d, a, x[14], S24); // 28
  MD4_GG(a, b, c, d, x[ 3], S21); // 29
  MD4_GG(d, a, b, c, x[ 7], S22); // 30
  MD4_GG(c, d, a, b, x[11], S23); // 31
  MD4_GG(b, c, d, a, x[15], S24); // 32

  /* Round 3 */
  MD4_HH(a, b, c, d, x[ 0], S31); // 33
  MD4_HH(d, a, b, c, x[ 8], S32); // 34
  MD4_HH(c, d, a, b, x[ 4], S33); // 35
  MD4_HH(b, c, d, a, x[12], S34); // 36
  MD4_HH(a, b, c, d, x[ 2], S31); // 37
  MD4_HH(d, a, b, c, x[10], S32); // 38
  MD4_HH(c, d, a, b, x[ 6], S33); // 39
  MD4_HH(b, c, d, a, x[14], S34); // 40
  MD4_HH(a, b, c, d, x[ 1], S31); // 41
  MD4_HH(d, a, b, c, x[ 9], S32); // 42
  MD4_HH(c, d, a, b, x[ 5], S33); // 43
  MD4_HH(b, c, d, a, x[13], S34); // 44
  MD4_HH(a, b, c, d, x[ 3], S31); // 45
  MD4_HH(d, a, b, c, x[11], S32); // 46
  MD4_HH(c, d, a, b, x[ 7], S33); // 47
  MD4_HH(b, c, d, a, x[15], S34); // 48

  Hash[0] += a;
  Hash[1] += b;
  Hash[2] += c;
  Hash[3] += d;
}


//from emule
void DonkeyFile::CreateHashFromInput(FILE* file,CFile* file2, int Length, byte* Output, byte* in_string) { 
	// time critial
	md4clr(Output);
	bool PaddingStarted = false;
	UINT Hash[4];
	Hash[0] = 0x67452301;
	Hash[1] = 0xEFCDAB89;
	Hash[2] = 0x98BADCFE;
	Hash[3] = 0x10325476;
	CFile* data = 0;
	if (in_string)
		data = new CMemFile(in_string,Length);
	UINT Required = Length;
	byte   X[64*128];  
	while (Required >= 64){
        UINT len = Required / 64; 
        if (len > sizeof(X)/(64 * sizeof(X[0]))) 
             len = sizeof(X)/(64 * sizeof(X[0])); 
		if (in_string)
			data->Read(&X,len*64);
		else if (file)
            fread(&X,len*64,1,file); 
		else if (file2)
			file2->Read(&X,len*64);
		for (UINT i = 0; i < len; i++) 
        { 
           MD4Transform(Hash, (UINT*)(X + i*64)); 
        }
		Required -= len*64;
	}
	// bytes to read
	Required = Length % 64;
	if (Required != 0){
		if (in_string)
			data->Read(&X,Required);
		else if (file)
			fread(&X,Required,1,file);
		else if (file2)
			file2->Read(&X,Required);
	}
	// in byte scale 512 = 64, 448 = 56
	if (Required >= 56){
		X[Required] = 0x80;
		PaddingStarted = TRUE;
		memset(&X[Required + 1], 0, 63 - Required);
		MD4Transform(Hash, (UINT*)X);
		Required = 0;
	}
	if (!PaddingStarted)
		X[Required++] = 0x80;
	memset(&X[Required], 0, 64 - Required);
	// add size (convert to bits)
	UINT Length2 = Length >> 29;
	Length <<= 3;
	memcpy(&X[56], &Length, 4);
	memcpy(&X[60], &Length2, 4);
	MD4Transform(Hash, (UINT*)X);
	md4cpy(Output, Hash);
	if(data!=NULL){
		delete data;
	}
	//safe_delete(data);
}



void DonkeyFile::WriteFileStatus(Buffer2000 &packet,bool b_false_decoy)
{
	int num_parts=m_size/PARTSIZE;
	if(num_parts==0){
		packet.WriteWord((WORD)(0));  //write in how many parts for this status we are considering
	}
	else
		packet.WriteWord((WORD)(num_parts+1));  //write in how many parts for this status we are considering
	//TRACE("DonkeyFile::WriteFileStatus() Writing file_status out %d parts for file %s\n",num_parts+1,m_file_name.c_str());
	for(int i=0;i<=num_parts/8;i++){
		if(m_size>PARTSIZE && !mb_completable && i==0){
			packet.WriteByte(0xfe); //we are missing the first part (must encode backwards 11111110)
		}
		else{
			packet.WriteByte(0xff); //we always have the whole file so we don't need to worry about toggling bits
		}
	}
}

void DonkeyFile::WriteHashSet(Buffer2000 & packet)
{
	int num_parts=mv_hash_set.Size();
	packet.WriteWord((WORD)(num_parts));  //write in how many parts for this status we are considering
	TRACE("DonkeyFile::WriteHashSet() Writing hash_set out %d parts for file %s\n",num_parts+1,m_file_name.c_str());
	for(int i=0;i<num_parts ;i++){
		HashPart* hp=(HashPart*)mv_hash_set.Get(i);
		packet.WriteBytes(hp->m_hash,16);
	}
}

void DonkeyFile::SetProject(const char* project)
{
	m_project=project;
}

void DonkeyFile::CreateFromSignature()
{
//	SetPath(in_directory);
//	SetFileName(in_filename);

	TRACE("DonkeyFile::CreateFromSignature() file %s\n",m_file_name.c_str());
	// open file

	// create hashset
	UINT togo = m_size;

	byte *a_part=new byte[PARTSIZE];
	PredictData(a_part,0,min(m_size,PARTSIZE));
	//fill part buffer with signature
	//for(int i=0;i<PARTSIZE;i+=SIGNATURELENGTH){
	//	memcpy(a_part+i,m_signature,SIGNATURELENGTH);
	//}

	
	byte newhash[16];
	if(togo>=PARTSIZE){
		CreateHashFromInput(0,0,PARTSIZE,newhash,a_part);
	}

	for (UINT hashcount = 0; togo >= PARTSIZE; ) {
		//byte newhash[16];
		//CreateHashFromInput(0,0,PARTSIZE,newhash,a_part);
		// SLUGFILLER: SafeHash
		TRACE("DonkeyFile::CreateFromSignature() file '%s' Adding Hash Part.\n",m_file_name.c_str());
		mv_hash_set.Add(new HashPart((byte*)newhash));
		togo -= PARTSIZE;
		hashcount++;
		Sleep(5);
	}
	byte lasthash[16];
	md4clr(lasthash);
	CreateHashFromInput(0,0,togo,lasthash,a_part);

	delete [] a_part;  //this 10 meg byte buffer is no longer required.

	if (!hashcount){
		TRACE("DonkeyFile::CreateFromSignature() %s Has No Hash Parts.\n",m_file_name.c_str());
		md4cpy(m_hash, lasthash);
	} 
	else {
		TRACE("DonkeyFile::CreateFromSignature() %s Adding Last Hash Part.\n",m_file_name.c_str());
		mv_hash_set.Add(new HashPart((byte*)lasthash));
		byte* buffer = new byte[mv_hash_set.Size()*16];
		for (int i = 0; i < (int)mv_hash_set.Size(); i++){
			md4cpy(buffer+(i*16), ((HashPart*)mv_hash_set.Get(i))->m_hash);
		}
		CreateHashFromInput(0,0,mv_hash_set.Size()*16,m_hash,buffer);
		//CreateHashFromString(buffer, hashlist.GetCount()*16, m_abyFileHash);
		delete[] buffer;
	}
}

void DonkeyFile::PredictData(byte* buffer, UINT start_offset, UINT length)
{
	if(start_offset+length>m_size){
		ASSERT(0);
		return;
	}
	if(!mb_evil_file){
		if(m_size<9000000){
			//use old very compressible system for small files
			UINT sig_offset=start_offset%SIGNATURELENGTH;
			UINT init_copy_len=min((SIGNATURELENGTH-sig_offset),length);
			memcpy(buffer,m_signature+sig_offset,init_copy_len);
			start_offset+=init_copy_len;
			length-=init_copy_len;
			UINT cur_buf_pos=init_copy_len;
			while(length>0){
				memcpy(buffer+cur_buf_pos,m_signature,min(SIGNATURELENGTH,length));
				cur_buf_pos+=SIGNATURELENGTH;
				length-=min(SIGNATURELENGTH,length);
			}
		}
		else{
			//switch to non compressible for large files
			UINT cur_buf_pos=0;
			if(start_offset<SIGNATURELENGTH){  //start of file has signature, rest of file has garbage
				UINT sig_offset=start_offset%SIGNATURELENGTH;
				UINT init_copy_len=min((SIGNATURELENGTH-sig_offset),length);
				memcpy(buffer,m_signature+sig_offset,init_copy_len);
				start_offset+=init_copy_len;
				length-=init_copy_len;
				cur_buf_pos+=init_copy_len;  //advance the buffer position to account for how much data we've already predicted
			}
			if(length>0){
				MetaSpooferReference ref;
				Buffer2000 *garbage_data=ref.System()->GetGarbageData();
				if(garbage_data->GetLength()<1){
					for(int i=0;i<100;i++)  //somehow init hasn't been called yet?  This should never happen, but just in case
						garbage_data->WriteByte(rand());
				}

				const byte* p_garbage=garbage_data->GetBufferPtr();
				UINT garbage_length=garbage_data->GetLength();
				UINT g_offset=start_offset%garbage_length;
				UINT init_copy_len=min((garbage_length-g_offset),length);
				memcpy(buffer+cur_buf_pos,p_garbage+g_offset,init_copy_len);
				start_offset+=init_copy_len;
				length-=init_copy_len;	
				cur_buf_pos+=init_copy_len;  //advance the buffer position to account for how much data we've already predicted
				while(length>0){  //continue filling with garbage data, looping
					memcpy(buffer+cur_buf_pos,p_garbage,min(garbage_length,length));
					cur_buf_pos+=garbage_length;  //advance the buffer position to account for how much data we've already predicted
					length-=min(garbage_length,length);
				}
			}
		}
	}
	else{
		MetaSpooferReference ref;
		
		Buffer2000 *evil_data=ref.System()->GetEvilSeedData();

		UINT pos=start_offset;
		UINT end_pos=pos+length;

		if(end_pos>m_size)
			return;

		UINT evil_length=evil_data->GetLength();

		//do something special for the start of the file
		if(pos<evil_length){  //are we looking at the start of the data, where we will need to use evil data
			const byte *p_evil=evil_data->GetBufferPtr();
			UINT amount_to_copy=min(evil_length-pos,end_pos-pos);
			memcpy(buffer,p_evil+pos,amount_to_copy);
			pos+=amount_to_copy;
		}
		
		//put zeroes for any data that is neither the start nor the last SIGNATURELENGTH bytes of the file
		if(pos<end_pos && pos<(m_size-SIGNATURELENGTH)){
			UINT amount_to_zero=min(m_size-pos-SIGNATURELENGTH,end_pos-pos);
			ZeroMemory(buffer+pos-start_offset,amount_to_zero);
			pos+=amount_to_zero;
		}

		//append our signature to the end data
		if(pos<end_pos){
			UINT sig_offset=pos%SIGNATURELENGTH;
			UINT init_copy_len=min((SIGNATURELENGTH-sig_offset),(end_pos-pos));
			memcpy(buffer+pos-start_offset,m_signature+sig_offset,init_copy_len);
			pos+=init_copy_len;
			
			//int cur_buf_pos=init_copy_len;
			while(pos<end_pos){
				memcpy(buffer+pos-start_offset,m_signature,min(SIGNATURELENGTH,end_pos-pos));
				pos+=SIGNATURELENGTH;
			}


			/*
			for(;pos<end_pos && pos<m_size;pos++){
				buffer[pos-start_offset]=m_signature[(pos)&255];
			}*/
		}
	}
}

void DonkeyFile::ForceCompletable(void)
{
	mb_completable=true;
}

//a special function used to calculate the hash of an evil decoy, it assumes the file is large (greater than 10 megs)
void DonkeyFile::CreateEvilFileFromSignature(void)
{
	TRACE("DonkeyFile::CreateEvilFileFromSignature() file %s\n",m_file_name.c_str());
	// open file
	MetaSpooferReference ref;
	
	// create hashset
	Buffer2000 *evil_data=ref.System()->GetEvilSeedData();

	UINT togo = m_size;

	byte *part_buffer=new byte[PARTSIZE];
	
	byte start_hash[16];
	byte middle_hash[16];
	byte a_hash[16];

	PredictData(part_buffer,0,min(PARTSIZE,m_size));
	
	CreateHashFromInput(0,0,PARTSIZE,start_hash,part_buffer);

	ZeroMemory(part_buffer,PARTSIZE);
	CreateHashFromInput(0,0,PARTSIZE,middle_hash,part_buffer);
	
	UINT hashcount=0;
	//add the start hash
	mv_hash_set.Add(new HashPart((byte*)start_hash));
	
	togo-=PARTSIZE;

	//add all the middle hashes
	while(togo>=(PARTSIZE+SIGNATURELENGTH)){
		mv_hash_set.Add(new HashPart((byte*)middle_hash));
		togo -= PARTSIZE;
	}	

	if(togo>=PARTSIZE){ //add the chunk before the last
		PredictData(part_buffer,m_size-togo,PARTSIZE);
		CreateHashFromInput(0,0,PARTSIZE,a_hash,part_buffer);
		mv_hash_set.Add(new HashPart((byte*)a_hash));
		togo-=PARTSIZE;
	}

	if(togo>0){ //add the last fractional part
		PredictData(part_buffer,m_size-togo,togo);
		CreateHashFromInput(0,0,togo,a_hash,part_buffer);
		mv_hash_set.Add(new HashPart((byte*)a_hash));
		togo=0;
	}
	
	delete []part_buffer;  //delete the 10 megs of buffer that we don't need anymore
	//add the first hash

	//compute the hash from the hashes of the parts
	byte* buffer = new byte[mv_hash_set.Size()*16];
	for (int i = 0; i < (int)mv_hash_set.Size(); i++){
		md4cpy(buffer+(i*16), ((HashPart*)mv_hash_set.Get(i))->m_hash);
	}
	CreateHashFromInput(0,0,mv_hash_set.Size()*16,m_hash,buffer);
	delete[] buffer;

}
