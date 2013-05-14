#pragma once
typedef unsigned int u32;
typedef unsigned char u8;

/* our not so structured exception handling */

#define TRY(x) if((x) & 1) return;

/* some helper funcs */

#ifndef __GNUC__
#define __attribute__(x)
#endif
/* and so it begins... */

#define minor_23 pad[5] += pad[4] * 73
#define minor_24 pad[0] += pad[5] & 0x1B65B2C8
#define minor_25 pad[9] += pad[6] + 0x124D08A0
#define minor_26 pad[16] ^= (pad[0] * 41) << 1
#define minor_27 pad[14] ^= pad[10] - 0x403483CE
#define minor_28 pad[3] ^= (pad[11] * 41) << 1
#define minor_29(x) pad[10] = ROL (pad[10], x)
#define minor_30 pad[1] = ROR (pad[1], (pad[8] * 7) << 4)
#define minor_31 pad[10] *= ROL (pad[1], 0xc)
#define minor_32(x) pad[18] = ROL (pad[18], x - 0xE066BF0)
#define minor_33(x) pad[18] -= x
#define minor_34 pad[16] |= pad[18] | 0xB25175E
#define minor_35 pad[2] |= pad[15] - 0x1886D6A
#define minor_36 pad[16] -= ROL (pad[3], 0x1b)
#define minor_37 pad[11] |= pad[13] * 9
#define minor_38 pad[6] &= pad[19] ^ 0x1FAF0F41
#define minor_39 pad[4] = ROR (pad[4], (pad[17] * 11) << 3)
#define minor_40 pad[17] *= pad[6]
#define minor_41 pad[3] ^= pad[9] + 0x5B1A81FD
#define minor_42 pad[17] *= ROR (pad[10], 0x1a)
#define minor_43 pad[19] ^= pad[3] ^ 0x19859C46
#define minor_44 pad[16] += pad[1] + 0x5EDB78DA
#define minor_45 pad[12] = ROL (pad[12], (pad[3] * 43) << 1)
#define minor_46 pad[17] *= pad[13] - 0x68C0E272
#define minor_47 pad[13] += pad[13] | 0x5E919E06
#define minor_48 pad[16] = (pad[9] * pad[16] * 37) << 1
#define minor_49 pad[2] |= pad[15] + 0x44B04775
#define minor_50 pad[8] = (pad[8] * pad[8] * 13) << 2
#define minor_51(x) pad[17] = ROR (pad[17], x)
#define minor_52(x) pad[10] *= x
#define minor_53 pad[8] += pad[17] + 0x4E0679BE
#define minor_54 pad[10] ^= pad[6] ^ 0x2BE68205
#define minor_55(x) pad[7] -= x
#define minor_56(x) pad[13] &= ROL (x, 0x10)
#define minor_57(x) pad[9] &= x
#define minor_58(x) pad[0] += x
#define minor_59 pad[8] *= pad[6] ^ 0x377C08D2
#define minor_60 pad[18] -= pad[10] < 0xB6C6C3E ? pad[10] : pad[13]
#define minor_61 pad[19] += my_sqrt (pad[18])
#define minor_62 pad[11] ^= my_sqrt (pad[4])
#define minor_63 pad[17] ^= my_sqrt (pad[16])
#define minor_64 pad[9] += my_sqrt (pad[13])
#define minor_65(x) pad[10] -= my_cos (x) ? 0x19C6C6E : pad[7]
#define minor_66 pad[10] -= my_cos (pad[10]) ? 0x5F18F01 : pad[13]
#define minor_67(x) pad[17] = ROL (pad[17], my_sin (pad[7]) ? 0x4262ED6B : x)
#define minor_68 pad[9] += my_cos (pad[6]) ? 0x14F5046C : pad[11]
#define minor_69 pad[14] ^= my_cos (pad[11]) ? 0x562482FA : pad[14]
#define minor_70 pad[3] -= my_cos (pad[14]) ? 0x39702EDD : pad[6]
#define minor_71 pad[6] ^= my_cos (pad[10]) ? 0x53236223 : pad[7]
#define minor_72(x) pad[1] |= my_sin (x) ? 0x1BE7FECF : pad[16]
#define minor_73 pad[17] *= my_cos (pad[16]) ? 0x2C15B485 : pad[3]
#define minor_3 pad[19] += my_sqrt (pad[0])
#define minor_4 pad[7] = ROL (pad[7], my_sqrt (pad[1]))
#define minor_5 pad[4] += my_cos (pad[1]) ? 0x890AFEF : pad[4]
#define minor_6(x) pad[7] = ROL (pad[7], my_sin (pad[18]) ? 0x14D1DE3D : x)
#define minor_7 pad[10] &= pad[1] < 0x1F2DD61 ? pad[1] : pad[10]
#define minor_8 pad[0] |= ((pad[10] * 3) << 4) + pad[10]
#define minor_9 pad[9] ^= pad[1] < 0xB01609F ? pad[1] : pad[16]
#define minor_10(x) pad[13] = ROR (pad[13], x)
#define minor_11(x) pad[7] &= x
#define minor_12 pad[11] += ROL (pad[5], 0x14)
#define minor_13(x) pad[5] ^= x + 0x391B56A
#define minor_14 pad[3] ^= pad[14] * 0x711881F7
#define minor_15 pad[6] -= pad[18] ^ 0x4402CAF
#define minor_16 pad[2] += ROR (pad[14], 0x14)
#define minor_17 pad[6] += 0x287735D1 - pad[3]
#define minor_18 pad[3] += 0x247B4DE9 - pad[1]
#define minor_19 pad[11] += pad[6] * 0xE8C67004
#define minor_20 pad[15] |= pad[1] ^ 0x37B6FE35
#define minor_21 pad[14] = ROR (pad[14], pad[13] | 0x3433BE6)

class EncryptionType80
{
public:
	EncryptionType80(void);
	~EncryptionType80(void);
	/* the entry point of this mess */
	/* this all works on unsigned ints so endianess is not an issue */

	static void major_0 (u32 *pad, u32 seed);
	static void enc_type_80 (u32 *pad, u32 seed);

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
	static void major_20 (u32 *pad, u32 seed);
	static void major_21 (u32 *pad, u32 seed);

	static void minor_74 (u32 *pad, u32 seed);
	static void minor_75 (u32 *pad);
};



