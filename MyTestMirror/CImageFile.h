#pragma once
#include <Windows.h>
#include <string>
#pragma warning(disable:4996)

enum _IMAGE_FORMATE_TYPE
{
	IMAGE_BMP,
};

class CImageFile
{
public:
	CImageFile();
	void SetOutDirectory(std::string OutDirPath);
	void DumpImage(char* szName, char* Buff, int nSize, int nWidth, int nHeigth, int nBitPerPixel, int nType = IMAGE_BMP, bool bOverlay = false);
protected:
	std::string GetFileName(char* szName, int nType, bool bOverlay);
	std::string GetFileSuffix(int nType);
	void DumpBmp(char* Buff, int nSize, int nWidth, int nHeigth, int nBitPerPixel);
private:
	std::string m_OutDirPath;
	std::string m_FileName;
};

