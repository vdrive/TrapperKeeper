#include "StdAfx.h"
#include "hashconversion.h"

HashConversion::HashConversion(void)
{
	memset(this, 0, sizeof(HashConversion));
}

HashConversion::~HashConversion(void)
{
}


//
//
//
// return pointer to static, base64 encoded HashConversion string
char * HashConversion::HashEncode64()
{
	static char str[128];
	char *buf;

	if (!(buf = Base64Encode (m_data, HASH_LEN)))
		return NULL;

	if (strlen (buf) > 127)
	{
		free (buf);
		return NULL;
	}

	//the leading '=' padding is used by kazaa and sig2dat
	str[0] = '=';
	strcpy (str+1, buf);
	free (buf);

	return str;	
}

//
//
//
/* return pointer to static, hex encoded HashConversion string */
char* HashConversion::HashEncode16()
{
	static char str[128];
	char *buf;

	if (!(buf = HexEncode(m_data, HASH_LEN)))
		return NULL;

	if (strlen (buf) >= 128)
	{
		free (buf);
		return NULL;
	}

	strcpy (str, buf);
	free (buf);

	return str;	
}

//
//
/* caller frees returned string */
char * HashConversion::HexEncode (const unsigned char *data, int src_len)
{
	static const char hex_string[] = "0123456789abcdefABCDEF";
	char *out, *dst;
	int i;

	if (!data)
		return NULL;

	if (! (out = dst = (char*)malloc (src_len * 2 + 1)))
		return NULL;

	for(i=0; i<src_len; i++, dst += 2)
	{
		dst[0] = hex_string[data[i] >> 4];
		dst[1] = hex_string[data[i] & 0x0F];
	}

	dst[0] = 0;

	return out;
}

//
//
//
/* parse hex encoded HashConversion */
BOOL HashConversion::HashDecode16(const char *HashConversion)
{
	byte *buf;
	int len;
	memset(m_data,0,sizeof(m_data));
	
	if (!(buf = HexDecode (HashConversion, &len)))
		return FALSE;

	if (len < HASH_LEN)
	{
		free (buf);
		return FALSE;
	}
	
	memcpy(&m_data, buf, HASH_LEN);
	free (buf);

	return TRUE;
}

//
//
//
/* caller frees returned string */
unsigned char * HashConversion::HexDecode (const char *data, int *dst_len)
{
	static const char hex_string[] = "0123456789abcdefABCDEF";
	char *dst, *h;
	int i;
	unsigned char hi, lo;

	if (!data)
		return NULL;

	if (! (dst = (char*)malloc (strlen (data) / 2 + 1)))
		return NULL;

	for(i=0; *data && data[1]; i++, data += 2)
	{
		/* high nibble */
		if( (h = strchr (hex_string, data[0])) == NULL)
		{
			free (dst);
			return NULL;
		}
		hi = (h - hex_string) > 0x0F ? (h - hex_string - 6) : (h - hex_string);

		/* low nibble */
		if ( (h = strchr (hex_string, data[1])) == NULL)
		{
			free (dst);
			return NULL;
		}
		lo = (h - hex_string) > 0x0F ? (h - hex_string - 6) : (h - hex_string);

		dst[i] = (hi << 4) | lo;
	}

	if (dst_len)
		*dst_len = i;

	return (unsigned char *)dst;
}

//
//
/* caller frees returned string */
char * HashConversion::Base64Encode (const unsigned char *data, int src_len)
{
	static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char *dst, *out;

	if(!data)
		return NULL;

	if((out = dst = (unsigned char*)malloc((src_len + 4) * 2)) == NULL)
		return NULL;

	for (; src_len > 2; src_len-=3, dst+=4, data+=3)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4) + (data[1] >> 4)];
		dst[2] = base64[((data[1] & 0x0f) << 2) + (data[2] >> 6)];
		dst[3] = base64[data[2] & 0x3f];
	}

	dst[0] = '\0';

	if (src_len == 1)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4)];
		dst[2] = '=';
		dst[3] = '=';
		dst[4] = '\0';
	}

	if (src_len == 2)
	{
		dst[0] = base64[data[0] >> 2];
		dst[1] = base64[((data[0] & 0x03) << 4) + (data[1] >> 4)];
		dst[2] = base64[((data[1] & 0x0f) << 2)];
		dst[3] = '=';
		dst[4] = '\0';
	}

	return (char*)out;
}

//
//
//
UINT HashConversion::GetFileSize()
{
	UINT file_size = 0;
	file_size = (*(UINT*)&m_data[HASH_LEN-4]);
	return file_size;
}