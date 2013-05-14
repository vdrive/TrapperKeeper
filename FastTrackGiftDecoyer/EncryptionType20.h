#pragma once

typedef unsigned int u32;

/* our crude SEH replacement */

#define TRY(x) if((x) & 1) return;

/* some helper funcs */

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define ROR(value, count) ((value) >> ((count) & 0x1f) | ((value) << (32 - (((count) & 0x1f)))))
#define ROL(value, count) ((value) << ((count) & 0x1f) | ((value) >> (32 - (((count) & 0x1f)))))

#define ROREQ(value, count) value = ROR(value, count)
#define ROLEQ(value, count) value = ROL(value, count)
/* minor_ implementation details below this line ;) */

#define minor_1(x) pad[8] += my_sin(x&255) ? 0x4f0cf8d : x
#define minor_2(x) pad[2] += pad[2] < 0x36def3e1 ? pad[2] : x
#define minor_3 pad[10] ^= ROL(pad[1], 20)
#define minor_4 pad[16] -= pad[6]
#define minor_5 pad[10] -= pad[9] * 0x55
#define minor_6 ROLEQ(pad[0], pad[19] ^ 0xc)
#define minor_7 pad[17] += pad[8] * 0xf6084c92
#define minor_8 pad[12] ^= pad[10] & 0x28acec82
#define minor_9(x) pad[12] *= pad[12] < 0x12d7bed ? pad[12] : x
#define minor_10(x) pad[18] += pad[5] < 0xfd0aa3f ? pad[5] : x
#define minor_12(x) pad[11] &= my_cos(pad[18]) ? 0x146a49cc : x
#define minor_13 pad[2] &= my_cos(pad[2]) ? 0x7ebbfde : pad[11]
#define minor_17 pad[19] ^= pad[7] * 0x3a
#define minor_19 ROLEQ(pad[6], ROR(pad[8], 14))
#define minor_20 pad[0] &= ROR(pad[18], 31)
#define minor_22 ROREQ(pad[3], pad[11] ^ 0x7)
#define minor_26 pad[0] |= my_cos(pad[1]) ? 0x56e0e99 : pad[8]
#define minor_27 pad[18] += my_cos(pad[15]) ? 0x10d11d00 : pad[9]
#define minor_28 pad[10] -= my_cos(pad[15]) ? 0x268cca84 : pad[9]
#define minor_29 pad[3] -= my_cos(pad[6]) ? 0x2031618a : pad[8]
#define minor_30 ROLEQ(pad[1], my_sin(pad[5]) ? 4 : pad[6])
#define minor_31(x) ROREQ(pad[17], my_sin(pad[6]) ? 29 : x)
#define minor_32(x) pad[15] ^= my_sin(pad[14]) ? 0x40a33fd4 : x
#define minor_34 pad[7] ^= my_sqrt(pad[11])
#define minor_35 pad[5] += my_sqrt(pad[7])

class EncryptionType20
{
public:
	EncryptionType20(void);
	~EncryptionType20(void);
	/* the entry point of this mess */
	/* this all works on unsigned ints so endianess is not an issue */

	static void mix (u32 *pad, u32 seed);

	static void enc_type_20 (u32 *pad, u32 seed);

	/* major functions which make calls to other funcs */

	static void major_1 (u32 *pad, u32 seed);
	static void major_2 (u32 *pad, u32 seed);
	static void major_3 (u32 *pad, u32 seed);
	static void major_4 (u32 *pad, u32 seed);
	static void major_5 (u32 *pad, u32 seed);
	static void major_6 (u32 *pad, u32 seed);
	static void major_7 (u32 *pad, u32 seed);
	static void major_8 (u32 *pad, u32 seed);
	static void major_9 (u32 *pad, u32 seed);
	static void major_10 (u32 *pad, u32 seed);
	static void major_11 (u32 *pad, u32 seed);
	static void major_12 (u32 *pad, u32 seed);
	static void major_13 (u32 *pad, u32 seed);
	static void major_14 (u32 *pad, u32 seed);
	static void major_15 (u32 *pad, u32 seed);
	static void major_16 (u32 *pad, u32 seed);
	static void major_17 (u32 *pad, u32 seed);
	static void major_18 (u32 *pad, u32 seed);
	static void major_19 (u32 *pad, u32 seed);
	static void major_21 (u32 *pad, u32 seed);
	static void major_22 (u32 *pad, u32 seed);
	static void major_23 (u32 *pad, u32 seed);
	static void major_24 (u32 *pad, u32 seed);
	static void major_25 (u32 *pad, u32 seed);

	/* simple pad manipulation functions */

	static void minor_36 (u32 *pad);
	static void minor_37 (u32 *pad);
};
