#include "IMirrorCapture.h"
#include <iostream>
#include <stdlib.h>
#include <tchar.h>
#include <winbase.h>
#include <StrSafe.h>

#define  MIRROR_DEVICE_DESC TEXT("Microsoft Mirror Driver")
#define  MIRROR_DEVICE_NAME TEXT("mirror")
#define  MIRROR_SHARE_FILE_NAME TEXT("c:\\video.dat")

IMirrorCapture::IMirrorCapture()
{
}

IMirrorCapture::~IMirrorCapture()
{
}

bool IMirrorCapture::Start()
{
    bool bRet = false;

    do
    {
        // find Primary Display device
        if (!FindPrimaryDisplay())
        {
            std::cout << "FindPrimaryDisplay fail\n";
            break;
        }

        // find mirror display device 
        if (!FindMirrorDisplay())
        {
            std::cout << "FindMirrorDisplay fail\n";
            break;
        }

        // load mirror.dll
        if (!AttachMirror())
        {
            std::cout << "AttachMirror fail\n";
            break;
        }

        if (!GetSharedMemory())
        {
            std::cout << "GetSharedMemory fail\n";
            break;
        }

        // new wait some seconds
        DWORD dwCounter = 0;
        while (dwCounter < 10)
        {
            Sleep(200);
            DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
            DWORD dwHeigth = GetSystemMetrics(SM_CYSCREEN);
            if (dwWidth == m_Width && dwHeigth == m_Heigth)
            {
                bRet = true;
                break;
            }
            dwCounter++;
        }
         
    } while (false);

    return bRet;
}

void IMirrorCapture::Stop()
{
    CloseShareMemory();
    DetachMirror();
}

bool IMirrorCapture::CaptureScreenFrame(char* ScreenBuff, int nSize)
{
    if (nSize != GetScreenSize())
    {
        std::cout << "nSize != GetScreenSize\n";
        return false;
    }

    if (!ScreenBuff || !m_ShareMem)
    {
        std::cout << "ScreenBuff or  m_ShareMem is null\n";
        return false;
    }

    memcpy(ScreenBuff, m_ShareMem, nSize);

#if MIRROR_DEBUG
    m_ImageObj.DumpImage("WinImage", ScreenBuff, m_Width * m_Heigth * 4, m_Width, m_Heigth, 4);
#endif

    return true;
}

bool IMirrorCapture::CaptureWindowFrame(char* WinBuff, RECT& WinRect)
{

    if (!WinBuff || !m_ShareMem)
    {
        std::cout << "ScreenBuff or  m_ShareMem is null\n";
        return false;
    }

    // cut window buffer
    int WinWidth = WinRect.right - WinRect.left;
    int WinHeight = WinRect.bottom - WinRect.top;
    int line = 0;
    int offset = 0;
    for (line = m_Heigth - WinRect.bottom; line < m_Heigth - WinRect.top; line++) {
        offset = line * (m_Width * 4) + WinRect.left * 4;
        memcpy(WinBuff, m_ShareMem + offset, WinWidth * 4);
        WinBuff += WinWidth * 4;
    }

#if MIRROR_DEBUG
    m_ImageObj.DumpImage("WinImage", WinBuff, WinWidth * WinHeight * 4, WinWidth, WinHeight, 4);
#endif

    return true;
}

bool IMirrorCapture::AttachMirror()
{
    int ret = 0;

    do
    {
        this->ResetDevmode(m_MirrorMode);
        m_MirrorMode.dmPelsWidth = m_Width;
        m_MirrorMode.dmPelsHeight = m_Heigth;
        StringCbCopy(m_MirrorMode.dmDeviceName, sizeof(m_MirrorMode.dmDeviceName), MIRROR_DEVICE_NAME);

        // Update the mirror device's registry data with the devmode. Dont
        // do a mode change. 
        ret = ChangeDisplaySettingsEx(m_MirrorDevName, &m_MirrorMode, NULL,
            (CDS_UPDATEREGISTRY | CDS_NORESET), NULL);
        if (ret != DISP_CHANGE_SUCCESSFUL)
        {
            std::cout << "Update Registry on device mode:" << GetDispCode(ret) << std::endl;
            break;
        }
        

        // Now do the real mode change to take mirror driver changes into
        // effect.
        ret = ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
        if (ret != DISP_CHANGE_SUCCESSFUL)
        {
            std::cout << "Raw dynamic mode change on device mode:" << GetDispCode(ret) << std::endl;
            break;
        }

        std::cout << "AttachMirror Success\n";
    } while (false);

    return ret == DISP_CHANGE_SUCCESSFUL;
}

