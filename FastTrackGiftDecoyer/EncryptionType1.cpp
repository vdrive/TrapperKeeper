#include "StdAfx.h"
#include "encryptiontype1.h"

EncryptionType1::EncryptionType1(void)
{
}

//
//
//
EncryptionType1::~EncryptionType1(void)
{
}

//
//
//
void EncryptionType1::enc_type_1 (u8 *out_key, u8 *in_key)
{
	u32 exp[64], num1[64];
	u8 num2[256];

	big_set (exp, 3);

	memcpy (num2, in_key, 255);
	num2[255] = 1;

	big_letoh (num1, num2);
	big_expmod (num1, exp, Modulus);
	big_htole (out_key, num1, 256);
}

void EncryptionType1::big_set (u32 *num, u32 val)
{
	memset (num, 0, 4 * 64);
	num[0] = val;
}

void EncryptionType1::big_letoh (u32 *dst, u8 *src)
{
	int i, j;
	u32 tmp;

	for (i = 0; i < 64; i++)
	{
		for (j = 0, tmp = 0; j < 32; j += 8)
			tmp |= ((u32) (*src++)) << j;
		dst[i] = tmp;
	}
}

void EncryptionType1::big_htole (u8 *dst, u32 *src, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++)
		dst[i] = (src[i >> 2] >> ((i & 3) << 3)) & 0xff;
}

int EncryptionType1::big_getbit (u32 *num, int i)
{
	return (num[i >> 5] >> (i & 0x1f)) & 1;
}

void EncryptionType1::big_expmod (u32 *num, u32 *exp, u32 *mod)
{
	u32 lnum[64];
	int i, j;

	for (i = 2048; i > 0; i--)
		if (big_getbit (exp, i - 1) != 0)
			break;

	big_set (lnum, 1);

	for (j = 0; j < i; j++)
	{
		big_mulmod (lnum, num, mod);
		big_mulmod (num, num, mod);
	}

	memcpy (num, lnum, 64 * 4);
}

void EncryptionType1::big_mulmod (u32 *num1, u32 *num2, u32 *mod)
{
	u32 lnum[128];
	int len = mod[63] ? 64 : 32;

	big_mul (len, lnum, num1, num2);
	big_mod (len, num1, lnum, mod);
}

void EncryptionType1::big_mul (int cnt, u32 *out, u32 *in1, u32 *in2)
{
	int i, j;
	u64 k;

	memset (out, 0, cnt * 8);

	for (i = 0; i < cnt; i++)
	{
		for (j = 0, k = 0; j < cnt; j++)
		{
			k += (u64) out[i + j] + (u64) in1[i] * (u64) in2[j];
			out[i + j] = k & 0xffffffff;
			k >>= 32;
		}
		out[i + j] = k;
	}
}

int EncryptionType1::big_isless (int cnt, u32 *num1, u32 *num2)
{
	for (cnt--; cnt >= 0; cnt--)
	{
		if (num1[cnt] < num2[cnt])
			return 1;
		else if (num1[cnt] > num2[cnt])
			return 0;
	}

	return 0;
}

void EncryptionType1::big_mod (int cnt, u32 *out, u32 *in1, u32 *in2)
{
	u64 x;
	u32 k, l;
	int i, j;

	for (i = cnt - 1; i >= 0; i--)
	{
		k = x = (((u64) in1[cnt + i] << 32) +
			 (u64) in1[cnt + i - 1]) / ((u64) in2[cnt - 1] + 1);

		for (j = 0, l = 0; j < cnt; j++)
		{
			x = (u64) k *(u64) in2[j] + l;

			l = x >> 32;
			if (in1[i + j] < (x & 0xffffffff))
				l++;
			in1[i + j] -= x;
		}
		in1[cnt + i] -= l;

		while (in1[cnt + i] != 0 || !big_isless (cnt, in1 + i, in2))
		{
			for (j = 0, l = 0; j < cnt; j++)
			{
				in1[i + j] -= l;
				l = in1[i + j] < in2[j];
				in1[i + j] -= in2[j];
			}
			in1[cnt + i] -= l;
		}
	}

	memcpy (out, in1, cnt * 4);
}