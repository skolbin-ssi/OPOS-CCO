/****************************************************************************
**
** RemoteOrderDisplayImpl.cpp -- Main control implementation file.
**
**     Date                   Modification                          Author
** -----------|----------------------------------------------------|----------
**  1999/03/20 Initial development.                                 C. Monroe
**  2003/05/29 On Debug builds, add tracing of Open and Close to
**               C:\OposCCO_RemoteOrderDisplay.log
**  2004/03/22 Add significantly more tracing when Debug.
**  2004/10/26 Minor enhancement of Debug tracing of strings.
**  2005/12/14 Add Debug tracing of event delivery.
**  2008/01/15 Add more parameter and error checking.
**  2013/09/29 Update license section.
**  2015/02/07 Update license section.
**             Use OposVariantResult for Invoke results.
**             Correct possible buffer overflow in Open.
**               Updates to copies and printf to remove any
**               potential buffer overflows.
**  2015/02/07 Version 1.14.001.
**  2021/12/02 Version 1.16.000.                                    K. Fukuchi
**
*****************************************************************************
**
{{Begin License}}

Copyright 2015 Monroe Consulting Services, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

{{End License}}
*/


/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

// Include the standard header.
#include "stdafx.h"

// Include the MIDL-generated header.
#include "RemoteOrderDisplay.h"

// Include this control's header.
#include "RemoteOrderDisplayImpl.h"

// Include character <--> wide conversions header.
#include "CWString.h"

// Include the OPOS header.
#include "OposRod.hi"

// TSIZEOF(x) returns characters in "x" bytes.
#define TSIZEOF(x)  (sizeof(x)/sizeof(TCHAR))

// Ensure that character array has NUL as last character.
#define NULTERMINATE(x) x[TSIZEOF(x) - 1] = '\0';


/////////////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////////////

// Hidden window class and window name.
TCHAR g_WindowName[] = _T("HW_OPOSRemoteOrderDisplay");

// Control Object description and version.
char g_ControlDescription[] = "OPOS RemoteOrderDisplay Control 1.16.000 [Public, by CRM/MCS, and modified by KF]" IFDEBUG(" [With Tracing]");
LONG g_ControlVersion =
    1 * 1000000 +
    16 * 1000 +
    000;

// Count of instances within this process.
int COPOSRemoteOrderDisplay::s_nInstances = 0;

//===========================================================================
// Method name table. Used to lookup methods in SO.
//===========================================================================

static char* s_SOMethodNames[SO_DISP_COUNT + 1] =
{
// Release 1.3 methods
    "COFreezeEvents",
        #define nDICOFreezeEvents 0
    "GetPropertyNumber",
        #define nDIGetPropertyNumber 1
    "SetPropertyNumber",
        #define nDISetPropertyNumber 2
    "GetPropertyString",
        #define nDIGetPropertyString 3
    "SetPropertyString",
        #define nDISetPropertyString 4
    "OpenService",
        #define nDIOpenService 5
    "CheckHealth",
        #define nDICheckHealth 6
    "ClaimDevice",
        #define nDIClaimDevice 7
    "ClearInput",
        #define nDIClearInput 8
    "ClearOutput",
        #define nDIClearOutput 9
    "CloseService",
        #define nDICloseService 10
    "DirectIO",
        #define nDIDirectIO 11
    "ReleaseDevice",
        #define nDIReleaseDevice 12
    "ClearVideo",
        #define nDIClearVideo 13
    "ClearVideoRegion",
        #define nDIClearVideoRegion 14
    "ControlClock",
        #define nDIControlClock 15
    "ControlCursor",
        #define nDIControlCursor 16
    "CopyVideoRegion",
        #define nDICopyVideoRegion 17
    "DisplayData",
        #define nDIDisplayData 18
    "DrawBox",
        #define nDIDrawBox 19
    "FreeVideoRegion",
        #define nDIFreeVideoRegion 20
    "ResetVideo",
        #define nDIResetVideo 21
    "RestoreVideoRegion",
        #define nDIRestoreVideoRegion 22
    "SaveVideoRegion",
        #define nDISaveVideoRegion 23
    "SelectCharacterSet",
        #define nDISelectCharacterSet 24
    "SetCursor",
        #define nDISetCursor 25
    "TransactionDisplay",
        #define nDITransactionDisplay 26
    "UpdateVideoRegionAttribute",
        #define nDIUpdateVideoRegionAttribute 27
    "VideoSound",
        #define nDIVideoSound 28
// Release 1.4 methods
// Release 1.5 methods
// Release 1.6 methods
// Release 1.7 methods
// Release 1.8 methods
    "ResetStatistics",
        #define nDIResetStatistics 29
    "RetrieveStatistics",
        #define nDIRetrieveStatistics 30
    "UpdateStatistics",
        #define nDIUpdateStatistics 31
// Release 1.9 methods
    "CompareFirmwareVersion",
        #define nDICompareFirmwareVersion 32
    "UpdateFirmware",
        #define nDIUpdateFirmware 33
    0
};

//===========================================================================
// Required method count table.
//   Contains count of methods from table above that must be present
//   for each release.
// Followed by the initial minor release of the OPOS RemoteOrderDisplay.
//===========================================================================

static int s_RequiredMethodsPerRelease[] =
{
     29, // Release 1.3
     29, // Release 1.4
     29, // Release 1.5
     29, // Release 1.6
     29, // Release 1.7
     32, // Release 1.8
     34  // Release 1.9
};

const int MinorReleaseInitial = 3;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Constructor / Destructor
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

COPOSRemoteOrderDisplay::COPOSRemoteOrderDisplay()
    IFDEBUG( : CProxy_IOPOSEvents< COPOSRemoteOrderDisplay >(this) )
{
}

//===========================================================================

HRESULT COPOSRemoteOrderDisplay::FinalConstruct()
{
    TRACEINIT;
    DOTRACE( ( _T("") ) );
    DOTRACE( ( _T("*Constructing: %hs"), g_ControlDescription ) );

    _bOpened = false;
    _nOpenResult = 0;
    _nSOMajor = 0;
    _nSOMinor = 0;
    _nCOResultCode = -1;
    _nCOFreezeEventCount = 0;
    _bFreezeState = false;
    IFDEBUG( _nBinaryConversion = OPOS_BC_NONE );

    HRESULT hRC = EventInit();

    s_nInstances++; // Increment instance count at end of FinalConstruct.

    return hRC;
}

//===========================================================================

