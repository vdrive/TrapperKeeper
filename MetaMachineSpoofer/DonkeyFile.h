#pragma once
#include "..\tkcom\vector.h"
#include "Buffer2000.h"

#define	PARTSIZE				9728000
#define EMBLOCKSIZE				184320
#define SIGNATURELENGTH			512  //must divide 


class DonkeyFile :
	public Object
{
protected:
	bool mb_completable;
	bool mb_evil_file;
	void CreateFromSignature();
	bool CreateFromDecoy(LPCTSTR in_directory, LPCTSTR in_filename);
	void MD4Transform(UINT Hash[4], UINT x[16]);
	void CreateHashFromInput(FILE* file,CFile* file2, int Length, byte* Output, byte* in_string);

	CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
	{
		CString base16_buff;
		static byte base16_alphabet[] = "0123456789ABCDEF";

		for(unsigned int i = 0; i < bufLen; i++) {
			base16_buff += base16_alphabet[buffer[i] >> 4];
			base16_buff += base16_alphabet[buffer[i] & 0xf];
		}

		return base16_buff;
	}
public:
	string m_file_name;
	string m_file_path;
	string m_project;
	UINT m_size;
	byte m_hash[16];
	byte m_signature[SIGNATURELENGTH];
	string m_shash;
	Vector mv_hash_set;

	int m_seed;

	inline int CompareTo(Object *object){
		DonkeyFile *host=(DonkeyFile*)object;
		return stricmp(m_shash.c_str(),host->m_shash.c_str());
	}

//**** TAKEN FROM EMULE SOURCE ****//
// md4cmp -- replacement for memcmp(hash1,hash2,16)
// Like 'memcmp' this function returns 0, if hash1==hash2, and !0, if hash1!=hash2.
// NOTE: Do *NOT* use that function for determining if hash1<hash2 or hash1>hash2.
inline int md4cmp(const void* hash1, const void* hash2) {
	return !(((UINT*)hash1)[0] == ((UINT*)hash2)[0] &&
		     ((UINT*)hash1)[1] == ((UINT*)hash2)[1] &&
		     ((UINT*)hash1)[2] == ((UINT*)hash2)[2] &&
		     ((UINT*)hash1)[3] == ((UINT*)hash2)[3]);
}

// md4clr -- replacement for memset(hash,0,16)
inline void md4clr(const void* hash) {
	((UINT*)hash)[0] = ((UINT*)hash)[1] = ((UINT*)hash)[2] = ((UINT*)hash)[3] = 0;
}

// md4cpy -- replacement for memcpy(dst,src,16)
inline void md4cpy(void* dst, const void* src) {
	((UINT*)dst)[0] = ((UINT*)src)[0];
	((UINT*)dst)[1] = ((UINT*)src)[1];
	((UINT*)dst)[2] = ((UINT*)src)[2];
	((UINT*)dst)[3] = ((UINT*)src)[3];
}
//**** END TAKEN FROM EMULE SOURCE ****//

public:
	DonkeyFile(LPCTSTR in_directory, LPCTSTR in_filename);
	DonkeyFile(LPCTSTR in_filename, UINT length,byte* signature,bool b_possibly_evil,bool b_force_evil);
	DonkeyFile(const char* file_name,byte* hash,int size);
	DonkeyFile(const char* file_name,byte* hash,int size,byte* hashparts);
	~DonkeyFile(void);
	void WriteFileStatus(Buffer2000 &packet,bool b_false_decoy);
	void WriteHashSet(Buffer2000 & packet);
	void SetProject(const char* project);
	
	void PredictData(byte* buffer, UINT start_offset, UINT length);
	void ForceCompletable(void);
protected:
	void CreateEvilFileFromSignature(void);
};
