#pragma once
typedef unsigned char u8;
typedef unsigned int u32;
#define mix_minor20 ROREQ (pad[12], pad[14] * 0x3)
#define mix_minor21 pad[12] *= pad[1] * 0x4b4f2e1
#define mix_minor22 pad[2] *= pad[10] + 0xfa1f1e0b
#define mix_minor23 pad[19] += pad[19] ^ 0x43b6b05
#define mix_minor24 pad[18] -= ROR (pad[4], 18)
#define mix_minor25 pad[0] &= pad[10] + 0xfc9be92d
#define mix_minor26 pad[9] ^= pad[3] + 0xbe5fec7d
#define mix_minor27 pad[6] *= pad[15] | 0x46afede0
#define mix_minor28 pad[17] -= pad[6] * 0x1b677cc8
#define mix_minor29 pad[14] &= pad[15] + 0xfc471d2b
#define mix_minor30 pad[19] += pad[16] + 0x24a7d94d
#define mix_minor31 pad[15] *= pad[0] ^ 0x48ad05f2
#define mix_minor32 pad[16] -= pad[4] - 0xbb834311
#define mix_minor33 pad[8] += ROL (pad[4], 26)
#define mix_minor34 pad[13] *= pad[18] + 0xac048a2
#define mix_minor35 pad[16] &= pad[18] + 0xe832eb88
#define mix_minor36 pad[4] -= pad[1] - 0xe6f17893
#define mix_minor37 pad[6] *= pad[7] | 0x17b60bb5
#define mix_minor38 pad[15] += ROL (pad[12], 16)
#define mix_minor39 pad[6] &= pad[10] + 0xfd7af7e
#define mix_minor40 ROREQ (pad[7], pad[18] & 2)
#define mix_minor41 ROREQ (pad[17], pad[7] ^ 3)
#define mix_minor42 pad[0] ^= pad[8] + 0xeee530d5
#define mix_minor43 pad[10] += pad[1] + 0xc484cfa2
#define mix_minor44 pad[16] += pad[5] ^ 0x19a836dc
#define mix_minor45 pad[17] += pad[7] + 0xd68a11c3
#define mix_minor46 pad[17] += ROL (pad[7], 19)
#define mix_minor47 pad[18] -= pad[6] * 0x368eaf4e
#define mix_minor48 ROREQ (pad[2], pad[7] ^ 3)
#define mix_minor49 pad[19] |= pad[5] + 0xda7c6c8e
#define mix_minor50 pad[6] *= ROR (pad[2], 12)
#define mix_minor51 pad[14] += pad[18] + 0xf655a040
#define mix_minor52 pad[11] += pad[19] * 0x251df1bd
#define mix_minor53 pad[11] -= pad[0] ^ 0x51a859c
#define mix_minor54 pad[18] += pad[6] + 0xdcccfc5
#define mix_minor55 pad[16] -= pad[18] ^ 0x39848960
#define mix_minor56 pad[14] ^= pad[19] + 0x1a6f3b29
#define mix_minor57 pad[12] &= pad[5] + 0x4ef1335a
#define mix_minor58 pad[14] *= pad[13] + 0xdb61abf8
#define mix_minor59 pad[18] ^= pad[19] * 0x378f67
#define mix_minor60 pad[18] ^= pad[4] * 0x2dd2a2fe
#define mix_minor61 pad[16] -= pad[4] - 0xe357b476
#define mix_minor62 pad[6] *= pad[16] * 0x381203
#define mix_minor63 pad[10] |= ROR (pad[11], 24)
#define mix_minor64 pad[10] ^= pad[5] + 0x147c80d5
#define mix_minor65 pad[16] ^= pad[3] * 0x27139980
#define mix_minor66 ROREQ (pad[15], pad[17])
#define mix_minor67 pad[14] &= ROL (pad[19], 6)
#define mix_minor68 pad[8] *= pad[0] * 0x1a4c02dd
#define mix_minor69 pad[16] ^= pad[14] + 0xfddb63a2

#define ROR(value, count) ((value) >> ((count) & 0x1f) | ((value) << (32 - (((count) & 0x1f)))))
#define ROL(value, count) ((value) << ((count) & 0x1f) | ((value) >> (32 - (((count) & 0x1f)))))

#define ROREQ(value, count) value = ROR(value, count)
#define ROLEQ(value, count) value = ROL(value, count)


class EncryptionType2
{
public:
	EncryptionType2(void);
	~EncryptionType2(void);

	/* this all works on unsigned ints so endianess is not an issue */

	static void enc_type_2(unsigned int *key, unsigned int seed)
	{
		mix_major0 (key, seed);
	}

	static void mix_major0 (u32 *pad, u32 seed);
	static void mix_major1 (u32 *pad, u32 seed);
	static void mix_major2 (u32 *pad, u32 seed);
	static void mix_major3 (u32 *pad, u32 seed);
	static void mix_major4 (u32 *pad, u32 seed);
	static void mix_major5 (u32 *pad, u32 seed);
	static void mix_major6 (u32 *pad, u32 seed);
	static void mix_major7 (u32 *pad, u32 seed);
	static void mix_major8 (u32 *pad, u32 seed);
	static void mix_major9 (u32 *pad, u32 seed);
	static void mix_major10 (u32 *pad, u32 seed);
	static void mix_major11 (u32 *pad, u32 seed);
	static void mix_major12 (u32 *pad, u32 seed);
	static void mix_major13 (u32 *pad, u32 seed);
	static void mix_major14 (u32 *pad, u32 seed);
	static void mix_major15 (u32 *pad, u32 seed);
	static void mix_major16 (u32 *pad, u32 seed);
	static void mix_major17 (u32 *pad, u32 seed);
	static void mix_major18 (u32 *pad, u32 seed);
	static void mix_major19 (u32 *pad, u32 seed);
	static void mix_major20 (u32 *pad, u32 seed);
	static void mix_major21 (u32 *pad, u32 seed);
	static void mix_major22 (u32 *pad, u32 seed);
	static void mix_major23 (u32 *pad, u32 seed);
	static void mix_major24 (u32 *pad, u32 seed);
};
