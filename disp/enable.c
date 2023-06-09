/******************************Module*Header*******************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: enable.c
*
* This module contains the functions that enable and disable the
* driver, the pdev, and the surface.
*
* Copyright (c) 1992-1998 Microsoft Corporation
\**************************************************************************/
#define DBG 1
#include "driver.h"

// The driver function table with all function index/address pairs

static DRVFN gadrvfn[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvNotify,                (PFN) DrvNotify             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo             },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },
    {   INDEX_DrvEscape,                (PFN) DrvEscape             }
};

//
// always hook these routines to ensure the mirrored driver
// is called for our surfaces
//

#define flGlobalHooks   HOOK_FILLPATH | HOOK_STROKEPATH | HOOK_LINETO | HOOK_TEXTOUT | HOOK_BITBLT | HOOK_COPYBITS
// Define the functions you want to hook for 8/16/24/32 pel formats

#define HOOKS_BMF8BPP 0
#define HOOKS_BMF16BPP 0
#define HOOKS_BMF24BPP 0
#define HOOKS_BMF32BPP 0

/******************************Public*Routine******************************\
* DrvEnableDriver
*
* Enables the driver by retrieving the drivers function table and version.
*
\**************************************************************************/

BOOL DrvEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded)
{
// Engine Version is passed down so future drivers can support previous
// engine versions.  A next generation driver can support both the old
// and new engine conventions if told what version of engine it is
// working with.  For the first version the driver does nothing with it.

    iEngineVersion;

    DISPDBG((0,"DrvEnableDriver:\n"));

// Fill in as much as we can.

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = sizeof(gadrvfn) / sizeof(DRVFN);

// DDI version this driver was targeted for is passed back to engine.
// Future graphic's engine may break calls down to old driver format.

    if (cj >= sizeof(ULONG))
	// DDI_DRIVER_VERSION is now out-dated. See winddi.h
	// DDI_DRIVER_VERSION_NT4 is equivalent to the old DDI_DRIVER_VERSION
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

    return(TRUE);
}

/******************************Public*Routine******************************\
* DrvEnablePDEV
*
* DDI function, Enables the Physical Device.
*
* Return Value: device handle to pdev.
*
\**************************************************************************/

DHPDEV
DrvEnablePDEV(
    __in     DEVMODEW   *pDevmode,                // Pointer to DEVMODE
    __in_opt PWSTR       pwszLogAddress,          // Logical address
    __in     ULONG       cPatterns,               // number of patterns
    __in_opt HSURF      *ahsurfPatterns,          // return standard patterns
    __in     ULONG       cjGdiInfo,               // Length of memory pointed to by pGdiInfo
    __out_bcount(cjGdiInfo)  ULONG    *pGdiInfo,  // Pointer to GdiInfo structure
    __in     ULONG       cjDevInfo,               // Length of following PDEVINFO structure
    __out_bcount(cjDevInfo)  DEVINFO  *pDevInfo,  // physical device information structure
    __in_opt HDEV        hdev,                    // HDEV, used for callbacks
    __in_opt PWSTR       pwszDeviceName,          // DeviceName - not used
    __in     HANDLE      hDriver                  // Handle to base driver
    )
{
    GDIINFO GdiInfo;
    DEVINFO DevInfo;
    PPDEV   ppdev = (PPDEV) NULL;

    DISPDBG((0,"DrvEnablePDEV:\n"));

    UNREFERENCED_PARAMETER(pwszLogAddress);
    UNREFERENCED_PARAMETER(cPatterns);
    UNREFERENCED_PARAMETER(ahsurfPatterns);
    UNREFERENCED_PARAMETER(hdev);
    UNREFERENCED_PARAMETER(pwszDeviceName);

    // Allocate a physical device structure.
    ppdev = (PPDEV) EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);
    if (ppdev == (PPDEV) NULL)
    {
        RIP("DISP DrvEnablePDEV failed EngAllocMem\n");
        return((DHPDEV) 0);
    }

    // Save the screen handle in the PDEV.

    ppdev->hDriver = hDriver;

    // Get the current screen mode information.  Set up device caps and devinfo.

    if (!bInitPDEV(ppdev, pDevmode, &GdiInfo, &DevInfo))
    {
        DISPDBG((0,"DISP DrvEnablePDEV failed\n"));
        goto error_free;
    }
    
    // Copy the devinfo into the engine buffer.

    // memcpy(pDevInfo, &DevInfo, min(sizeof(DEVINFO), cjDevInfo));
    if (sizeof(DEVINFO) > cjDevInfo)
    {
        DISPDBG((0,"DISP DrvEnablePDEV failed: insufficient pDevInfo memory\n"));
        goto error_free;
    }
	RtlCopyMemory(pDevInfo, &DevInfo, sizeof(DEVINFO));

    // Set the pdevCaps with GdiInfo we have prepared to the list of caps for this
    // pdev.

    //memcpy(pGdiInfo, &GdiInfo, min(cjGdiInfo, sizeof(GDIINFO)));
    if (sizeof(GDIINFO) > cjGdiInfo)
    {
        DISPDBG((0,"DISP DrvEnablePDEV failed: insufficient pDevInfo memory\n"));
        goto error_free;
    }
	RtlCopyMemory(pGdiInfo, &GdiInfo, sizeof(GDIINFO));

    return((DHPDEV) ppdev);

    // Error case for failure.
