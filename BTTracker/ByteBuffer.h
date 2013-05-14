/* 
** FILE: ByteBuffer.h
**
** AUTHOR:
**	Dylan Douglas
**
** CREATION DATE:
**	07/03/2004
**
** NOTES:
**	None
*/

#pragma once

class ByteBuffer
{
public:
	ByteBuffer(void);
	ByteBuffer(const BYTE *pData, size_t nLen);
	ByteBuffer(const CHAR *pData, size_t nLen);
	ByteBuffer(const CHAR *pData);
	ByteBuffer(const ByteBuffer &rBuf);
	ByteBuffer(size_t nLen);
	~ByteBuffer(void);

	void SetBuffer(const BYTE *pData, size_t nLen);
	void SetBuffer(const CHAR *pData, size_t nLen);
	void SetBuffer(const CHAR *pData);
	void SetBuffer(size_t nLen);
	void ResetBuffer(void);
	void ZeroOutBuffer(void);
	void ResizeBuffer(size_t nLen);
	inline bool ValidIndex(size_t index) const { return ( index < m_size ); };

	// Resets the index back to 0
	inline void ResetIndex(void) { m_index = 0; };
	// Returns the size of the buffer
	inline size_t Size(void) const { return m_size; };
	// Returns the current index location
	inline size_t Index(void) const { return m_index; };

	inline CHAR GetChar(size_t index) const { return ( index + sizeof(CHAR) > m_size ? 0 : (CHAR)m_pData[index] ); };
	inline BYTE GetByte(size_t index) const { return ( index + sizeof(BYTE) > m_size ? 0 : m_pData[index] ); };
	inline WORD GetWord(size_t index) const { return ( index + sizeof(WORD) > m_size ? 0 : *( (WORD *)( m_pData + index ) ) ); };
	inline DWORD GetDword(size_t index) const { return ( index + sizeof(DWORD) > m_size ? 0 : *( (DWORD *)( m_pData + index ) ) ); };
	inline SHORT GetShort(size_t index) const { return ( index + sizeof(SHORT) > m_size ? 0 : *( (SHORT *)( m_pData + index ) ) ); };
	inline USHORT GetUshort(size_t index) const { return ( index + sizeof(USHORT) > m_size ? 0 : *( (USHORT *)( m_pData + index ) ) ); };
	inline INT GetInt(size_t index) const { return ( index + sizeof(INT) > m_size ? 0 : *( (INT *)( m_pData + index ) ) ); };
	inline UINT GetUint(size_t index) const { return ( index + sizeof(UINT) > m_size ? 0 : *( (UINT *)( m_pData + index ) ) ); };
	inline LONG GetLong(size_t index) const { return ( index + sizeof(LONG) > m_size ? 0 : *( (LONG *)( m_pData + index ) ) ); };
	inline const CHAR *GetCharPtr(size_t index = 0) const { return ( index >= m_size ? NULL : (const CHAR *)( m_pData + index ) ); };
	inline const UCHAR *GetUcharPtr(size_t index = 0) const { return ( index >= m_size ? NULL : (const UCHAR *)( m_pData + index ) ); };
	inline const BYTE *GetBytePtr(size_t index = 0) const { return ( index >= m_size ? NULL : (const BYTE *)( m_pData + index ) ); };
	inline const void *GetVoidPtr(size_t index = 0) const { return ( index >= m_size ? NULL : m_pData + index ); };
	inline CHAR *GetMutableCharPtr(size_t index = 0) { return ( index >= m_size ? NULL : (CHAR *)( m_pData + index ) ); };
	inline UCHAR *GetMutableUcharPtr(size_t index = 0) { return ( index >= m_size ? NULL : (UCHAR *)( m_pData + index ) ); };
	inline BYTE *GetMutableBytePtr(size_t index = 0) { return ( index >= m_size ? NULL : (BYTE *)( m_pData + index ) ); };
	inline void *GetMutableVoidPtr(size_t index = 0) { return ( index >= m_size ? NULL : m_pData + index ); };

	void SetChar(CHAR data, size_t index);
	void SetByte(BYTE data, size_t index);
	void SetWord(WORD data, size_t index);
	void SetDword(DWORD data, size_t index);
	void SetShort(SHORT data, size_t index);
	void SetUshort(USHORT data, size_t index);
	void SetInt(INT data, size_t index);
	void SetUint(UINT data, size_t index);
	void SetLong(LONG data, size_t index);
	void SetCharPtr(const CHAR *pData, size_t nLen, size_t index = 0);
	void SetBytePtr(const BYTE *pData, size_t nLen, size_t index = 0);

	void PutChar(CHAR data);
	void PutByte(BYTE data);
	void PutWord(WORD data);
	void PutDword(DWORD data);
	void PutShort(SHORT data);
	void PutUshort(USHORT data);
	void PutInt(INT data);
	void PutUint(UINT data);
	void PutLong(LONG data);
	void PutCharPtr(const CHAR *pData);
	void PutCharPtr(const CHAR *pData, size_t nLen);
	void PutBytePtr(const BYTE *pData, size_t nLen);
	void PutByteBuffer(ByteBuffer *pBuf, bool bDeleteAfter = false);

	// drops n bytes off the front of the buffer and resizes the buffer
	void DropFront(size_t nLen);
	// drops n bytes off the back of the buffer and resizes the buffer
	void DropBack(size_t nLen);

	// overloaded operators
	ByteBuffer &operator=(const ByteBuffer &rRight);
	bool operator==(const ByteBuffer &rRight) const;

protected:
	BYTE *	m_pData;
	size_t	m_allocated;
	size_t	m_size;
	size_t	m_index;
};
