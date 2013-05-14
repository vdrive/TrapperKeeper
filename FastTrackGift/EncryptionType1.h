#pragma once
//#include <string.h>				/* memcpy(), memset() */
#ifdef WIN32
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif							/* WIN32 */

typedef unsigned int u32;
typedef unsigned char u8;

static u32 Modulus[] = {
	0x0eb96841d, 0x031c4a05e, 0x0193bbd8f, 0x0bf77b6d6, 0x096d7f927,
		0x0e569c5fc, 0x05efa55ff, 0x0ba1519a1, 0x01b32a36f, 0x0e84b25f8,
		0x08d5b5eb2, 0x0c11f00e3, 0x019d2974d, 0x0e7ee26ad, 0x0bc8c0457,
		0x091fd3a9e, 0x06e37192e, 0x0e475aa9e, 0x072f554fc, 0x0695b3ca2,
		0x0f97bb445, 0x069f29f9a, 0x0b15e2216, 0x0390a5a36, 0x02c0054b3,
		0x0363dc15e, 0x0d238b5b8, 0x0e957a9b5, 0x00032a3df, 0x08781ab21,
		0x00a7703d8, 0x05250e6fe, 0x0b42d2df0, 0x010a931f8, 0x01e4970e5,
		0x0c3741ca7, 0x08606342a, 0x0d61a7055, 0x018e4dd48, 0x00829ee6d,
		0x08443fd05, 0x0e9db6341, 0x0de3c6725, 0x0a4037cdb, 0x096e537a7,
		0x0ed64060a, 0x0da2d7944, 0x051be98b0, 0x0315d5780, 0x0b6a31787,
		0x04d23ffc1, 0x0dbd26bb0, 0x0de7aa2fe, 0x071a157ac, 0x019288512,
		0x00d7a0050, 0x0522f69d1, 0x0dd331dd5, 0x096e62735, 0x0c6f2dc0e,
		0x0ae08473d, 0x00a532f6a, 0x034eb20ca, 0x0d453ec0a
};

class EncryptionType1
{
public:
	EncryptionType1(void);
	~EncryptionType1(void);

	static void enc_type_1 (u8 *out_key, u8 *in_key);
	static void big_set (u32 *num, u32 val);
	static void big_letoh (u32 *dst, u8 *src);
	static void big_htole (u8 *dst, u32 *src, int cnt);
	static void big_expmod (u32 *num, u32 *exp, u32 *mod);
	static void big_mulmod (u32 *num1, u32 *num2, u32 *mod);
	static void big_mul (int cnt, u32 *out, u32 *in1, u32 *in2);
	static void big_mod (int cnt, u32 *out, u32 *in1, u32 *in2);
	static int big_getbit (u32 *num, int i);
	static int big_isless (int cnt, u32 *num1, u32 *num2);
};