COPOSRemoteOrderDisplay::~COPOSRemoteOrderDisplay()
{
    DOTRACE( ( _T("*Destructing") ) );

    s_nInstances--; // Decrement instance count at start of destructor.
    long RC;
    if ( _bOpened )
        Close( &RC );
    EventUninit();

    DOTRACE( ( _T("") ) );
    TRACEEXIT;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Control handlers
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Override the IOleControlImpl handler for container request to freeze/unfreeze events.
STDMETHODIMP COPOSRemoteOrderDisplay::FreezeEvents( BOOL bFreeze )
{
    // If requesting events to be frozen...
    if ( bFreeze )
    {
        DOTRACEV( ( _T("*FreezeEvents: Freezing %d"), _nCOFreezeEventCount ) );
        if ( _nCOFreezeEventCount == 0 &&   // If events are not currently frozen and
             _bOpened )                     //   SO is opened,
            SetCOFreezeEvents( TRUE );      //   then freeze events in the SO.
        _nCOFreezeEventCount++;             // Increment freeze count. Must do after informing SO.
    }
    // Else requesting events to be unfrozen...
    else
    {
        _nCOFreezeEventCount--;             // Decrement freeze count. Must do before informing SO.
        DOTRACEV( ( _T("*FreezeEvents: Unfreezing %d"), _nCOFreezeEventCount ) );
        if ( _nCOFreezeEventCount == 0 &&   // If events have become unfrozen and
             _bOpened )                     //   SO is opened,
            SetCOFreezeEvents( FALSE );     //   then unfreeze events in the SO.
    }

    // Just in case anyone cares ... keep the CComControlBase one in sync.
    m_nFreezeEvents = _nCOFreezeEventCount;
    return S_OK;
}

//===========================================================================

// Implement the draw handler.
HRESULT COPOSRemoteOrderDisplay::OnDraw( ATL_DRAWINFO& di )
{
    RECT& rc = *(RECT*)di.prcBounds;

    // Draw our bitmap...
    //  - Load bitmap and get info
    HBITMAP hBitmap = LoadBitmap(
        _Module.GetModuleInstance(),            // handle of the instance containing the image
        MAKEINTRESOURCE(IDB_OPOSREMOTEORDERDISPLAY2) ); // bitmap resource name
    BITMAP bmp;
    GetObject(
        hBitmap,                                // handle to graphics object of interest
        sizeof(BITMAP),                         // size of buffer for object information
        &bmp );                                 // pointer to buffer for object information
    //  - Create a compatible memory DC and select bitmap into it
    HDC hMemDC = CreateCompatibleDC(
        di.hdcDraw );                           // handle to the device context
    HBITMAP hOldBitmap =
        (HBITMAP) SelectObject(
            hMemDC,                             // handle to device context
            hBitmap );                          // handle to object
    //  - Copy bitmap from memory DC to screen DC
    BitBlt(
      di.hdcDraw,                               // handle to destination device context
      rc.left,                                  // x-coordinate of destination rectangle's upper-left corner
      rc.top,                                   // y-coordinate of destination rectangle's upper-left corner
      min( bmp.bmWidth, rc.right - rc.left ),   // width of destination rectangle
      min( bmp.bmHeight, rc.bottom - rc.top ),  // height of destination rectangle
      hMemDC,                                   // handle to source device context
      0,                                        // x-coordinate of source rectangle's upper-left corner
      0,                                        // y-coordinate of source rectangle's upper-left corner
      SRCCOPY );                                // raster operation code

    // Cleanup
    SelectObject( hMemDC, hOldBitmap );
    DeleteObject( hMemDC );
    DeleteObject( hBitmap );

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Property Access / Freeze Setting
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

HRESULT COPOSRemoteOrderDisplay::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, long* pnValue, long nMinor )
{
    DOTRACEV( ( _T("+%hs [Get]"), pPropName ) );

    // Initialize return value.
    HRESULT hRC = S_OK;
    *pnValue = 0;

    // If open and SO version support's property...
    if ( _bOpened && _nSOMinor >= nMinor )
    {
        // Set up and call the SO's get property number method.
        OposVariant Var;
        OposVariantResult VarResult;
        Var.SetLONG( nIndex );
        DISPPARAMS Disp = { &Var, NULL, 1, 0 };
        hRC = _pService->Invoke(
                _DispIDs[nDIGetPropertyNumber], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );

        // If success, then set return result.
        if ( hRC == S_OK )
            VarResult.ChangeLONG( pnValue, hRC );
    }

    // Trace result.
#ifdef _DEBUG
    LPCTSTR pszInfo = _T("");
    TCHAR szHR[50];
    if ( !_bOpened )
        pszInfo = _T(" -- Closed");
    else if ( _nSOMinor < nMinor )
        pszInfo = _T(" -- Not supported by SO");
    else if ( hRC != 0 )
    {
        _sntprintf( szHR, TSIZEOF(szHR), _T(" -- COM error: HR = 0x%X"), hRC );
        NULTERMINATE(szHR);
        pszInfo = szHR;
    }

    DOTRACEV( ( _T("-%hs [Get]: %d (0x%X)%s"), pPropName, *pnValue, *pnValue, pszInfo ) );
#endif

    return hRC;
}

//---------------------------------------------------------------------------

HRESULT COPOSRemoteOrderDisplay::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, long nValue, long nMinor )
{
    DOTRACEV( ( _T("+%hs [Set]: %d (0x%X)"), pPropName, nValue, nValue ) );
    HRESULT hRC = S_OK;

    SetRC();                            // Clear COResultCode.

    // If open and SO version support's property...
    if ( _bOpened && _nSOMinor >= nMinor )
    {
        // Set up and call the SO's set property number method.
        OposVariant Vars[2];
        OposVariantResult VarResult;
        Vars[1].SetLONG( nIndex );
        Vars[0].SetLONG( nValue );
        OposDispParms Disp( Vars, 2 );
        hRC = _pService->Invoke(
                _DispIDs[nDISetPropertyNumber], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );
    }

    // Handle error.
    else if ( _bOpened )                // If open but not supported,
        SetRC( OPOS_E_NOSERVICE );      //   set COResultCode.

    // Trace result.
#ifdef _DEBUG
    LPCTSTR pszInfo = _T("");
    TCHAR szHR[50];
    if ( !_bOpened )
        pszInfo = _T(" -- Closed");
    else if ( _nSOMinor < nMinor )
        pszInfo = _T(" -- Not supported by SO");
    else if ( hRC != 0 )
    {
        _sntprintf( szHR, TSIZEOF(szHR), _T(" -- COM error: HR = 0x%X"), hRC );
        NULTERMINATE(szHR);
        pszInfo = szHR;
    }

    DOTRACEV( ( _T("-%hs [Set]%s"), pPropName, pszInfo ) );
#endif

    return hRC;
}

//===========================================================================

HRESULT COPOSRemoteOrderDisplay::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, VARIANT_BOOL* pnValue, long nMinor )
{
    // If getting a boolean, then force return value to proper VARIANT_BOOL value.
    long nValue;
    HRESULT hRC = GetOposProp( DEBUGPARAM(pPropName)
        nIndex, &nValue, nMinor );
    *pnValue = nValue ? VARIANT_TRUE : VARIANT_FALSE;
    return hRC;
}

//---------------------------------------------------------------------------

HRESULT COPOSRemoteOrderDisplay::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, VARIANT_BOOL nValue, long nMinor )
{
    // If setting a boolean, then force to use VC++ TRUE or FALSE,
    //   since several vendors test specifically against TRUE (1)
    //   rather than non-zero.
    return SetOposProp( DEBUGPARAM(pPropName)
        nIndex, static_cast<long>(nValue ? TRUE : FALSE), nMinor );
}

//===========================================================================

HRESULT COPOSRemoteOrderDisplay::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, BSTR* psValue, long nMinor )
{
    DOTRACEV( ( _T("+%hs [Get]"), pPropName ) );

    // Initialize return value.
    HRESULT hRC = S_OK;

    // If open and SO version support's property...
    if ( _bOpened && _nSOMinor >= nMinor )
    {
        // Set up and call the SO's get property number method.
        OposVariant Var;
        OposVariantResult VarResult;
        Var.SetLONG( nIndex );
        DISPPARAMS Disp = { &Var, NULL, 1, 0 };
        hRC = _pService->Invoke(
                _DispIDs[nDIGetPropertyString], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );

        // If succeeds, then return result.
        if ( hRC == S_OK )
            VarResult.ChangeBSTR( psValue, hRC );
    }
    // Else return default string.
    else
        *psValue = ::SysAllocString( L"[Error]" );

    // Trace result.
#ifdef _DEBUG
    LPCTSTR pszFormat = _T("-%hs [Get].");
    if ( !_bOpened )
        pszFormat = _T("-%hs [Get] -- Closed.");
    else if ( _nSOMinor < nMinor )
        pszFormat = _T("-%hs [Get] -- Not supported by SO.");
    else if ( hRC != 0 )
        pszFormat = _T("-%hs [Get] -- COM error: HR = 0x%X");

    TCHAR szPrefix[100];
    _sntprintf( szPrefix, TSIZEOF(szPrefix), pszFormat, pPropName, hRC );
    NULTERMINATE(szPrefix);
    DOTRACESTRINGV( szPrefix, *psValue );
#endif

    return hRC;
}

//---------------------------------------------------------------------------

HRESULT COPOSRemoteOrderDisplay::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, BSTR sValue, long nMinor )
{
#ifdef _DEBUG
    TCHAR szPrefix[100];
    _sntprintf( szPrefix, TSIZEOF(szPrefix), _T("+%hs [Set]."), pPropName );
    NULTERMINATE(szPrefix);
    DOTRACESTRINGV( szPrefix, sValue );
#endif
    HRESULT hRC = S_OK;

    SetRC();                            // Clear COResultCode.

    // If open and SO version support's property...
    if ( _bOpened && _nSOMinor >= nMinor )
    {
        // Set up and call the SO's set property number method.
        OposVariant Vars[2];
        OposVariantResult VarResult;
        Vars[1].SetLONG( nIndex );
        Vars[0].SetBSTR( sValue, hRC );
        OposDispParms Disp( Vars, 2 );
        if ( hRC == S_OK )
            hRC = _pService->Invoke(
                _DispIDs[nDISetPropertyString], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );
    }

    // Handle error.
    else if ( _bOpened )                // If open but not supported,
        SetRC( OPOS_E_NOSERVICE );      //   set COResultCode.

    // Trace result.
#ifdef _DEBUG
    LPCTSTR pszInfo = _T("");
    TCHAR szHR[50];
    if ( !_bOpened )
        pszInfo = _T(" -- Closed");
    else if ( _nSOMinor < nMinor )
        pszInfo = _T(" -- Not supported by SO");
    else if ( hRC != 0 )
    {
        _sntprintf( szHR, TSIZEOF(szHR), _T(" -- COM error: HR = 0x%X"), hRC );
        NULTERMINATE(szHR);
        pszInfo = szHR;
    }

    DOTRACEV( ( _T("-%hs [Set]%s"), pPropName, pszInfo ) );
#endif

    return hRC;
}

//===========================================================================