error_free:
    EngFreeMem(ppdev);
    return((DHPDEV) 0);
}

/******************************Public*Routine******************************\
* DrvCompletePDEV
*
* Store the HPDEV, the engines handle for this PDEV, in the DHPDEV.
*
\**************************************************************************/

VOID DrvCompletePDEV(
DHPDEV dhpdev,
HDEV  hdev)
{
    DISPDBG((1,"DrvCompletePDEV:\n"));
    ((PPDEV) dhpdev)->hdevEng = hdev;
}

/******************************Public*Routine******************************\
* DrvDisablePDEV
*
* Release the resources allocated in DrvEnablePDEV.  If a surface has been
* enabled DrvDisableSurface will have already been called.
*
\**************************************************************************/

VOID DrvDisablePDEV(
DHPDEV dhpdev)
{
   PPDEV ppdev = (PPDEV) dhpdev;

   DISPDBG((1,"DrvDisablePDEV:\n"));

   EngDeletePalette(ppdev->hpalDefault);
   EngFreeMem(dhpdev);
   EngUnmapFile(ppdev->pMappedFile);
   EngDeleteFile(L"\\??\\c:\\video.dat");
}

/******************************Public*Routine******************************\
* DrvEnableSurface
*
* Enable the surface for the device.  Hook the calls this driver supports.
*
* Return: Handle to the surface if successful, 0 for failure.
*
\**************************************************************************/

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PPDEV ppdev;
    HSURF hsurf;
    SIZEL sizl;
    ULONG ulBitmapType;
    FLONG flHooks;
    ULONG mirrorsize;
    MIRRSURF *mirrsurf;
    DHSURF dhsurf;
    ULONG BitsPerPel;

    // Create engine bitmap around frame buffer.

    DISPDBG((0,"DrvEnableSurface:\n"));

    ppdev = (PPDEV) dhpdev;

    ppdev->ptlOrg.x = 0;
    ppdev->ptlOrg.y = 0;

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    if (ppdev->ulBitCount == 16)
    {
        ulBitmapType = BMF_16BPP;
        flHooks = HOOKS_BMF16BPP;
        BitsPerPel = 2;
    }
    else if (ppdev->ulBitCount == 24)
    {
        ulBitmapType = BMF_24BPP;
        flHooks = HOOKS_BMF24BPP;
        BitsPerPel = 3;
    }
    else
    {
        ulBitmapType = BMF_32BPP;
        flHooks = HOOKS_BMF32BPP;
        BitsPerPel = 4;
    }
    
    flHooks |= flGlobalHooks;

    
    mirrorsize = (ULONG)(ppdev->cxScreen * ppdev->cyScreen * BitsPerPel);
    ppdev->pvTmpBuffer = EngMapFile(L"\\??\\c:\\video.dat", mirrorsize,&ppdev->pMappedFile);
    hsurf = (HSURF) EngCreateBitmap(sizl,ppdev->lDeltaScreen,ulBitmapType,0,(PVOID)(ppdev->pvTmpBuffer));
    if (hsurf == (HSURF) 0)
    {
        RIP("DISP DrvEnableSurface failed EngCreateBitmap\n");
        return(FALSE);
    }

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, flHooks))
    {
        RIP("DISRP DrvEnableSurface failed EngAssociateSurface\n");
        EngDeleteSurface(hsurf);
        return(FALSE);
    }

    return(hsurf);
}

/******************************Public*Routine******************************\
* DrvNotify
*
* Receives notification on where the mirror driver is positioned.
* Also gets notified before drawing happens 
*
\**************************************************************************/

VOID DrvNotify(
SURFOBJ *pso,
ULONG iType,
PVOID pvData)
{
    UNREFERENCED_PARAMETER(pso);
    UNREFERENCED_PARAMETER(pvData);

    switch(iType)
    {
        case DN_DEVICE_ORIGIN:
            DISPDBG((0,"DrvNotify: DN_DEVICE_ORIGIN (%d,%d)\n", ((POINTL*)pvData)->x, ((POINTL*)pvData)->y));
            break;
        case DN_DRAWING_BEGIN:
            DISPDBG((0,"DrvNotify: DN_DRAWING_BEGIN\n"));
            break;
    }
}

