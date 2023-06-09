#pragma once
#include <windows.h>
#include <winddi.h>
#define MIRROR_DEBUG 1
#if MIRROR_DEBUG
#include "CImageFile.h"
#endif

class IMirrorCapture
{
public:
	IMirrorCapture();
	~IMirrorCapture();
	bool Start();
	void Stop();
	bool CaptureScreenFrame(char* ScreenBuff,int nSize);
	bool CaptureWindowFrame(char* WinBuff, RECT& WinRect);
protected:
	bool AttachMirror();
	void DetachMirror();
	bool FindPrimaryDisplay();
	bool FindMirrorDisplay();
	void ResetDevmode(DEVMODE& DevMode);
	bool GetSharedMemory();
	void CloseShareMemory();
	DWORD GetScreenSize();
	char* GetDispCode(int code);

private:
	DEVMODE m_MirrorMode = { 0 };
	DWORD m_Width = 0;
	DWORD m_Heigth = 0;
	DISPLAY_DEVICE m_MirrorDev = {0};
	TCHAR m_MirrorDevName[100] = { 0 };
	PCHAR m_ShareMem = nullptr;
#if MIRROR_DEBUG
	CImageFile m_ImageObj;
#endif
};

void MirrorScreenExample();
void MirrorWinExample();