HRESULT COPOSRemoteOrderDisplay::DoInvoke( DEBUGPARAM(LPCSTR pFuncName)
    HRESULT hRC, OposVariant* pParms, unsigned nParms, int nDispIDIndex, long* pRC, bool bOpenCheck )
{
    // Trace incoming function and parameters.
#ifdef _DEBUG
    OposVariant* pVar;
    TCHAR szParmNum[20];
    TCHAR szPrefix[100];
    OposVariant* pStrRefs = new OposVariant[nParms];

    DOTRACEV( ( _T("+%hs [Function]  %u parameter%s"), pFuncName, nParms, nParms == 1 ? _T("") : _T("s") ) );
    unsigned u;
    for ( u = 0; ++u <= nParms; )
    {
        HRESULT hRCTmp;
        pVar = &pParms[ nParms - u ];
        _sntprintf( szParmNum, TSIZEOF(szParmNum), _T("    #%u: "), u );
        NULTERMINATE(szParmNum);
        switch ( pVar->vt )
        {
        case VT_EMPTY: DOTRACEV( ( _T("%sEMPTY"), szParmNum ) ); break;
        case VT_BOOL: DOTRACEV( ( _T("%sBOOL 0x%X"), szParmNum, pVar->boolVal ) ); break;
        case VT_BYREF|VT_BOOL: DOTRACEV( ( _T("%sBOOL* 0x%X"), szParmNum, *pVar->pboolVal ) ); break;
        case VT_I4: DOTRACEV( ( _T("%sLONG %d (0x%X)"), szParmNum, pVar->lVal, pVar->lVal ) ); break;
        case VT_BYREF|VT_I4: DOTRACEV( ( _T("%sLONG* %d (0x%X)"), szParmNum, *pVar->plVal, *pVar->plVal ) ); break;
        case VT_DISPATCH: DOTRACEV( ( _T("%sDISPATCH 0x%X"), szParmNum, pVar->pdispVal ) ); break;
        case VT_BSTR:
            _sntprintf( szPrefix, TSIZEOF(szPrefix), _T("%sSTRING;"), szParmNum );
            NULTERMINATE(szPrefix);
            DOTRACESTRINGV( szPrefix, pVar->bstrVal );
            break;
        case VT_BYREF|VT_BSTR:
            pStrRefs[u-1].SetBSTR_Copy( *pVar->pbstrVal, hRCTmp );
            _sntprintf( szPrefix, TSIZEOF(szPrefix), _T("%sSTRING*;"), szParmNum );
            NULTERMINATE(szPrefix);
            DOTRACESTRINGV( szPrefix, *pVar->pbstrVal );
            break;
        default: DOTRACEV( ( _T("%sUnknown (VarType=0x%X)"), szParmNum, pVar->vt ) ); break;
        }
    }

    // If bad ResultCode pointer, then HRESULT.
    if ( pRC == 0 )
        hRC = E_POINTER;

    // If bad HRESULT (either due to incoming value or bad ResultCode pointer), then return now.
    if ( FAILED(hRC) )
    {
        DOTRACEERR( _T("-  Failed: Parameter error 0x%X\n    %s"), hRC );
        delete [] pStrRefs;
        return hRC;
    }
#endif

    hRC = -1;

    // Clear the CO result code.
    SetRC();

    // If the SO must be open...
    if ( bOpenCheck )
    {
        // If not opened, set error code and return....
        if ( ! _bOpened )
        {
            *pRC = OPOS_E_CLOSED;
            hRC = S_OK;
        }
        // If the SO's release level doesn't support this method, then return error.
        else if ( nDispIDIndex >= _nMaxMethod )
        {
            *pRC = SetRC( OPOS_E_NOSERVICE );
            hRC = S_OK;
        }
    }

    // Complete setup for Invoke call.
    if ( hRC == -1 )
    {
        OposVariantResult VarResult;
        OposDispParms Disp( pParms, nParms );

        // Invoke the SO method.
        hRC = _pService->Invoke(
                _DispIDs[nDispIDIndex], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );

        // Set OPOS result code to SO method value if Invoke succeeded,
        //   else flag that we failed to get to the SO.
        *pRC = (hRC == S_OK) ? VarResult.GetLONG() : SetRC( OPOS_E_NOSERVICE );
    }

    // Trace result and outgoing parameters.
#ifdef _DEBUG
    LPCTSTR pszInfo = _T("");
    TCHAR szHR[50];
    if ( !_bOpened )
        pszInfo = _T(" -- Closed");
    else if ( nDispIDIndex >= _nMaxMethod )
        pszInfo = _T(" -- Not supported by SO");
    else if ( hRC != 0 )
    {
        _sntprintf( szHR, TSIZEOF(szHR), _T(" -- COM error: HR = 0x%X"), hRC );
        NULTERMINATE(szHR);
        pszInfo = szHR;
    }

    DOTRACEV( ( _T("-%hs [Function]  RC=%d%s"), pFuncName, *pRC, pszInfo ) );
    for ( u = 0; ++u <= nParms; )
    {
        pVar = &pParms[ nParms - u ];
        _sntprintf( szParmNum, TSIZEOF(szParmNum), _T("    #%u: "), u );
        NULTERMINATE(szParmNum);
        switch ( pVar->vt )
        {
        case VT_BYREF|VT_BOOL: DOTRACEV( ( _T("%sBOOL* 0x%X"), szParmNum, *pVar->pboolVal ) ); break;
        case VT_BYREF|VT_I4: DOTRACEV( ( _T("%sLONG* %d (0x%X)"), szParmNum, *pVar->plVal, *pVar->plVal ) ); break;
        case VT_BYREF|VT_BSTR:
            {
                _sntprintf( szPrefix, TSIZEOF(szPrefix), _T("%sSTRING*;"), szParmNum );
                NULTERMINATE(szPrefix);
                UINT nStrRefLen;
                OposVariant* pStrRef = &pStrRefs[u-1];
                if ( pStrRef->vt == VT_BSTR &&
                     ( nStrRefLen = ::SysStringByteLen( pStrRef->bstrVal ) ) == ::SysStringByteLen( *pVar->pbstrVal ) &&
                      0 == ::memcmp( pStrRef->bstrVal, *pVar->pbstrVal, nStrRefLen ) )
                    DOTRACEV( ( _T("%s Unchanged"), szPrefix ) );
                else
                    DOTRACESTRINGV( szPrefix, *pVar->pbstrVal );
                break;
            }
        }
    }

    delete [] pStrRefs;
#endif

    return hRC;
}

//===========================================================================