void IMirrorCapture::DetachMirror()
{
    int ret = 0;

    do
    {
        this->ResetDevmode(m_MirrorMode);
        m_MirrorMode.dmPelsWidth = 0;
        m_MirrorMode.dmPelsHeight = 0;
        StringCbCopy(m_MirrorMode.dmDeviceName, sizeof(m_MirrorMode.dmDeviceName), MIRROR_DEVICE_NAME);

        // Update the mirror device's registry data with the devmode. Dont
        // do a mode change. 
        ret = ChangeDisplaySettingsEx(m_MirrorDevName, &m_MirrorMode, NULL,
            (CDS_UPDATEREGISTRY | CDS_NORESET), NULL);
        if (ret != DISP_CHANGE_SUCCESSFUL)
        {
            std::cout << "Update Registry on device mode:" << GetDispCode(ret) << std::endl;
            break;
        }


        // Now do the real mode change to take mirror driver changes into
        // effect.
        ret = ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
        if (ret != DISP_CHANGE_SUCCESSFUL)
        {
            std::cout << "Raw dynamic mode change on device mode:" << GetDispCode(ret) << std::endl;
            break;
        }

        std::cout << "AttachMirror Success\n";
    } while (false);
}

bool IMirrorCapture::FindPrimaryDisplay()
{
    BOOL result = FALSE;
    INT devNum = 0;

    DISPLAY_DEVICE DisplayDev;
    ZeroMemory(&DisplayDev, sizeof(DISPLAY_DEVICE));
    DisplayDev.cb = sizeof(DISPLAY_DEVICE);

    DEVMODE devmode;
    this->ResetDevmode(devmode);
    devmode.dmDeviceName[0] = '\0';

    while ((result = EnumDisplayDevices(NULL, devNum, &DisplayDev, 0)) == TRUE)
    {
        std::cout << "Num: " << devNum
            << "\nDeviceName: " << DisplayDev.DeviceName
            << "\nDriverName: " << DisplayDev.DeviceString
            << "\nDeviceID: " << DisplayDev.DeviceID
            << "\nDeviceKey: " << DisplayDev.DeviceKey << std::endl;

        if (DisplayDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            // Primary device. Find out its dmPelsWidht and dmPelsHeight.
            EnumDisplaySettings(DisplayDev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode);
            this->m_Width = devmode.dmPelsWidth;
            this->m_Heigth = devmode.dmPelsHeight; 
            std::cout << "Primary dmPelsWidth:" << this->m_Width << " dmPelsHeight:" << this->m_Heigth;
            return TRUE;
        }

        devNum++;
    }

    return FALSE;
}

bool IMirrorCapture::FindMirrorDisplay()
{
    BOOL result = FALSE;
    INT devNum = 0;

    DISPLAY_DEVICE DisplayDev;
    ZeroMemory(&DisplayDev, sizeof(DISPLAY_DEVICE));
    DisplayDev.cb = sizeof(DISPLAY_DEVICE);

    while ((result = EnumDisplayDevices(NULL, devNum, &DisplayDev, 0)) == TRUE)
    {
        //std::cout << "Num: " << devNum
        //    << "\nDeviceName: " << DisplayDev.DeviceName
        //    << "\nDriverName: " << DisplayDev.DeviceString
        //    << "\nDeviceID: " << DisplayDev.DeviceID
        //    << "\nDeviceKey: " << DisplayDev.DeviceKey << std::endl;

        if (lstrcmp(DisplayDev.DeviceString, MIRROR_DEVICE_DESC) == 0)
        {
            lstrcpy(m_MirrorDevName, DisplayDev.DeviceName);
            return TRUE;
        }
        devNum++;
    }

    return FALSE;
}


void IMirrorCapture::ResetDevmode(DEVMODE& DevMode)
{
    ZeroMemory(&DevMode, sizeof(DEVMODE));
    DevMode.dmSize = sizeof(DEVMODE);
    DevMode.dmDriverExtra = 0;
    DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;
}