/******************************Public*Routine******************************\
* DrvDisableSurface
*
* Free resources allocated by DrvEnableSurface.  Release the surface.
*
\**************************************************************************/

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    DISPDBG((0,"DrvDisableSurface:\n"));

    EngDeleteSurface( ppdev->hsurfEng );
    
    // deallocate MIRRSURF structure.

    //EngFreeMem( ppdev->pvTmpBuffer );
}

/******************************Public*Routine******************************\
* DrvCopyBits
*
\**************************************************************************/

BOOL DrvCopyBits(
   OUT SURFOBJ *psoDst,
   IN SURFOBJ *psoSrc,
   IN CLIPOBJ *pco,
   IN XLATEOBJ *pxlo,
   IN RECTL *prclDst,
   IN POINTL *pptlSrc
   )
{
    return DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc,NULL, NULL, NULL, ROP4_SRCCOPY);
}

/******************************Public*Routine******************************\
* DrvBitBlt
*
\**************************************************************************/

BOOL DrvBitBlt(
   IN SURFOBJ *psoDst,
   IN SURFOBJ *psoSrc,
   IN SURFOBJ *psoMask,
   IN CLIPOBJ *pco,
   IN XLATEOBJ *pxlo,
   IN RECTL *prclDst,
   IN POINTL *pptlSrc,
   IN POINTL *pptlMask,
   IN BRUSHOBJ *pbo,
   IN POINTL *pptlBrush,
   IN ROP4 rop4
   )
{
    return EngBitBlt(psoDst, psoSrc, psoMask, pco, pxlo, prclDst, pptlSrc, pptlMask, pbo, pptlBrush, rop4);
  
}

BOOL DrvTextOut(
   IN SURFOBJ *psoDst,
   IN STROBJ *pstro,
   IN FONTOBJ *pfo,
   IN CLIPOBJ *pco,
   IN RECTL *prclExtra,
   IN RECTL *prclOpaque,
   IN BRUSHOBJ *pboFore,
   IN BRUSHOBJ *pboOpaque,
   IN POINTL *pptlOrg,
   IN MIX mix
   )
{
   return EngTextOut(psoDst, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlOrg, mix);
}

BOOL
DrvStrokePath(SURFOBJ*   pso,
              PATHOBJ*   ppo,
              CLIPOBJ*   pco,
              XFORMOBJ*  pxo,
              BRUSHOBJ*  pbo,
              POINTL*    pptlBrush,
              LINEATTRS* pLineAttrs,
              MIX        mix)
{
    
    return EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrush, pLineAttrs, mix);
}

BOOL DrvLineTo(
SURFOBJ   *pso,
CLIPOBJ   *pco,
BRUSHOBJ  *pbo,
LONG       x1,
LONG       y1,
LONG       x2,
LONG       y2,
RECTL     *prclBounds,
MIX        mix)
{
   
    return EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
}



BOOL DrvFillPath(
 SURFOBJ  *pso,
 PATHOBJ  *ppo,
 CLIPOBJ  *pco,
 BRUSHOBJ *pbo,
 PPOINTL   pptlBrushOrg,
 MIX       mix,
 FLONG     flOptions)
{
    return EngFillPath(pso, ppo,pco, pbo, pptlBrushOrg, mix, flOptions);
}

/******************************Public*Routine******************************\
* DrvAssertMode
*
* Enable/Disable the given device.
*
\**************************************************************************/

DrvAssertMode(DHPDEV  dhpdev,
              BOOL    bEnable)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    UNREFERENCED_PARAMETER(bEnable);
    UNREFERENCED_PARAMETER(ppdev);

    DISPDBG((0, "DrvAssertMode(%lx, %lx)\n", dhpdev, bEnable));

    return TRUE;

}// DrvAssertMode()


ULONG
DrvEscape(SURFOBJ *pso,
          ULONG iEsc,
          ULONG cjIn,
          PVOID pvIn,
          ULONG cjOut,
          PVOID pvOut)
{
    ULONG ulRet = 0;

    UNREFERENCED_PARAMETER(cjIn);
    UNREFERENCED_PARAMETER(pvIn);
    UNREFERENCED_PARAMETER(cjOut);
    UNREFERENCED_PARAMETER(pvOut);


    return ulRet;
}


void DrvMovePointer(SURFOBJ  *pso, LONG  x, LONG  y, RECTL  *prcl)
{

    DISPDBG((1, "DrvMovePointer1 (%ld, %ld)\n" ,x,y));
    
}


ULONG DrvSetPointerShape(SURFOBJ  *pso, SURFOBJ  *psoMask, SURFOBJ  *psoColor, XLATEOBJ  *pxlo, LONG  xHot, LONG  yHot, LONG  x, LONG  y, RECTL  *prcl, FLONG  fl)
{

    DISPDBG((1, "DrvSetPointerShape1 (%ld, %ld) (%ld, %ld)\n" ,x,y,yHot, yHot));
    return SPS_ACCEPT_NOEXCLUDE;
}