void COPOSRemoteOrderDisplay::SetCOFreezeEvents( VARIANT_BOOL bFreeze )
{
    // If open...
    if ( _bOpened )
    {
        DOTRACEV( ( _T("*SetCOFreezeEvents: 0x%X"), bFreeze ) );

        // Set up and call the method to inform the SO of container freeze/unfreeze.
        OposVariant Var;
        OposVariantResult VarResult;
        Var.SetVARIANT_BOOL( bFreeze );
        DISPPARAMS Disp = { &Var, NULL, 1, 0 };
        _pService->Invoke(
                _DispIDs[nDICOFreezeEvents], IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL );
    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Methods
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Open, Close
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP COPOSRemoteOrderDisplay::Open( BSTR DeviceName, long *pRC )
{
    HRESULT hRC;
    LONG nError;

    DOTRACE( ( _T("*Open [Function]: DeviceName <%ls>"), DeviceName ) );

    // If already open, return error.
    if ( _bOpened )
    {
        *pRC = SetRC( OPOS_E_ILLEGAL );
        _nOpenResult = OPOS_OR_ALREADYOPEN;
        DOTRACE( ( _T("-  Failed: CCO already open (%d/%d)"), *pRC, _nOpenResult ) );
        return S_OK;
    }

    // Initialize ResultCode and OpenResult for possible returns below.
    *pRC = SetRC( OPOS_E_NOEXIST );
    _nOpenResult = OPOS_OR_REGBADNAME;

    // Check the DeviceName length...
    UINT nDeviceNameLen = ::SysStringLen( DeviceName );

    //  - Ensure that the DeviceName is not an empty string.
    if ( nDeviceNameLen == 0 )
    {
        DOTRACE( ( _T("-  Failed: Empty device name (%d/%d)"), *pRC, _nOpenResult ) );
        return S_OK;
    }

    //  - Ensure that the DeviceName is not unreasonably long.
    //     Otherwise it may overrun this line below: wcscpy( szPhysicalDeviceName, DeviceName )
    if ( nDeviceNameLen >= MAX_REGBUFFER_SIZE )
    {
        DOTRACE( ( _T("-  Failed: Device name too big (%d/%d)"), *pRC, _nOpenResult ) );
        return S_OK;
    }

    // Form and open the base key.
    TCHAR szBaseKey[ MAX_REGBUFFER_SIZE ];
    HKEY hBaseKey;
    cw_strcpy( szBaseKey, OPOS_ROOTKEY "\\" OPOS_CLASSKEY_ROD );
    nError = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,         // Handle to open key
            szBaseKey,                  // Address of name of subkey to open
            0,                          // Reserved
            KEY_READ,                   // Security access mask
            &hBaseKey );                // Address of handle to open key
    if ( ERROR_SUCCESS != nError )
    {
        DOTRACE( ( _T("-  Failed: Cannot open registry key <%s> (%d/%d)"), szBaseKey, *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    RegOpenKeyEx error %d\n    %s"), nError );
        return S_OK;
    }
    DOTRACE( ( _T("    Opened registry key HKLM\\%s"), szBaseKey ) );

    // If the DeviceName parameter is a string value in the base key,
    //   then a logical name was specified:
    //   Use the value's data as the physical device name.
    // Else the name is the physical device name.
    TCHAR szPhysicalDeviceName[ MAX_REGBUFFER_SIZE ];
    DWORD dwType;
    DWORD dwSize = sizeof( szPhysicalDeviceName );
    if ( ERROR_SUCCESS !=
         RegQueryValueEx(
            hBaseKey,                   // Handle to key to query
            _CW(DeviceName),            // Address of name of value to query
            0,                          // Reserved
            &dwType,                    // Address of buffer for value type
            (LPBYTE) szPhysicalDeviceName, // Address of data buffer
            &dwSize ) ||                // Address of data buffer size
         dwType != REG_SZ
       )
    {
        wcscpy( szPhysicalDeviceName, DeviceName );
        DOTRACE( ( _T("    DeviceName is not a LogicalName; treating as PhysicalName") ) );
    }
    else
    {
        NULTERMINATE(szPhysicalDeviceName);
        DOTRACE( ( _T("    DeviceName is a LogicalName; converted to PhysicalName <%s>"), szPhysicalDeviceName ) );
    }

    // Open the physical registry entry. Close the base key.
    //   If fails, then return error.
    HKEY hServiceKey;
    LONG nRegStat = RegOpenKeyEx(
            hBaseKey,                   // handle to open key
            szPhysicalDeviceName,       // address of name of subkey to open
            0,                          // reserved
            KEY_READ,                   // security access mask
            &hServiceKey );             // address of handle to open key
    RegCloseKey( hBaseKey );
    if ( ERROR_SUCCESS != nRegStat )
    {
        DOTRACE( ( _T("-  Failed: Cannot open DeviceName subkey (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    RegOpenKeyEx error %d\n    %s"), nRegStat );
        return S_OK;
    }
    DOTRACE( ( _T("    Opened DeviceName subkey") ) );

    // Initialize ResultCode and OpenResult for possible returns below.
    *pRC = SetRC( OPOS_E_NOSERVICE );
    _nOpenResult = OPOS_OR_REGPROGID;

    // Now get the Service Object's ProgID, which is the default value's data for the device's key.
    //   If fails, then return error.
    TCHAR sSOProgID[ MAX_REGBUFFER_SIZE ];
    dwSize = sizeof( sSOProgID );
    nRegStat = RegQueryValueEx(
            hServiceKey,                // Handle to key to query
            0,                          // Address of name of value to query
            0,                          // Reserved
            &dwType,                    // Address of buffer for value type
            (LPBYTE) sSOProgID,         // Address of data buffer
            &dwSize );                  // Address of data buffer size
    RegCloseKey( hServiceKey );
    if ( ERROR_SUCCESS != nRegStat || dwType != REG_SZ )
    {
        DOTRACE( ( _T("-  Failed: Cannot get ProgID from registry (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    RegOpenKeyEx error %d\n    %s"), nRegStat );
        return S_OK;
    }
    NULTERMINATE(sSOProgID);
    DOTRACE( ( _T("    DeviceName mapped to ProgID <%s>"), sSOProgID ) );

    // Convert the ProgID to a CLSID.
    CLSID SOClassID;
    hRC = CLSIDFromProgID(
            _CW(sSOProgID),             // Pointer to the ProgID
            &SOClassID );               // Pointer to the CLSID
    if ( S_OK != hRC )
    {
        DOTRACE( ( _T("-  Failed: Cannot convert ProgID to ClassID (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    CLSIDFromProgID error 0x%X\n    %s"), hRC );
        return S_OK;
    }
    DOTRACE( ( _T("    ProgID mapped to ClassID") ) );

    // Make the connection to the Service Object.
    //   We create an instance, get the IDispatch pointer, then release the IUnknown pointer.
    IUnknown* pUnknown;
    hRC = CoCreateInstance(
            SOClassID,                  // Class identifier (CLSID) of the object
            0,                          // Pointer to whether object is or isn't part of an aggregate
            CLSCTX_ALL,                 // Context for running executable code
            IID_IUnknown,               // Reference to the identifier of the interface
            (void**) &pUnknown);        // Address of output variable that receives the interface pointer requested in riid
    if ( S_OK != hRC )
    {
        _nOpenResult = OPOS_OR_CREATE;
        DOTRACE( ( _T("-  Failed: Cannot create Service Object (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    CoCreateInstance error 0x%X\n    %s"), hRC );
        return S_OK;
    }
    DOTRACE( ( _T("    Created Service Object") ) );

    hRC = pUnknown->QueryInterface(
            IID_IDispatch,              // Identifier of the requested interface
            (void**) &_pService );      // Address of output variable that receives the interface pointer requested in iid
    pUnknown->Release();                // Done with original pointer.
    if ( S_OK != hRC )
    {
        _nOpenResult = OPOS_OR_CREATE;
        DOTRACE( ( _T("-  Failed: Cannot get Service Object's Dispatch pointer (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    QueryInterface error 0x%X\n    %s"), hRC );
        return S_OK;
    }
    DOTRACE( ( _T("    Acquired Service Object's dispatch pointer") ) );

    // Get the dispatch IDs for the methods supported by the Service Object.
    //   Stop when we can't get one or when all are acquired.
    int nMethodCount = 0;
    while ( s_SOMethodNames[nMethodCount] != 0 )
    {
        _CWNCC MN( s_SOMethodNames[nMethodCount] ); // Init for conversion.
        OLECHAR* pMN = MN;                          // Assign pointer, converting to wide if needed.
        if ( S_OK !=
             _pService->GetIDsOfNames(
                IID_NULL, &pMN, 1, LOCALE_SYSTEM_DEFAULT, &_DispIDs[nMethodCount] )
           )
        {
            // If failed, do special cases:
            //  - If we failed the newer CloseService, then try Close before failing.
            if ( nMethodCount == nDICloseService )
                pMN = L"Close";
            //  - If we failed the newer COM-friendly name ClaimDevice or ReleaseDevice,
            //    then try the original name Claim or Release before failing.
            else if ( nMethodCount == nDIClaimDevice )
                pMN = L"Claim";
            else if ( nMethodCount == nDIReleaseDevice )
                pMN = L"Release";
            else                        // If not special case,
                break;                  //   stop.
            if ( S_OK !=
                 _pService->GetIDsOfNames(
                    IID_NULL, &pMN, 1, LOCALE_SYSTEM_DEFAULT, &_DispIDs[nMethodCount] )
               )
                break;                  // If special case but still not found, stop.
        }
        nMethodCount++;
    }

    // When debug mode, repeat above trying to get all methods, for later tracing.
    #ifdef _DEBUG
    int nAllMethods = nMethodCount;
    while ( s_SOMethodNames[nAllMethods] != 0 )
    {
        _CWNCC MN( s_SOMethodNames[nAllMethods] );  // Init for conversion.
        OLECHAR* pMN = MN;                          // Assign pointer, converting to wide if needed.
        if ( S_OK !=
             _pService->GetIDsOfNames(
                IID_NULL, &pMN, 1, LOCALE_SYSTEM_DEFAULT, &_DispIDs[nAllMethods] )
           )
        {
            pMN = 0;
            // If failed, do special cases:
            //  - If we failed the newer CloseService, then try Close before failing.
            if ( nAllMethods == nDICloseService )
                pMN = L"Close";
            //  - If we failed the newer COM-friendly name ClaimDevice or ReleaseDevice,
            //    then try the original name Claim or Release before failing.
            else if ( nAllMethods == nDIClaimDevice )
                pMN = L"Claim";
            else if ( nAllMethods == nDIReleaseDevice )
                pMN = L"Release";
            else                                // If not special case,
                _DispIDs[nAllMethods] = -123;   //   mark.
            if ( pMN != 0 && S_OK !=
                 _pService->GetIDsOfNames(
                    IID_NULL, &pMN, 1, LOCALE_SYSTEM_DEFAULT, &_DispIDs[nAllMethods] )
               )
                _DispIDs[nAllMethods] = -123;   // If special case but still not found, mark.
        }
        nAllMethods++;
    }
    #endif

    // The SO must at least support all of the initial release's methods,
    //   or we won't continue.
    if ( nMethodCount < s_RequiredMethodsPerRelease[0] )
    {
        _nOpenResult = OPOS_OR_BADIF;
        _pService->Release();
        DOTRACE( ( _T("-  Failed: Doesn't support all of initial release's methods (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEMETHODS( _DispIDs, s_RequiredMethodsPerRelease[0] );
        return S_OK;
    }

    // Get our own dispatch pointer.
    //   Immediately release it -- saves releasing later,
    //   since it is just another reference to us.
    IDispatch* pDispatch = 0;
    CMVERIFY( S_OK == QueryInterface( IID_IDispatch, (void**) &pDispatch ) );
    if ( pDispatch != 0 )
        pDispatch->Release();

    // Now call the Service Object's OpenService method.
    OposVariant Vars[3];
    Vars[2].SetString( _CW(OPOS_CLASSKEY_ROD), hRC );
    Vars[1].SetString( _CW(szPhysicalDeviceName), hRC );
    Vars[0].SetDispatch_Ptr( pDispatch, hRC );
    if ( FAILED(hRC) )
    {
        _pService->Release();
        DOTRACEERR( _T("-  Failed: Setup for OpenService call 0x%X\n    %s"), hRC );
        return hRC;
    }
    hRC = DoInvoke( DEBUGPARAM("Open-OpenService") S_OK, Vars, 3, nDIOpenService, pRC, false );

    // If the OpenService fails, then set result code and return.
    if ( hRC != S_OK || *pRC != OPOS_SUCCESS )
    {
        // Set the default open result code.
        _nOpenResult = OPOS_OR_FAILEDOPEN;
        // If not a COM error, and the SO supports the GetOpenResult method,
        //   then call it to get the open result code.
        if ( hRC == S_OK )
        {
            // Try to get DispID of the method.
            OLECHAR* pMN = L"GetOpenResult";
            DISPID DispID;
            if ( S_OK ==
                 _pService->GetIDsOfNames(
                    IID_NULL, &pMN, 1, LOCALE_SYSTEM_DEFAULT, &DispID )
               )
            {
                // If method exists, set up and call the SO's get property open result method.
                OposVariantResult VarResult;
                DISPPARAMS Disp = { NULL, NULL, 0, 0 };
                if ( S_OK == _pService->Invoke(
                        DispID, IID_NULL, LOCALE_USER_DEFAULT,
                        DISPATCH_METHOD, &Disp, &VarResult, NULL, NULL )
                   )
                {
                    // If method succeeds, convert (if needed) to long and set open result code.
                    HRESULT hRCTmp;
                    VarResult.ChangeLONG( &_nOpenResult, hRCTmp );
                }
            }
        }
        // Release the SO and return its OpenService HRESULT.
        _pService->Release();
        DOTRACE( ( _T("-  Failed: Service Object's OpenService reported failure (%d/%d)"), *pRC, _nOpenResult ) );
        DOTRACEERR( _T("    Invoke error 0x%X\n    %s"), hRC );
        return hRC;
    }
    DOTRACE( ( _T("    Service Object's OpenService succeeded") ) );

    // Prepare for events.
    EventOpen();

    // The CO and SO are now open.
    _bOpened = true;

    // Trace some service object information, and initialize debug data.
    #ifdef _DEBUG
    DOTRACE( ( _T("    <Start> Service Object Properties.") ) );
    BSTR bstrSOInfo;
    GetOposProp( DEBUGPARAM("Open-ServiceObjectDescription") PIDX_ServiceObjectDescription, &bstrSOInfo, 0 );
    SysFreeString(bstrSOInfo);
    GetOposProp( DEBUGPARAM("Open-DeviceDescription") PIDX_DeviceDescription, &bstrSOInfo, 0 );
    SysFreeString(bstrSOInfo);
    GetOposProp( DEBUGPARAM("Open-DeviceName") PIDX_DeviceName, &bstrSOInfo, 0 );
    SysFreeString(bstrSOInfo);
    DOTRACE( ( _T("    <End> Service Object Properties.") ) );
    #endif

    // Now do some versioning support checks...
    bool bBadSO = false;

    //  - Get the SO's version, and determine the major and minor version.
    long nSOVersion;
    GetOposProp( DEBUGPARAM("Open-ServiceObjectVersion") PIDX_ServiceObjectVersion, &nSOVersion, 0 );
    _nSOMajor = nSOVersion / 1000000;
    _nSOMinor = ( nSOVersion / 1000 ) % 1000;

    //  - If major version is not 1, then bad SO.
    if ( _nSOMajor != 1 )
    {
        _nOpenResult = OPOS_OR_BADVERSION;
        bBadSO = true;
        DOTRACE( ( _T("-  Failed: Service Object's major version is not 1 (%d/%d)"), OPOS_E_NOSERVICE, _nOpenResult ) );
    }

    //  - If SO doesn't support enough methods, then bad SO.
    else
    {
        long nReqMethodIndex = sizeof(s_RequiredMethodsPerRelease)/sizeof(int) - 1;
        // Shouldn't have SO with release below control's first release, but check anyway.
        if ( _nSOMinor < MinorReleaseInitial )
            nReqMethodIndex = 0;
        // If SO release is below that supported by this control, use its release's methods.
        else if ( nReqMethodIndex > _nSOMinor - MinorReleaseInitial )
            nReqMethodIndex = _nSOMinor - MinorReleaseInitial;
        // Set the maximum method index.
        _nMaxMethod = s_RequiredMethodsPerRelease[ nReqMethodIndex ];
        // Ensure that the SO supports the methods that it claims to support.
        if ( nMethodCount < _nMaxMethod )
        {
            _nOpenResult = OPOS_OR_BADIF;
            bBadSO = true;
            DOTRACE( ( _T("-  Failed: Doesn't support all of its release's methods (%d/%d)"), OPOS_E_NOSERVICE, _nOpenResult ) );
            DOTRACEMETHODS( _DispIDs, _nMaxMethod );
        }
    }

    // If bad SO, then fail the open.
    if ( bBadSO )
    {
        Close( pRC );
        *pRC = SetRC( OPOS_E_NOSERVICE );
        return S_OK;
    }

    // If CO OnFreezeEvents(TRUE) method has been called before open then freeze
    //   events in the SO at this time.
    if ( _nCOFreezeEventCount )
        SetCOFreezeEvents( TRUE );

    // Everything went okay, so return success.
    SetRC();
    *pRC = OPOS_SUCCESS;
    _nOpenResult = OPOS_SUCCESS;
    DOTRACE( ( _T("-  Success (0/0)") ) );
    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::Close( long *pRC )
{
    SetRC();

    // If not opened, set return code.
    if ( ! _bOpened )
    {
        *pRC = OPOS_E_CLOSED;
        DOTRACE( ( _T("*Close [Function]  RC=%d"), *pRC ) );
        return S_OK;
    }

    // Events are no longer legal.
    EventPreCloseRelease();

    // Call down into Service Object to close the control.
    HRESULT hRC = DoInvoke( DEBUGPARAM("Close-CloseService") S_OK, NULL, 0, nDICloseService, pRC, false );

    // Release the connection to the Service Object.
    _pService->Release();

    // Close down events.
    EventClose();

    // We are now closed.
    _bOpened = false;

    DOTRACE( ( _T("*Close [Function]  RC=%d"), *pRC ) );
    return hRC;
}

/////////////////////////////////////////////////////////////////////////////
// Claim, Release
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP COPOSRemoteOrderDisplay::ClaimDevice( long Timeout, long *pRC )
{
    SetRC();

    // If not opened, set return code.
    if ( ! _bOpened )
    {
        *pRC = OPOS_E_CLOSED;
        DOTRACEV( ( _T("*ClaimDevice [Function] -- Closed") ) );
        return S_OK;
    }

    // Initialize so that events are allowed.
    EventClaim();

    // Call down into the Service Object to execute this method.
    OposVariant Var;
    Var.SetLONG( Timeout );
    return DoInvoke( DEBUGPARAM("ClaimDevice") S_OK, &Var, 1, nDIClaimDevice, pRC, false );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ReleaseDevice( long *pRC )
{
    SetRC();

    // If not opened, set return code.
    if ( ! _bOpened )
    {
        *pRC = OPOS_E_CLOSED;
        DOTRACEV( ( _T("*ReleaseDevice [Function] -- Closed") ) );
        return S_OK;
    }

    // Events are no longer legal.
    EventPreCloseRelease();

    // Call down into the Service Object to execute this method.
    return DoInvoke( DEBUGPARAM("ReleaseDevice") S_OK, NULL, 0, nDIReleaseDevice, pRC, false );
}

/////////////////////////////////////////////////////////////////////////////
// SOProcessID
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP COPOSRemoteOrderDisplay::SOProcessID( long *pProcessID )
{
    *pProcessID = GetCurrentProcessId();
    DOTRACEV( ( _T("*SOProcessID: 0x%X"), *pProcessID ) );
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// The other methods.
/////////////////////////////////////////////////////////////////////////////

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.3
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::CheckHealth( 
    /*[in]*/ LONG Level, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( Level );
    return DoInvoke( DEBUGPARAM("CheckHealth") S_OK, &Var, 1, nDICheckHealth, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ClearInput( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("ClearInput") S_OK, NULL, 0, nDIClearInput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ClearOutput( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("ClearOutput") S_OK, NULL, 0, nDIClearOutput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::DirectIO( 
    /*[in]*/ LONG Command, 
    /*[in, out]*/ LONG* pData, 
    /*[in, out]*/ BSTR* pString, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( Command );
    Vars[1].SetLONG_Ptr( pData, hRC );
    Vars[0].SetBSTR_InOutPtr( pString, hRC );
    return DoInvoke( DEBUGPARAM("DirectIO") hRC, Vars, 3, nDIDirectIO, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ClearVideo( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Attribute, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Units );
    Vars[0].SetLONG( Attribute );
    return DoInvoke( DEBUGPARAM("ClearVideo") S_OK, Vars, 2, nDIClearVideo, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ClearVideoRegion( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Height, 
    /*[in]*/ LONG Width, 
    /*[in]*/ LONG Attribute, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    Vars[5].SetLONG( Units );
    Vars[4].SetLONG( Row );
    Vars[3].SetLONG( Column );
    Vars[2].SetLONG( Height );
    Vars[1].SetLONG( Width );
    Vars[0].SetLONG( Attribute );
    return DoInvoke( DEBUGPARAM("ClearVideoRegion") S_OK, Vars, 6, nDIClearVideoRegion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ControlClock( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Function, 
    /*[in]*/ LONG ClockId, 
    /*[in]*/ LONG Hour, 
    /*[in]*/ LONG Min, 
    /*[in]*/ LONG Sec, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Attribute, 
    /*[in]*/ LONG Mode, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[10];
    Vars[9].SetLONG( Units );
    Vars[8].SetLONG( Function );
    Vars[7].SetLONG( ClockId );
    Vars[6].SetLONG( Hour );
    Vars[5].SetLONG( Min );
    Vars[4].SetLONG( Sec );
    Vars[3].SetLONG( Row );
    Vars[2].SetLONG( Column );
    Vars[1].SetLONG( Attribute );
    Vars[0].SetLONG( Mode );
    return DoInvoke( DEBUGPARAM("ControlClock") S_OK, Vars, 10, nDIControlClock, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ControlCursor( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Function, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Units );
    Vars[0].SetLONG( Function );
    return DoInvoke( DEBUGPARAM("ControlCursor") S_OK, Vars, 2, nDIControlCursor, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::CopyVideoRegion( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Height, 
    /*[in]*/ LONG Width, 
    /*[in]*/ LONG TargetRow, 
    /*[in]*/ LONG TargetColumn, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[7];
    Vars[6].SetLONG( Units );
    Vars[5].SetLONG( Row );
    Vars[4].SetLONG( Column );
    Vars[3].SetLONG( Height );
    Vars[2].SetLONG( Width );
    Vars[1].SetLONG( TargetRow );
    Vars[0].SetLONG( TargetColumn );
    return DoInvoke( DEBUGPARAM("CopyVideoRegion") S_OK, Vars, 7, nDICopyVideoRegion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::DisplayData( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Attribute, 
    /*[in]*/ BSTR Data, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[5];
    HRESULT hRC = S_OK;
    Vars[4].SetLONG( Units );
    Vars[3].SetLONG( Row );
    Vars[2].SetLONG( Column );
    Vars[1].SetLONG( Attribute );
    Vars[0].SetBSTR( Data, hRC );
    return DoInvoke( DEBUGPARAM("DisplayData") hRC, Vars, 5, nDIDisplayData, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::DrawBox( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Height, 
    /*[in]*/ LONG Width, 
    /*[in]*/ LONG Attribute, 
    /*[in]*/ LONG BorderType, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[7];
    Vars[6].SetLONG( Units );
    Vars[5].SetLONG( Row );
    Vars[4].SetLONG( Column );
    Vars[3].SetLONG( Height );
    Vars[2].SetLONG( Width );
    Vars[1].SetLONG( Attribute );
    Vars[0].SetLONG( BorderType );
    return DoInvoke( DEBUGPARAM("DrawBox") S_OK, Vars, 7, nDIDrawBox, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::FreeVideoRegion( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG BufferId, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Units );
    Vars[0].SetLONG( BufferId );
    return DoInvoke( DEBUGPARAM("FreeVideoRegion") S_OK, Vars, 2, nDIFreeVideoRegion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::ResetVideo( 
    /*[in]*/ LONG Units, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( Units );
    return DoInvoke( DEBUGPARAM("ResetVideo") S_OK, &Var, 1, nDIResetVideo, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::RestoreVideoRegion( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG TargetRow, 
    /*[in]*/ LONG TargetColumn, 
    /*[in]*/ LONG BufferId, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[4];
    Vars[3].SetLONG( Units );
    Vars[2].SetLONG( TargetRow );
    Vars[1].SetLONG( TargetColumn );
    Vars[0].SetLONG( BufferId );
    return DoInvoke( DEBUGPARAM("RestoreVideoRegion") S_OK, Vars, 4, nDIRestoreVideoRegion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::SaveVideoRegion( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Height, 
    /*[in]*/ LONG Width, 
    /*[in]*/ LONG BufferId, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    Vars[5].SetLONG( Units );
    Vars[4].SetLONG( Row );
    Vars[3].SetLONG( Column );
    Vars[2].SetLONG( Height );
    Vars[1].SetLONG( Width );
    Vars[0].SetLONG( BufferId );
    return DoInvoke( DEBUGPARAM("SaveVideoRegion") S_OK, Vars, 6, nDISaveVideoRegion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::SelectCharacterSet( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG CharacterSet, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Units );
    Vars[0].SetLONG( CharacterSet );
    return DoInvoke( DEBUGPARAM("SelectCharacterSet") S_OK, Vars, 2, nDISelectCharacterSet, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::SetCursor( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    Vars[2].SetLONG( Units );
    Vars[1].SetLONG( Row );
    Vars[0].SetLONG( Column );
    return DoInvoke( DEBUGPARAM("SetCursor") S_OK, Vars, 3, nDISetCursor, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::TransactionDisplay( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Function, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Units );
    Vars[0].SetLONG( Function );
    return DoInvoke( DEBUGPARAM("TransactionDisplay") S_OK, Vars, 2, nDITransactionDisplay, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::UpdateVideoRegionAttribute( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Function, 
    /*[in]*/ LONG Row, 
    /*[in]*/ LONG Column, 
    /*[in]*/ LONG Height, 
    /*[in]*/ LONG Width, 
    /*[in]*/ LONG Attribute, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[7];
    Vars[6].SetLONG( Units );
    Vars[5].SetLONG( Function );
    Vars[4].SetLONG( Row );
    Vars[3].SetLONG( Column );
    Vars[2].SetLONG( Height );
    Vars[1].SetLONG( Width );
    Vars[0].SetLONG( Attribute );
    return DoInvoke( DEBUGPARAM("UpdateVideoRegionAttribute") S_OK, Vars, 7, nDIUpdateVideoRegionAttribute, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::VideoSound( 
    /*[in]*/ LONG Units, 
    /*[in]*/ LONG Frequency, 
    /*[in]*/ LONG Duration, 
    /*[in]*/ LONG NumberOfCycles, 
    /*[in]*/ LONG InterSoundWait, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[5];
    Vars[4].SetLONG( Units );
    Vars[3].SetLONG( Frequency );
    Vars[2].SetLONG( Duration );
    Vars[1].SetLONG( NumberOfCycles );
    Vars[0].SetLONG( InterSoundWait );
    return DoInvoke( DEBUGPARAM("VideoSound") S_OK, Vars, 5, nDIVideoSound, pRC );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.4
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.5
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.6
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.7
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.8
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::ResetStatistics( 
    /*[in]*/ BSTR StatisticsBuffer, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( StatisticsBuffer, hRC );
    return DoInvoke( DEBUGPARAM("ResetStatistics") hRC, &Var, 1, nDIResetStatistics, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::RetrieveStatistics( 
    /*[in, out]*/ BSTR* pStatisticsBuffer, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR_InOutPtr( pStatisticsBuffer, hRC );
    return DoInvoke( DEBUGPARAM("RetrieveStatistics") hRC, &Var, 1, nDIRetrieveStatistics, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::UpdateStatistics( 
    /*[in]*/ BSTR StatisticsBuffer, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( StatisticsBuffer, hRC );
    return DoInvoke( DEBUGPARAM("UpdateStatistics") hRC, &Var, 1, nDIUpdateStatistics, pRC );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.9
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::CompareFirmwareVersion( 
    /*[in]*/ BSTR FirmwareFileName, 
    /*[out]*/ LONG* pResult, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetBSTR( FirmwareFileName, hRC );
    Vars[0].SetLONG_Ptr( pResult, hRC );
    return DoInvoke( DEBUGPARAM("CompareFirmwareVersion") hRC, Vars, 2, nDICompareFirmwareVersion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::UpdateFirmware( 
    /*[in]*/ BSTR FirmwareFileName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( FirmwareFileName, hRC );
    return DoInvoke( DEBUGPARAM("UpdateFirmware") hRC, &Var, 1, nDIUpdateFirmware, pRC );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Properties
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//   FreezeEvents
//   ResultCode
//   State
//   ControlObjectDescription
//   ControlObjectVersion
//   OpenResult
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP COPOSRemoteOrderDisplay::get_FreezeEvents( VARIANT_BOOL *pFreezeEvents )
{
    return GetOposProp( DEBUGPARAM("FreezeEvents") PIDX_FreezeEvents, pFreezeEvents );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::put_FreezeEvents( VARIANT_BOOL FreezeEvents )
{
    SetRC();

    // If not opened, do nothing.
    if ( ! _bOpened )
    {
        DOTRACEV( ( _T("*FreezeEvents [Set] -- Closed") ) );
        return S_OK;
    }

    if ( FreezeEvents )
    {
        // Call down into the Service Object first to set the property value.
        SetOposProp( DEBUGPARAM("FreezeEvents") PIDX_FreezeEvents, FreezeEvents ); // First tell the SO the new state.
        _bFreezeState = true;                           // Then update ours.
    }
    else
    {
        // Update our state first, then tell the SO.
        // This avoids a lockup in the event firing object's DoEvent() routine
        //   that would occur if the SO calls an event from its property update method.
        _bFreezeState = false;
        SetOposProp( DEBUGPARAM("FreezeEvents") PIDX_FreezeEvents, FreezeEvents );
    }

    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ResultCode( long *pResultCode )
{
    if ( ! _bOpened )                               // If not open,
    {
        *pResultCode = OPOS_E_CLOSED;               //   then return "closed".
        DOTRACEV( ( _T("*ResultCode [Get] -- Closed") ) );
    }
    else if ( _nCOResultCode != -1 )                // If the control object has to report a status,
    {
        *pResultCode = _nCOResultCode;              //   then return it.
        DOTRACEV( ( _T("*ResultCode [Get]: %d -- from CO"), _nCOResultCode ) );
    }
    else
        GetOposProp( DEBUGPARAM("ResultCode") PIDX_ResultCode, pResultCode );// Otherwise go to SO.

    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_State( long *pState )
{
    if ( ! _bOpened )                           // If not open,
    {
        *pState = OPOS_S_CLOSED;                //   then return "closed".
        DOTRACEV( ( _T("*State [Get] -- Closed") ) );
    }
    else
        GetOposProp( DEBUGPARAM("State") PIDX_State, pState ); // Otherwise go to SO.

    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ControlObjectDescription( BSTR *pControlObjectDescription )
{
    *pControlObjectDescription = ::SysAllocString( _CW(g_ControlDescription) );
    DOTRACEV( ( _T("*ControlObjectDescription [Get]: <%hs>"), g_ControlDescription ) );
    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ControlObjectVersion( long *pControlObjectVersion )
{
    *pControlObjectVersion = g_ControlVersion;
    DOTRACEV( ( _T("*ControlObjectVersion [Get]: %d"), g_ControlVersion ) );
    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_OpenResult( long *pOpenResult )
{
    *pOpenResult = _nOpenResult;
    DOTRACEV( ( _T("*OpenResult [Get]: %d"), _nOpenResult ) );
    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// The other properties.
/////////////////////////////////////////////////////////////////////////////

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.3
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::get_BinaryConversion( /*[out, retval]*/ LONG* pBinaryConversion )
{
    return GetOposProp( DEBUGPARAM("BinaryConversion") PIDX_BinaryConversion, pBinaryConversion, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_BinaryConversion( /*[in]*/ LONG BinaryConversion )
{
    IFDEBUG( _nBinaryConversion = BinaryConversion);
    return SetOposProp( DEBUGPARAM("BinaryConversion") PIDX_BinaryConversion, BinaryConversion, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapPowerReporting( /*[out, retval]*/ LONG* pCapPowerReporting )
{
    return GetOposProp( DEBUGPARAM("CapPowerReporting") PIDX_CapPowerReporting, pCapPowerReporting, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CheckHealthText( /*[out, retval]*/ BSTR* pCheckHealthText )
{
    return GetOposProp( DEBUGPARAM("CheckHealthText") PIDX_CheckHealthText, pCheckHealthText, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_Claimed( /*[out, retval]*/ VARIANT_BOOL* pClaimed )
{
    return GetOposProp( DEBUGPARAM("Claimed") PIDX_Claimed, pClaimed, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_DataCount( /*[out, retval]*/ LONG* pDataCount )
{
    return GetOposProp( DEBUGPARAM("DataCount") PIDX_DataCount, pDataCount, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_DataEventEnabled( /*[out, retval]*/ VARIANT_BOOL* pDataEventEnabled )
{
    return GetOposProp( DEBUGPARAM("DataEventEnabled") PIDX_DataEventEnabled, pDataEventEnabled, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_DataEventEnabled( /*[in]*/ VARIANT_BOOL DataEventEnabled )
{
    return SetOposProp( DEBUGPARAM("DataEventEnabled") PIDX_DataEventEnabled, DataEventEnabled, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_DeviceEnabled( /*[out, retval]*/ VARIANT_BOOL* pDeviceEnabled )
{
    return GetOposProp( DEBUGPARAM("DeviceEnabled") PIDX_DeviceEnabled, pDeviceEnabled, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_DeviceEnabled( /*[in]*/ VARIANT_BOOL DeviceEnabled )
{
    return SetOposProp( DEBUGPARAM("DeviceEnabled") PIDX_DeviceEnabled, DeviceEnabled, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_OutputID( /*[out, retval]*/ LONG* pOutputID )
{
    return GetOposProp( DEBUGPARAM("OutputID") PIDX_OutputID, pOutputID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_PowerNotify( /*[out, retval]*/ LONG* pPowerNotify )
{
    return GetOposProp( DEBUGPARAM("PowerNotify") PIDX_PowerNotify, pPowerNotify, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_PowerNotify( /*[in]*/ LONG PowerNotify )
{
    return SetOposProp( DEBUGPARAM("PowerNotify") PIDX_PowerNotify, PowerNotify, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_PowerState( /*[out, retval]*/ LONG* pPowerState )
{
    return GetOposProp( DEBUGPARAM("PowerState") PIDX_PowerState, pPowerState, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ResultCodeExtended( /*[out, retval]*/ LONG* pResultCodeExtended )
{
    return GetOposProp( DEBUGPARAM("ResultCodeExtended") PIDX_ResultCodeExtended, pResultCodeExtended, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ServiceObjectDescription( /*[out, retval]*/ BSTR* pServiceObjectDescription )
{
    return GetOposProp( DEBUGPARAM("ServiceObjectDescription") PIDX_ServiceObjectDescription, pServiceObjectDescription, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ServiceObjectVersion( /*[out, retval]*/ LONG* pServiceObjectVersion )
{
    return GetOposProp( DEBUGPARAM("ServiceObjectVersion") PIDX_ServiceObjectVersion, pServiceObjectVersion, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_DeviceDescription( /*[out, retval]*/ BSTR* pDeviceDescription )
{
    return GetOposProp( DEBUGPARAM("DeviceDescription") PIDX_DeviceDescription, pDeviceDescription, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_DeviceName( /*[out, retval]*/ BSTR* pDeviceName )
{
    return GetOposProp( DEBUGPARAM("DeviceName") PIDX_DeviceName, pDeviceName, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_AsyncMode( /*[out, retval]*/ VARIANT_BOOL* pAsyncMode )
{
    return GetOposProp( DEBUGPARAM("AsyncMode") PIDXRod_AsyncMode, pAsyncMode, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_AsyncMode( /*[in]*/ VARIANT_BOOL AsyncMode )
{
    return SetOposProp( DEBUGPARAM("AsyncMode") PIDXRod_AsyncMode, AsyncMode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_AutoToneDuration( /*[out, retval]*/ LONG* pAutoToneDuration )
{
    return GetOposProp( DEBUGPARAM("AutoToneDuration") PIDXRod_AutoToneDuration, pAutoToneDuration, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_AutoToneDuration( /*[in]*/ LONG AutoToneDuration )
{
    return SetOposProp( DEBUGPARAM("AutoToneDuration") PIDXRod_AutoToneDuration, AutoToneDuration, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_AutoToneFrequency( /*[out, retval]*/ LONG* pAutoToneFrequency )
{
    return GetOposProp( DEBUGPARAM("AutoToneFrequency") PIDXRod_AutoToneFrequency, pAutoToneFrequency, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_AutoToneFrequency( /*[in]*/ LONG AutoToneFrequency )
{
    return SetOposProp( DEBUGPARAM("AutoToneFrequency") PIDXRod_AutoToneFrequency, AutoToneFrequency, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapSelectCharacterSet( /*[out, retval]*/ VARIANT_BOOL* pCapSelectCharacterSet )
{
    return GetOposProp( DEBUGPARAM("CapSelectCharacterSet") PIDXRod_CapSelectCharacterSet, pCapSelectCharacterSet, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapTone( /*[out, retval]*/ VARIANT_BOOL* pCapTone )
{
    return GetOposProp( DEBUGPARAM("CapTone") PIDXRod_CapTone, pCapTone, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapTouch( /*[out, retval]*/ VARIANT_BOOL* pCapTouch )
{
    return GetOposProp( DEBUGPARAM("CapTouch") PIDXRod_CapTouch, pCapTouch, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapTransaction( /*[out, retval]*/ VARIANT_BOOL* pCapTransaction )
{
    return GetOposProp( DEBUGPARAM("CapTransaction") PIDXRod_CapTransaction, pCapTransaction, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CharacterSet( /*[out, retval]*/ LONG* pCharacterSet )
{
    return GetOposProp( DEBUGPARAM("CharacterSet") PIDXRod_CharacterSet, pCharacterSet, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CharacterSetList( /*[out, retval]*/ BSTR* pCharacterSetList )
{
    return GetOposProp( DEBUGPARAM("CharacterSetList") PIDXRod_CharacterSetList, pCharacterSetList, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_Clocks( /*[out, retval]*/ LONG* pClocks )
{
    return GetOposProp( DEBUGPARAM("Clocks") PIDXRod_Clocks, pClocks, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CurrentUnitID( /*[out, retval]*/ LONG* pCurrentUnitID )
{
    return GetOposProp( DEBUGPARAM("CurrentUnitID") PIDXRod_CurrentUnitID, pCurrentUnitID, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_CurrentUnitID( /*[in]*/ LONG CurrentUnitID )
{
    return SetOposProp( DEBUGPARAM("CurrentUnitID") PIDXRod_CurrentUnitID, CurrentUnitID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ErrorString( /*[out, retval]*/ BSTR* pErrorString )
{
    return GetOposProp( DEBUGPARAM("ErrorString") PIDXRod_ErrorString, pErrorString, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_ErrorUnits( /*[out, retval]*/ LONG* pErrorUnits )
{
    return GetOposProp( DEBUGPARAM("ErrorUnits") PIDXRod_ErrorUnits, pErrorUnits, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_EventString( /*[out, retval]*/ BSTR* pEventString )
{
    return GetOposProp( DEBUGPARAM("EventString") PIDXRod_EventString, pEventString, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_EventType( /*[out, retval]*/ LONG* pEventType )
{
    return GetOposProp( DEBUGPARAM("EventType") PIDXRod_EventType, pEventType, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_EventType( /*[in]*/ LONG EventType )
{
    return SetOposProp( DEBUGPARAM("EventType") PIDXRod_EventType, EventType, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_EventUnitID( /*[out, retval]*/ LONG* pEventUnitID )
{
    return GetOposProp( DEBUGPARAM("EventUnitID") PIDXRod_EventUnitID, pEventUnitID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_EventUnits( /*[out, retval]*/ LONG* pEventUnits )
{
    return GetOposProp( DEBUGPARAM("EventUnits") PIDXRod_EventUnits, pEventUnits, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_SystemClocks( /*[out, retval]*/ LONG* pSystemClocks )
{
    return GetOposProp( DEBUGPARAM("SystemClocks") PIDXRod_SystemClocks, pSystemClocks, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_SystemVideoSaveBuffers( /*[out, retval]*/ LONG* pSystemVideoSaveBuffers )
{
    return GetOposProp( DEBUGPARAM("SystemVideoSaveBuffers") PIDXRod_SystemVideoSaveBuffers, pSystemVideoSaveBuffers, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_Timeout( /*[out, retval]*/ LONG* pTimeout )
{
    return GetOposProp( DEBUGPARAM("Timeout") PIDXRod_Timeout, pTimeout, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_Timeout( /*[in]*/ LONG Timeout )
{
    return SetOposProp( DEBUGPARAM("Timeout") PIDXRod_Timeout, Timeout, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_UnitsOnline( /*[out, retval]*/ LONG* pUnitsOnline )
{
    return GetOposProp( DEBUGPARAM("UnitsOnline") PIDXRod_UnitsOnline, pUnitsOnline, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_VideoDataCount( /*[out, retval]*/ LONG* pVideoDataCount )
{
    return GetOposProp( DEBUGPARAM("VideoDataCount") PIDXRod_VideoDataCount, pVideoDataCount, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_VideoMode( /*[out, retval]*/ LONG* pVideoMode )
{
    return GetOposProp( DEBUGPARAM("VideoMode") PIDXRod_VideoMode, pVideoMode, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_VideoMode( /*[in]*/ LONG VideoMode )
{
    return SetOposProp( DEBUGPARAM("VideoMode") PIDXRod_VideoMode, VideoMode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_VideoModesList( /*[out, retval]*/ BSTR* pVideoModesList )
{
    return GetOposProp( DEBUGPARAM("VideoModesList") PIDXRod_VideoModesList, pVideoModesList, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_VideoSaveBuffers( /*[out, retval]*/ LONG* pVideoSaveBuffers )
{
    return GetOposProp( DEBUGPARAM("VideoSaveBuffers") PIDXRod_VideoSaveBuffers, pVideoSaveBuffers, 3 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.4
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.5
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.6
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.7
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapMapCharacterSet( /*[out, retval]*/ VARIANT_BOOL* pCapMapCharacterSet )
{
    return GetOposProp( DEBUGPARAM("CapMapCharacterSet") PIDXRod_CapMapCharacterSet, pCapMapCharacterSet, 7 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_MapCharacterSet( /*[out, retval]*/ VARIANT_BOOL* pMapCharacterSet )
{
    return GetOposProp( DEBUGPARAM("MapCharacterSet") PIDXRod_MapCharacterSet, pMapCharacterSet, 7 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSRemoteOrderDisplay::put_MapCharacterSet( /*[in]*/ VARIANT_BOOL MapCharacterSet )
{
    return SetOposProp( DEBUGPARAM("MapCharacterSet") PIDXRod_MapCharacterSet, MapCharacterSet, 7 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.8
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapStatisticsReporting( /*[out, retval]*/ VARIANT_BOOL* pCapStatisticsReporting )
{
    return GetOposProp( DEBUGPARAM("CapStatisticsReporting") PIDX_CapStatisticsReporting, pCapStatisticsReporting, 8 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapUpdateStatistics( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateStatistics )
{
    return GetOposProp( DEBUGPARAM("CapUpdateStatistics") PIDX_CapUpdateStatistics, pCapUpdateStatistics, 8 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.9
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapCompareFirmwareVersion( /*[out, retval]*/ VARIANT_BOOL* pCapCompareFirmwareVersion )
{
    return GetOposProp( DEBUGPARAM("CapCompareFirmwareVersion") PIDX_CapCompareFirmwareVersion, pCapCompareFirmwareVersion, 9 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSRemoteOrderDisplay::get_CapUpdateFirmware( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateFirmware )
{
    return GetOposProp( DEBUGPARAM("CapUpdateFirmware") PIDX_CapUpdateFirmware, pCapUpdateFirmware, 9 /*MinorVersion*/ );
}


/////////////////////////////////////////////////////////////////////////////
// Debug build tracing support.
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

#include <stdio.h>
#include <tchar.h>
#include <stdarg.h>

//===========================================================================
// Simple built-in tracing to a file...

#ifdef _FILETRACE

void COPOSRemoteOrderDisplay::WriteTrace( LPCTSTR pFormat, ... )
{
    static TCHAR s_TraceFile[] = _T("C:\\OposCCO_RemoteOrderDisplay.log");
    static BOOL s_bFirst = TRUE;
    if (s_bFirst)
    {
        ::DeleteFile(s_TraceFile);
        s_bFirst = FALSE;
    }
    FILE* fTrace = _tfopen( s_TraceFile, _T("a") );
    if (fTrace)
    {
        va_list marker;
        va_start(marker, pFormat);
        _vftprintf( fTrace, pFormat, marker );
        putc( '\n', fTrace );
        fclose(fTrace);
    }
}

//===========================================================================
// Higher performance tracing using RCS trace software...

#else

#define OPOSCCOCLASS COPOSRemoteOrderDisplay
#define RCSTRACENAME "OposCCO_RemoteOrderDisplay"
#include "UseRCSTrace.h"

#endif

//===========================================================================
// Trace support functions...

//## If error value is non-zero, convert to message and display.
void COPOSRemoteOrderDisplay::WriteTraceError( LPCTSTR pFormat, DWORD dwError )
{
    if ( dwError != 0 )
    {
        // Convert HRESULT to message and display.
        LPTSTR pMsgBuf;
        DWORD dwChars = ::FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dwError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &pMsgBuf,
                0,
                NULL );
        if ( dwChars == 0 )
            pMsgBuf = _T("[Unknown]");
        else if ( pMsgBuf[dwChars-1] == '\n' )
            pMsgBuf[dwChars-1] = '\0';
        WriteTrace( pFormat, dwError, pMsgBuf );
        if ( dwChars > 0 )
            ::LocalFree(pMsgBuf);
    }
}

//## Output methods that are not implemented in Service Object.
void COPOSRemoteOrderDisplay::WriteTraceMethods( DISPID* pDispID, int nMethods )
{
    for ( int i = 0; i < nMethods; i++, pDispID++ )
        if ( *pDispID == -123 )
            WriteTrace( _T("    %hs"), s_SOMethodNames[i] );
}

//## Output string.
void COPOSRemoteOrderDisplay::WriteTraceString( LPCTSTR pszPrefix, BSTR bstr )
{
    // Get string length.
    int nLen = ::SysStringLen(bstr);

    // If empty...
    if ( nLen == 0 )
        DOTRACEV( ( _T("%s Length 0 <>"), pszPrefix ) );

    // If not empty...
    else
    {
        // Check the BString for non-ascii characters, and output the string.
        int i;
        for ( i = 0; i < nLen; i++ )
            if ( bstr[i] < 0x20 || 0x7F < bstr[i] )
                break;
        WriteTraceString( bstr, nLen * 2, i == nLen, 2 /*wide string*/,
            _T("%s Length %u"), pszPrefix, nLen );

        // If binary conversion in effect...
        if ( _nBinaryConversion != OPOS_BC_NONE )
        {
            #define LONIBBLE(x) ((x) & 0x0F)
            #define HINIBBLE(x) ((x) & 0xF0)

            OLECHAR* pSource = bstr;                        // Get source pointer.
            LPBYTE pDest = new BYTE[ ( nLen + 1 ) / 2 + 1 ];// Create destination buffer.
            int nDestLen = nLen;
            i = 0;

            // If decimal binary conversion...
            if ( _nBinaryConversion == OPOS_BC_DECIMAL && nDestLen % 3 == 0 )
            {
                nDestLen /= 3;                              // Bytes to create.
                for ( ; i < nDestLen; i++ )                 // Loop through all data.
                {                                           //
                    if ( !isdigit(pSource[0]) || !isdigit(pSource[1]) || !isdigit(pSource[2]) )
                        break;
                    pDest[i] = (BYTE)                       //  - Set destination character to
                        ( ( LONIBBLE(pSource[0]) * 10 +     //      Hundreds digit * 100 +
                              LONIBBLE(pSource[1])          //      Tens     digit *  10 +
                            ) * 10 +                        //
                            LONIBBLE(pSource[2]) );         //      Units    digit.
                    pSource += 3;                           //  - Move on to next source pair.
                }
            }

            // If nibble binary conversion...
            else if ( nDestLen % 2 == 0 )
            {
                nDestLen /= 2;                              // Bytes to create.
                for ( ; i < nDestLen; i++ )                 // Loop through all data.
                {                                           //
                    if ( HINIBBLE(pSource[0]) != 0x30 || HINIBBLE(pSource[1]) != 0x30 )
                        break;
                    pDest[i] = (BYTE)                       //  - Set destination character to
                        ( ( LONIBBLE(pSource[0]) << 4 ) |   //      first char's bits 3-0 as bits 7-4 ORed with
                            LONIBBLE(pSource[1]) );         //      second char's bits 3-0 as bits 3-0.
                    pSource += 2;                           //  - Move on to next source pair.
                }
            }

            // If was a valid binary converted string...
            if ( i == nDestLen )
            {
                pDest[i] = '\0';                            // End the string with a NUL.

                // Check the converted buffer for non-ascii characters, and output the string.
                for ( i = 0; i < nDestLen; i++ )
                    if ( pDest[i] < 0x20 || 0x7F < pDest[i] )
                        break;
                WriteTraceString( pDest, nDestLen, i == nDestLen, 1 /*narrow string*/,
                    _T("%s Length %u [BinConv]"), pszPrefix, nLen );
            }

            delete [] pDest;                                // Delete the destination buffer.
        }
    }
}

//## Output string helper.
#define WTS_LINELEN_MAX 50
void COPOSRemoteOrderDisplay::WriteTraceString( void* pString, int nLen, BOOL bAscii, int nCharSize,
    LPCTSTR pszFormat, ... )
{
    TCHAR szBuffer[100+10];
    LPCTSTR pszPrintfString = ( nCharSize == 1 ? _T("hs") : _T("ls") );

    // Use the "list of arguments" version of printf.
    va_list va;
    va_start( va, pszFormat );
    _vsntprintf( szBuffer, 100, pszFormat, va );
    va_end(va);
    NULTERMINATE(szBuffer);

    // If all ascii, then output.
    if (bAscii)
    {
        // If less or equal to the maximum length we put out per line, then output on single line.
        if ( nLen <= WTS_LINELEN_MAX * nCharSize )
        {
            _tcscat( szBuffer, _T(" <%") );
            _tcscat( szBuffer, pszPrintfString );
            _tcscat( szBuffer, _T(">") );
            DOTRACEV( ( szBuffer, pString ) );
        }
        // If greater than maximum line length, then break into multiple lines.
        else
        {
            DOTRACEV( ( szBuffer, pString ) );
            _sntprintf( szBuffer, TSIZEOF(szBuffer), _T("  <%%.%u%s>"), WTS_LINELEN_MAX, pszPrintfString );
            NULTERMINATE(szBuffer);
            for ( int i = 0; i < nLen; i += WTS_LINELEN_MAX * nCharSize )
                DOTRACEV( ( szBuffer, (LPBYTE) pString + i ) );
        }
    }

    // If non-ascii data, then output.
    else
    {
        DOTRACEV( ( _T("%s [Contains non-ASCII]"), szBuffer ) );
        DODUMPV( pString, nLen );
    }
}
#endif

// End RemoteOrderDisplayImpl.cpp