bool IMirrorCapture::GetSharedMemory()
{
    bool bRet = false;
    PCHAR pVideoMemory = nullptr;
    HANDLE hMapFile = INVALID_HANDLE_VALUE, hFile = INVALID_HANDLE_VALUE;

    do
    {
        hFile = CreateFile(MIRROR_SHARE_FILE_NAME, GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (!hFile || hFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "OpenFile " << MIRROR_SHARE_FILE_NAME << " fail:" << GetLastError() << std::endl;
            break;
        }

        hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
        if (!hMapFile || hMapFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "CreateFileMapping fail:" << GetLastError() << std::endl;
            break;
        }

        m_ShareMem = (PCHAR)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, GetScreenSize());
        if (!m_ShareMem)
        {
            std::cout << "MapViewOfFile fail, Please Check GetScreenSize()\n";
            break;
        }

        bRet = true;
    } while (false);

    if (hMapFile && hMapFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hMapFile);
        hMapFile = INVALID_HANDLE_VALUE;
    }
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return bRet;
}

void IMirrorCapture::CloseShareMemory()
{
    if (m_ShareMem)
    {
        UnmapViewOfFile(m_ShareMem);
    }   
}

DWORD IMirrorCapture::GetScreenSize()
{
    return m_Width * m_Heigth * 4;
}


char* IMirrorCapture::GetDispCode(int code)
{
#define case_txtErrCode(xx) case xx: return #xx;
    switch (code)
    {
        case_txtErrCode(DISP_CHANGE_SUCCESSFUL);
        case_txtErrCode(DISP_CHANGE_RESTART);
        case_txtErrCode(DISP_CHANGE_BADFLAGS);
        case_txtErrCode(DISP_CHANGE_BADPARAM);
        case_txtErrCode(DISP_CHANGE_FAILED);
        case_txtErrCode(DISP_CHANGE_BADMODE);
        case_txtErrCode(DISP_CHANGE_NOTUPDATED);
    default:
        static char tmp[MAX_PATH];
        StringCbPrintfA(&tmp[0], sizeof(tmp), "Unknown code: %08x\n", code);
        return tmp;   
    }
#undef case_txtErrCode
}

void MirrorScreenExample()
{
    IMirrorCapture MirrorCap;

    if (!MirrorCap.Start())
    {
        std::cout << "MirrorCap.Start fail\n";
    }

    DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD dwHeigth = GetSystemMetrics(SM_CYSCREEN);
    DWORD dwBuffSize = dwWidth * dwHeigth * 4;
    PCHAR ScreenBuff = new CHAR[dwBuffSize];

    DWORD Counter = 0;
    while (Counter < 10) {
        if (!MirrorCap.CaptureScreenFrame(ScreenBuff, dwBuffSize))
        {
            std::cout << "MirrorCap.CaptureScreenFrame fail\n";
        }
        Sleep(33);
        Counter++;
    }
    MirrorCap.Stop();
}

void MirrorWinExample()
{
    IMirrorCapture MirrorCap;

    if (!MirrorCap.Start())
    {
        std::cout << "MirrorCap.Start fail\n";
    }

    DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD dwHeigth = GetSystemMetrics(SM_CYSCREEN);
    DWORD dwBuffSize = dwWidth * dwHeigth * 4;
    PCHAR ScreenBuff = new CHAR[dwBuffSize];

    HWND hwnd = FindWindowEx(NULL, NULL, TEXT("Progman"), TEXT("Program Manager"));
    if (!hwnd)
    {
        printf("FindWindow fail\n");
        return;
    }
    ShowWindow(hwnd, SW_SHOW);

    RECT rect = { 0 };
    DWORD Counter = 0;
    while (Counter < 10) {
        if (!GetWindowRect(hwnd, &rect))
        {
            std::cout << "GetWindowRect fail\n";
            break;
        }

        if (!MirrorCap.CaptureWindowFrame(ScreenBuff, rect))
        {
            std::cout << "MirrorCap.CaptureWindowFrame fail\n";
        }
        Sleep(33);
        Counter++;
    }
    MirrorCap.Stop();
}
