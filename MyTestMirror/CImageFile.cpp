#include "CImageFile.h"
#include <iostream>

CImageFile::CImageFile()
{
	m_OutDirPath = ".";
}

void CImageFile::SetOutDirectory(std::string OutDirPath)
{
	m_OutDirPath = OutDirPath;
}

void CImageFile::DumpImage(char* szName, char* Buff, int nSize, int nWidth, int nHeigth, int nBitPerPixel, int nType, bool bOverlay)
{
	m_FileName = GetFileName(szName, nType, bOverlay);

	switch (nType)
	{
	case IMAGE_BMP:DumpBmp(Buff, nSize, nWidth, nHeigth, nBitPerPixel);
	default:
		return;
	}
}

std::string CImageFile::GetFileName(char* szName, int nType, bool bOverlay)
{
	static int nIndex = 0;

	std::string FileName = m_OutDirPath;
	FileName.append("\\").append(szName);

	if (bOverlay)
	{
		FileName.append(std::to_string(nIndex++));
	}

	FileName.append(GetFileSuffix(nType));

	return FileName;
}

std::string CImageFile::GetFileSuffix(int nType)
{
	switch (nType)
	{
	case IMAGE_BMP:return std::string(".bmp");
	default:
		return std::string();
	}	
}

void CImageFile::DumpBmp(char* Buff, int nSize, int nWidth, int nHeigth, int nBitPerPixel)
{
	BITMAPINFOHEADER infoHeader;
	BITMAPFILEHEADER fileHeader;
	const int headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	const int biBitCount = nBitPerPixel * 8;

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = nWidth;
	infoHeader.biHeight = nHeigth;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = (WORD)biBitCount;
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage = nSize;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = (biBitCount <= 8) ? 1 << biBitCount : 0;
	infoHeader.biClrImportant = 0;

	//位图文件头结构
	fileHeader.bfType = 0x4D42;//set the attribute of BITMAPFILEHEADER
	fileHeader.bfSize = headerSize + nSize;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = headerSize;

	HANDLE pFile = CreateFileA(m_FileName.c_str(),GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == pFile)
	{
		std::cout << "CreateFileA fail:" << GetLastError();
		return;
	}

	ULONG written;
	WriteFile(pFile, &fileHeader, sizeof(BITMAPFILEHEADER), &written, NULL);
	WriteFile(pFile, &infoHeader, sizeof(BITMAPINFOHEADER), &written, NULL);
	WriteFile(pFile, Buff, nSize, &written, NULL);
	CloseHandle(pFile);
}
