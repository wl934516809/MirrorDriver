//#include <vld.h>
//#include <vldapi.h>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "IMirrorCapture.h"
#pragma warning(disable:4996)

int main() {

    IMirrorCapture MirrorCap;

    if (!MirrorCap.Start())
    {
        std::cout << "MirrorCap.Start fail\n";
    }

    Sleep(2000);
    DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD dwHeigth = GetSystemMetrics(SM_CYSCREEN);
    DWORD dwBuffSize = dwWidth * dwHeigth * 4;
    PCHAR ScreenBuff = new CHAR[dwBuffSize];
    PCHAR CurrentBuff = new CHAR[dwBuffSize];

    std::cout << "Desktop dwWidth:" << dwWidth << " dwHeigth:" << dwHeigth;

    DWORD fps = 0;
    DWORD Counter = 0;
    while (Counter < 200) {
        if (!MirrorCap.CaptureScreenFrame(ScreenBuff, dwBuffSize))
        {
            std::cout << "MirrorCap.CaptureScreenFrame fail\n";
        }

        if (memcmp(ScreenBuff, CurrentBuff, dwBuffSize) != 0)
        {
            fps++;
        }
        memcpy(CurrentBuff, ScreenBuff, dwBuffSize);
        Sleep(10);
        Counter++;
    }

    printf("fps = %d\n", fps);
    MirrorCap.Stop();
    return 0;
}