#pragma once

////////////////////////////////////////
namespace Kademlia {
////////////////////////////////////////

class CUInt128
{
public:

	CUInt128();
	CUInt128(bool fill);
	CUInt128(ULONG value);
	CUInt128(const byte *valueBE);
	/**
	 * Generates a new number, copying the most significant 'numBits' bits from 'value'.
	 * The remaining bits are randomly generated.
	 */
	CUInt128(const CUInt128 &value, UINT numBits = 128);

	const byte* getData() const { return (byte*)m_data; }
	byte* getDataPtr() const { return (byte*)m_data; }

	/** Bit at level 0 being most significant. */
	UINT getBitNumber(UINT bit) const;
	int compareTo(const CUInt128 &other) const;
	int compareTo(ULONG value) const;

	void toHexString(CString *str) const;
	void toBinaryString(CString *str, bool trim = false) const;
	void toByteArray(byte *b) const;

	ULONG get32BitChunk(int val) const {return m_data[val];}

	CUInt128& setValue(const CUInt128 &value);
	CUInt128& setValue(ULONG value);
	CUInt128& setValueBE(const byte *valueBE);

	CUInt128& setValueRandom(void);
	CUInt128& setValueGUID(void);

	CUInt128& setBitNumber(UINT bit, UINT value);
	CUInt128& shiftLeft(UINT bits);

	CUInt128& add(const CUInt128 &value);
	CUInt128& add(ULONG value);
	CUInt128& subtract(const CUInt128 &value);
	CUInt128& subtract(ULONG value);

	CUInt128& xor(const CUInt128 &value);
	ULONG calcDistance(const CUInt128 &value);
	CUInt128& xorBE(const byte *valueBE);

	void operator+  (const CUInt128 &value) {add(value);}
	void operator-  (const CUInt128 &value) {subtract(value);}
	void operator=  (const CUInt128 &value) {setValue(value);}
	bool operator<  (const CUInt128 &value) const {return (compareTo(value) <  0);}
	bool operator>  (const CUInt128 &value) const {return (compareTo(value) >  0);}
	bool operator<= (const CUInt128 &value) const {return (compareTo(value) <= 0);}
	bool operator>= (const CUInt128 &value) const {return (compareTo(value) >= 0);}
	bool operator== (const CUInt128 &value) const {return (compareTo(value) == 0);}
	bool operator!= (const CUInt128 &value) const {return (compareTo(value) != 0);}

	void operator+  (ULONG value) {add(value);}
	void operator-  (ULONG value) {subtract(value);}
	void operator=  (ULONG value) {setValue(value);}
	bool operator<  (ULONG value) const {return (compareTo(value) <  0);}
	bool operator>  (ULONG value) const {return (compareTo(value) >  0);}
	bool operator<= (ULONG value) const {return (compareTo(value) <= 0);}
	bool operator>= (ULONG value) const {return (compareTo(value) >= 0);}
	bool operator== (ULONG value) const {return (compareTo(value) == 0);}
	bool operator!= (ULONG value) const {return (compareTo(value) != 0);}

private:

	ULONG m_data[4];
};

}