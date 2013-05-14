// BYTEBUFFER Class - ByteBuffer.h
// Written by Dylan Douglas

#pragma once

class buffer
{
public:
	buffer(void);
	buffer(const BYTE *pData, size_t len);
	buffer(const CHAR *pData, size_t len);
	buffer(const CHAR *pData);

	buffer(size_t len);
	~buffer(void);

	void SetBuffer(const BYTE *pData, size_t len);
	void SetBuffer(const CHAR *pData, size_t len);
	void SetBuffer(const CHAR *pData);
	void SetBuffer(size_t len);
	void ClearBuffer(void);
	void ResizeBuffer(size_t len);

	// Resets the index back to 0
	inline void ResetIndex(void) { m_index = 0; };
	// Returns the size of the buffer
	inline size_t Size(void) { return m_size; };
	// Returns the current index location
	inline size_t Index(void) { return m_index; };

	CHAR GetChar(size_t index);
	BYTE GetByte(size_t index);
	WORD GetWord(size_t index);
	DWORD GetDword(size_t index);
	SHORT GetShort(size_t index);
	USHORT GetUshort(size_t index);
	INT GetInt(size_t index);
	UINT GetUint(size_t index);
	LONG GetLong(size_t index);
	const CHAR *GetCharPtr(size_t index = 0);
	const UCHAR *GetUcharPtr(size_t index = 0);
	const BYTE *GetBytePtr(size_t index = 0);
	CHAR *GetMutableCharPtr(size_t index = 0);
	UCHAR *GetMutableUcharPtr(size_t index = 0);
	BYTE *GetMutableBytePtr(size_t index = 0);

	void SetChar(CHAR data, size_t index);
	void SetByte(BYTE data, size_t index);
	void SetWord(WORD data, size_t index);
	void SetDword(DWORD data, size_t index);
	void SetShort(SHORT data, size_t index);
	void SetUshort(USHORT data, size_t index);
	void SetInt(INT data, size_t index);
	void SetUint(UINT data, size_t index);
	void SetLong(LONG data, size_t index);
	void SetCharPtr(const CHAR *pData, size_t len, size_t index = 0);
	void SetBytePtr(const BYTE *pData, size_t len, size_t index = 0);

	void PutChar(CHAR data);
	void PutByte(BYTE data);
	void PutWord(WORD data);
	void PutDword(DWORD data);
	void PutShort(SHORT data);
	void PutUshort(USHORT data);
	void PutInt(INT data);
	void PutUint(UINT data);
	void PutLong(LONG data);
	void PutCharPtr(const CHAR *pData, size_t len);
	void PutBytePtr(const BYTE *pData, size_t len);
	void PutByteBuffer(buffer *pBuf, bool bDeleteAfter = false);

	// drops n bytes off the front of the buffer and resizes the buffer
	void DropFront(size_t len);
	// drops n bytes off the back of the buffer and resizes the buffer
	void DropBack(size_t len);

	// overloaded operators
	buffer &operator=( const buffer &right );
	bool operator==( const buffer &right );

protected:
	BYTE *m_data;
	size_t m_size;
	size_t m_index;
};
