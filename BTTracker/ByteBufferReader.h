#pragma once

class ByteBufferReader
{
public:
	ByteBufferReader(void);
	ByteBufferReader(const CHAR *pData, size_t nLen);
	ByteBufferReader(const BYTE *pData, size_t nLen);
	~ByteBufferReader(void);

	void SetBuffer(const BYTE *pData, size_t nLen);
	void SetBuffer(const CHAR *pData, size_t nLen);

	// Returns the size of the buffer
	inline size_t Size(void) const { return m_size; };

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
	inline bool ValidIndex(size_t index) const { return ( index < m_size ); };
	inline bool ValidRange(size_t index, size_t numOfBytes) const { return ( (index + numOfBytes - 1) < m_size ); };

protected:
	const BYTE *m_pData;
	size_t m_size;
};
