#pragma once

class Sig2DatMetaTag
{
public:
	Sig2DatMetaTag(void);
	~Sig2DatMetaTag(void);

	void CalculateLength();
	void SetValue(char * value);
	int GetSize();

	DWORD m_tagId;
	DWORD m_tagLength;
	byte m_tagValue[20];
};
