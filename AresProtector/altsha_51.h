#ifndef CRYPTOPP_altSHA_H
#define CRYPTOPP_altSHA_H

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/*
/// <a href="http://www.weidai.com/scan-mirror/md.html#SHA-1">SHA-1</a>
class CRYPTOPP_DLL altSHA : public IteratedHashWithStaticTransform<word32, BigEndian, 64, 20, altSHA>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	static const char *StaticAlgorithmName() {return "SHA-1";}
};

typedef altSHA altSHA1;
*/


class altSHA : public IteratedHashWithStaticTransform<word32, BigEndian, 64, altSHA>
{
public:
	enum {DIGESTSIZE = 20};
	altSHA() : IteratedHashWithStaticTransform<word32, BigEndian, 64, altSHA>(DIGESTSIZE) {Init();}
	static void Transform(word32 *digest, const word32 *data);
	static const char *StaticAlgorithmName() {return "SHA-1";}

protected:
	void Init();
};

typedef altSHA altSHA1;

NAMESPACE_END

#endif
