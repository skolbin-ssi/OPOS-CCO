/****************************************************************************
**
** FiscalPrinterImpl.cpp -- Main control implementation file.
**
**     Date                   Modification                          Author
** -----------|----------------------------------------------------|----------
**  1999/03/20 Initial development.                                 C. Monroe
**  2003/05/29 On Debug builds, add tracing of Open and Close to
**               C:\OposCCO_FiscalPrinter.log
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
#include "FiscalPrinter.h"

// Include this control's header.
#include "FiscalPrinterImpl.h"

// Include character <--> wide conversions header.
#include "CWString.h"

// Include the OPOS header.
#include "OposFptr.hi"

// TSIZEOF(x) returns characters in "x" bytes.
#define TSIZEOF(x)  (sizeof(x)/sizeof(TCHAR))

// Ensure that character array has NUL as last character.
#define NULTERMINATE(x) x[TSIZEOF(x) - 1] = '\0';


/////////////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////////////

// Hidden window class and window name.
TCHAR g_WindowName[] = _T("HW_OPOSFiscalPrinter");

// Control Object description and version.
char g_ControlDescription[] = "OPOS FiscalPrinter Control 1.16.000 [Public, by CRM/MCS, and modified by KF]" IFDEBUG(" [With Tracing]");
LONG g_ControlVersion =
    1 * 1000000 +
    16 * 1000 +
    000;

// Count of instances within this process.
int COPOSFiscalPrinter::s_nInstances = 0;

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
    "ClearOutput",
        #define nDIClearOutput 8
    "CloseService",
        #define nDICloseService 9
    "DirectIO",
        #define nDIDirectIO 10
    "ReleaseDevice",
        #define nDIReleaseDevice 11
    "BeginFiscalDocument",
        #define nDIBeginFiscalDocument 12
    "BeginFiscalReceipt",
        #define nDIBeginFiscalReceipt 13
    "BeginFixedOutput",
        #define nDIBeginFixedOutput 14
    "BeginInsertion",
        #define nDIBeginInsertion 15
    "BeginItemList",
        #define nDIBeginItemList 16
    "BeginNonFiscal",
        #define nDIBeginNonFiscal 17
    "BeginRemoval",
        #define nDIBeginRemoval 18
    "BeginTraining",
        #define nDIBeginTraining 19
    "ClearError",
        #define nDIClearError 20
    "EndFiscalDocument",
        #define nDIEndFiscalDocument 21
    "EndFiscalReceipt",
        #define nDIEndFiscalReceipt 22
    "EndFixedOutput",
        #define nDIEndFixedOutput 23
    "EndInsertion",
        #define nDIEndInsertion 24
    "EndItemList",
        #define nDIEndItemList 25
    "EndNonFiscal",
        #define nDIEndNonFiscal 26
    "EndRemoval",
        #define nDIEndRemoval 27
    "EndTraining",
        #define nDIEndTraining 28
    "GetData",
        #define nDIGetData 29
    "GetDate",
        #define nDIGetDate 30
    "GetTotalizer",
        #define nDIGetTotalizer 31
    "GetVatEntry",
        #define nDIGetVatEntry 32
    "PrintDuplicateReceipt",
        #define nDIPrintDuplicateReceipt 33
    "PrintFiscalDocumentLine",
        #define nDIPrintFiscalDocumentLine 34
    "PrintFixedOutput",
        #define nDIPrintFixedOutput 35
    "PrintNormal",
        #define nDIPrintNormal 36
    "PrintPeriodicTotalsReport",
        #define nDIPrintPeriodicTotalsReport 37
    "PrintPowerLossReport",
        #define nDIPrintPowerLossReport 38
    "PrintRecItem",
        #define nDIPrintRecItem 39
    "PrintRecItemAdjustment",
        #define nDIPrintRecItemAdjustment 40
    "PrintRecMessage",
        #define nDIPrintRecMessage 41
    "PrintRecNotPaid",
        #define nDIPrintRecNotPaid 42
    "PrintRecRefund",
        #define nDIPrintRecRefund 43
    "PrintRecSubtotal",
        #define nDIPrintRecSubtotal 44
    "PrintRecSubtotalAdjustment",
        #define nDIPrintRecSubtotalAdjustment 45
    "PrintRecTotal",
        #define nDIPrintRecTotal 46
    "PrintRecVoid",
        #define nDIPrintRecVoid 47
    "PrintRecVoidItem",
        #define nDIPrintRecVoidItem 48
    "PrintReport",
        #define nDIPrintReport 49
    "PrintXReport",
        #define nDIPrintXReport 50
    "PrintZReport",
        #define nDIPrintZReport 51
    "ResetPrinter",
        #define nDIResetPrinter 52
    "SetDate",
        #define nDISetDate 53
    "SetHeaderLine",
        #define nDISetHeaderLine 54
    "SetPOSID",
        #define nDISetPOSID 55
    "SetStoreFiscalID",
        #define nDISetStoreFiscalID 56
    "SetTrailerLine",
        #define nDISetTrailerLine 57
    "SetVatTable",
        #define nDISetVatTable 58
    "SetVatValue",
        #define nDISetVatValue 59
    "VerifyItem",
        #define nDIVerifyItem 60
// Release 1.4 methods
// Release 1.5 methods
// Release 1.6 methods
    "PrintRecCash",
        #define nDIPrintRecCash 61
    "PrintRecItemFuel",
        #define nDIPrintRecItemFuel 62
    "PrintRecItemFuelVoid",
        #define nDIPrintRecItemFuelVoid 63
    "PrintRecPackageAdjustment",
        #define nDIPrintRecPackageAdjustment 64
    "PrintRecPackageAdjustVoid",
        #define nDIPrintRecPackageAdjustVoid 65
    "PrintRecRefundVoid",
        #define nDIPrintRecRefundVoid 66
    "PrintRecSubtotalAdjustVoid",
        #define nDIPrintRecSubtotalAdjustVoid 67
    "PrintRecTaxID",
        #define nDIPrintRecTaxID 68
    "SetCurrency",
        #define nDISetCurrency 69
// Release 1.7 methods
// Release 1.8 methods
    "ResetStatistics",
        #define nDIResetStatistics 70
    "RetrieveStatistics",
        #define nDIRetrieveStatistics 71
    "UpdateStatistics",
        #define nDIUpdateStatistics 72
// Release 1.9 methods
    "CompareFirmwareVersion",
        #define nDICompareFirmwareVersion 73
    "UpdateFirmware",
        #define nDIUpdateFirmware 74
// Release 1.10 methods
// Release 1.11 methods
    "PrintRecItemAdjustmentVoid",
        #define nDIPrintRecItemAdjustmentVoid 75
    "PrintRecItemVoid",
        #define nDIPrintRecItemVoid 76
// Release 1.12 methods
    "PrintRecItemRefund",
        #define nDIPrintRecItemRefund 77
    "PrintRecItemRefundVoid",
        #define nDIPrintRecItemRefundVoid 78
    0
};

//===========================================================================
// Required method count table.
//   Contains count of methods from table above that must be present
//   for each release.
// Followed by the initial minor release of the OPOS FiscalPrinter.
//===========================================================================

static int s_RequiredMethodsPerRelease[] =
{
     61, // Release 1.3
     61, // Release 1.4
     61, // Release 1.5
     70, // Release 1.6
     70, // Release 1.7
     73, // Release 1.8
     75, // Release 1.9
     75, // Release 1.10
     77, // Release 1.11
     79  // Release 1.12
};

const int MinorReleaseInitial = 3;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Constructor / Destructor
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

COPOSFiscalPrinter::COPOSFiscalPrinter()
    IFDEBUG( : CProxy_IOPOSEvents< COPOSFiscalPrinter >(this) )
{
}

//===========================================================================

HRESULT COPOSFiscalPrinter::FinalConstruct()
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

COPOSFiscalPrinter::~COPOSFiscalPrinter()
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
STDMETHODIMP COPOSFiscalPrinter::FreezeEvents( BOOL bFreeze )
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
HRESULT COPOSFiscalPrinter::OnDraw( ATL_DRAWINFO& di )
{
    RECT& rc = *(RECT*)di.prcBounds;

    // Draw our bitmap...
    //  - Load bitmap and get info
    HBITMAP hBitmap = LoadBitmap(
        _Module.GetModuleInstance(),            // handle of the instance containing the image
        MAKEINTRESOURCE(IDB_OPOSFISCALPRINTER2) ); // bitmap resource name
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

HRESULT COPOSFiscalPrinter::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
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

HRESULT COPOSFiscalPrinter::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
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

HRESULT COPOSFiscalPrinter::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
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

HRESULT COPOSFiscalPrinter::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
    long nIndex, VARIANT_BOOL nValue, long nMinor )
{
    // If setting a boolean, then force to use VC++ TRUE or FALSE,
    //   since several vendors test specifically against TRUE (1)
    //   rather than non-zero.
    return SetOposProp( DEBUGPARAM(pPropName)
        nIndex, static_cast<long>(nValue ? TRUE : FALSE), nMinor );
}

//===========================================================================

HRESULT COPOSFiscalPrinter::GetOposProp( DEBUGPARAM(LPCSTR pPropName)
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

HRESULT COPOSFiscalPrinter::SetOposProp( DEBUGPARAM(LPCSTR pPropName)
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

HRESULT COPOSFiscalPrinter::DoInvoke( DEBUGPARAM(LPCSTR pFuncName)
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
        case VT_CY: DOTRACEV( ( _T("%sCURRENCY %s"), szParmNum, FormatCurrency(pVar->cyVal) ) ); break;
        case VT_BYREF|VT_CY: DOTRACEV( ( _T("%sCURRENCY* %s"), szParmNum, FormatCurrency(*pVar->pcyVal) ) ); break;
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
        case VT_BYREF|VT_CY: DOTRACEV( ( _T("%sCURRENCY* %s"), szParmNum, FormatCurrency(*pVar->pcyVal) ) ); break;
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

void COPOSFiscalPrinter::SetCOFreezeEvents( VARIANT_BOOL bFreeze )
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

STDMETHODIMP COPOSFiscalPrinter::Open( BSTR DeviceName, long *pRC )
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
    cw_strcpy( szBaseKey, OPOS_ROOTKEY "\\" OPOS_CLASSKEY_FPTR );
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
    Vars[2].SetString( _CW(OPOS_CLASSKEY_FPTR), hRC );
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

STDMETHODIMP COPOSFiscalPrinter::Close( long *pRC )
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

STDMETHODIMP COPOSFiscalPrinter::ClaimDevice( long Timeout, long *pRC )
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

STDMETHODIMP COPOSFiscalPrinter::ReleaseDevice( long *pRC )
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

STDMETHODIMP COPOSFiscalPrinter::SOProcessID( long *pProcessID )
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

STDMETHODIMP COPOSFiscalPrinter::CheckHealth( 
    /*[in]*/ LONG Level, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( Level );
    return DoInvoke( DEBUGPARAM("CheckHealth") S_OK, &Var, 1, nDICheckHealth, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::ClearOutput( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("ClearOutput") S_OK, NULL, 0, nDIClearOutput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::DirectIO( 
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

STDMETHODIMP COPOSFiscalPrinter::BeginFiscalDocument( 
    /*[in]*/ LONG DocumentAmount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( DocumentAmount );
    return DoInvoke( DEBUGPARAM("BeginFiscalDocument") S_OK, &Var, 1, nDIBeginFiscalDocument, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginFiscalReceipt( 
    /*[in]*/ VARIANT_BOOL PrintHeader, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetVARIANT_BOOL( PrintHeader );
    return DoInvoke( DEBUGPARAM("BeginFiscalReceipt") S_OK, &Var, 1, nDIBeginFiscalReceipt, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginFixedOutput( 
    /*[in]*/ LONG Station, 
    /*[in]*/ LONG DocumentType, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( Station );
    Vars[0].SetLONG( DocumentType );
    return DoInvoke( DEBUGPARAM("BeginFixedOutput") S_OK, Vars, 2, nDIBeginFixedOutput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginInsertion( 
    /*[in]*/ LONG Timeout, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( Timeout );
    return DoInvoke( DEBUGPARAM("BeginInsertion") S_OK, &Var, 1, nDIBeginInsertion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginItemList( 
    /*[in]*/ LONG VatID, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( VatID );
    return DoInvoke( DEBUGPARAM("BeginItemList") S_OK, &Var, 1, nDIBeginItemList, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginNonFiscal( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("BeginNonFiscal") S_OK, NULL, 0, nDIBeginNonFiscal, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginRemoval( 
    /*[in]*/ LONG Timeout, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( Timeout );
    return DoInvoke( DEBUGPARAM("BeginRemoval") S_OK, &Var, 1, nDIBeginRemoval, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::BeginTraining( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("BeginTraining") S_OK, NULL, 0, nDIBeginTraining, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::ClearError( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("ClearError") S_OK, NULL, 0, nDIClearError, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndFiscalDocument( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndFiscalDocument") S_OK, NULL, 0, nDIEndFiscalDocument, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndFiscalReceipt( 
    /*[in]*/ VARIANT_BOOL PrintHeader, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetVARIANT_BOOL( PrintHeader );
    return DoInvoke( DEBUGPARAM("EndFiscalReceipt") S_OK, &Var, 1, nDIEndFiscalReceipt, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndFixedOutput( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndFixedOutput") S_OK, NULL, 0, nDIEndFixedOutput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndInsertion( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndInsertion") S_OK, NULL, 0, nDIEndInsertion, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndItemList( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndItemList") S_OK, NULL, 0, nDIEndItemList, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndNonFiscal( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndNonFiscal") S_OK, NULL, 0, nDIEndNonFiscal, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndRemoval( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndRemoval") S_OK, NULL, 0, nDIEndRemoval, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::EndTraining( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("EndTraining") S_OK, NULL, 0, nDIEndTraining, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::GetData( 
    /*[in]*/ LONG DataItem, 
    /*[out]*/ LONG* OptArgs, 
    /*[out]*/ BSTR* Data, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( DataItem );
    Vars[1].SetLONG_Ptr( OptArgs, hRC );
    Vars[0].SetBSTR_OutPtr( Data, hRC );
    return DoInvoke( DEBUGPARAM("GetData") hRC, Vars, 3, nDIGetData, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::GetDate( 
    /*[out]*/ BSTR* Date, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR_OutPtr( Date, hRC );
    return DoInvoke( DEBUGPARAM("GetDate") hRC, &Var, 1, nDIGetDate, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::GetTotalizer( 
    /*[in]*/ LONG VatID, 
    /*[in]*/ LONG OptArgs, 
    /*[out]*/ BSTR* Data, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( VatID );
    Vars[1].SetLONG( OptArgs );
    Vars[0].SetBSTR_OutPtr( Data, hRC );
    return DoInvoke( DEBUGPARAM("GetTotalizer") hRC, Vars, 3, nDIGetTotalizer, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::GetVatEntry( 
    /*[in]*/ LONG VatID, 
    /*[in]*/ LONG OptArgs, 
    /*[out]*/ LONG* VatRate, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( VatID );
    Vars[1].SetLONG( OptArgs );
    Vars[0].SetLONG_Ptr( VatRate, hRC );
    return DoInvoke( DEBUGPARAM("GetVatEntry") hRC, Vars, 3, nDIGetVatEntry, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintDuplicateReceipt( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("PrintDuplicateReceipt") S_OK, NULL, 0, nDIPrintDuplicateReceipt, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintFiscalDocumentLine( 
    /*[in]*/ BSTR DocumentLine, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( DocumentLine, hRC );
    return DoInvoke( DEBUGPARAM("PrintFiscalDocumentLine") hRC, &Var, 1, nDIPrintFiscalDocumentLine, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintFixedOutput( 
    /*[in]*/ LONG DocumentType, 
    /*[in]*/ LONG LineNumber, 
    /*[in]*/ BSTR Data, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( DocumentType );
    Vars[1].SetLONG( LineNumber );
    Vars[0].SetBSTR( Data, hRC );
    return DoInvoke( DEBUGPARAM("PrintFixedOutput") hRC, Vars, 3, nDIPrintFixedOutput, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintNormal( 
    /*[in]*/ LONG Station, 
    /*[in]*/ BSTR Data, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetLONG( Station );
    Vars[0].SetBSTR( Data, hRC );
    return DoInvoke( DEBUGPARAM("PrintNormal") hRC, Vars, 2, nDIPrintNormal, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintPeriodicTotalsReport( 
    /*[in]*/ BSTR Date1, 
    /*[in]*/ BSTR Date2, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetBSTR( Date1, hRC );
    Vars[0].SetBSTR( Date2, hRC );
    return DoInvoke( DEBUGPARAM("PrintPeriodicTotalsReport") hRC, Vars, 2, nDIPrintPeriodicTotalsReport, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintPowerLossReport( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("PrintPowerLossReport") S_OK, NULL, 0, nDIPrintPowerLossReport, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItem( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Price, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY UnitPrice, 
    /*[in]*/ BSTR UnitName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    HRESULT hRC = S_OK;
    Vars[5].SetBSTR( Description, hRC );
    Vars[4].SetCURRENCY( Price );
    Vars[3].SetLONG( Quantity );
    Vars[2].SetLONG( VatInfo );
    Vars[1].SetCURRENCY( UnitPrice );
    Vars[0].SetBSTR( UnitName, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecItem") hRC, Vars, 6, nDIPrintRecItem, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemAdjustment( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG VatInfo, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[4];
    HRESULT hRC = S_OK;
    Vars[3].SetLONG( AdjustmentType );
    Vars[2].SetBSTR( Description, hRC );
    Vars[1].SetCURRENCY( Amount );
    Vars[0].SetLONG( VatInfo );
    return DoInvoke( DEBUGPARAM("PrintRecItemAdjustment") hRC, Vars, 4, nDIPrintRecItemAdjustment, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecMessage( 
    /*[in]*/ BSTR Message, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( Message, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecMessage") hRC, &Var, 1, nDIPrintRecMessage, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecNotPaid( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetBSTR( Description, hRC );
    Vars[0].SetCURRENCY( Amount );
    return DoInvoke( DEBUGPARAM("PrintRecNotPaid") hRC, Vars, 2, nDIPrintRecNotPaid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecRefund( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG VatInfo, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetBSTR( Description, hRC );
    Vars[1].SetCURRENCY( Amount );
    Vars[0].SetLONG( VatInfo );
    return DoInvoke( DEBUGPARAM("PrintRecRefund") hRC, Vars, 3, nDIPrintRecRefund, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecSubtotal( 
    /*[in]*/ CURRENCY Amount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetCURRENCY( Amount );
    return DoInvoke( DEBUGPARAM("PrintRecSubtotal") S_OK, &Var, 1, nDIPrintRecSubtotal, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecSubtotalAdjustment( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( AdjustmentType );
    Vars[1].SetBSTR( Description, hRC );
    Vars[0].SetCURRENCY( Amount );
    return DoInvoke( DEBUGPARAM("PrintRecSubtotalAdjustment") hRC, Vars, 3, nDIPrintRecSubtotalAdjustment, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecTotal( 
    /*[in]*/ CURRENCY Total, 
    /*[in]*/ CURRENCY Payment, 
    /*[in]*/ BSTR Description, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetCURRENCY( Total );
    Vars[1].SetCURRENCY( Payment );
    Vars[0].SetBSTR( Description, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecTotal") hRC, Vars, 3, nDIPrintRecTotal, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecVoid( 
    /*[in]*/ BSTR Description, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( Description, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecVoid") hRC, &Var, 1, nDIPrintRecVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecVoidItem( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ CURRENCY Adjustment, 
    /*[in]*/ LONG VatInfo, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    HRESULT hRC = S_OK;
    Vars[5].SetBSTR( Description, hRC );
    Vars[4].SetCURRENCY( Amount );
    Vars[3].SetLONG( Quantity );
    Vars[2].SetLONG( AdjustmentType );
    Vars[1].SetCURRENCY( Adjustment );
    Vars[0].SetLONG( VatInfo );
    return DoInvoke( DEBUGPARAM("PrintRecVoidItem") hRC, Vars, 6, nDIPrintRecVoidItem, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintReport( 
    /*[in]*/ LONG ReportType, 
    /*[in]*/ BSTR StartNum, 
    /*[in]*/ BSTR EndNum, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( ReportType );
    Vars[1].SetBSTR( StartNum, hRC );
    Vars[0].SetBSTR( EndNum, hRC );
    return DoInvoke( DEBUGPARAM("PrintReport") hRC, Vars, 3, nDIPrintReport, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintXReport( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("PrintXReport") S_OK, NULL, 0, nDIPrintXReport, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintZReport( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("PrintZReport") S_OK, NULL, 0, nDIPrintZReport, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::ResetPrinter( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("ResetPrinter") S_OK, NULL, 0, nDIResetPrinter, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetDate( 
    /*[in]*/ BSTR Date, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( Date, hRC );
    return DoInvoke( DEBUGPARAM("SetDate") hRC, &Var, 1, nDISetDate, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetHeaderLine( 
    /*[in]*/ LONG LineNumber, 
    /*[in]*/ BSTR Text, 
    /*[in]*/ VARIANT_BOOL DoubleWidth, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( LineNumber );
    Vars[1].SetBSTR( Text, hRC );
    Vars[0].SetVARIANT_BOOL( DoubleWidth );
    return DoInvoke( DEBUGPARAM("SetHeaderLine") hRC, Vars, 3, nDISetHeaderLine, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetPOSID( 
    /*[in]*/ BSTR POSID, 
    /*[in]*/ BSTR CashierID, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetBSTR( POSID, hRC );
    Vars[0].SetBSTR( CashierID, hRC );
    return DoInvoke( DEBUGPARAM("SetPOSID") hRC, Vars, 2, nDISetPOSID, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetStoreFiscalID( 
    /*[in]*/ BSTR ID, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( ID, hRC );
    return DoInvoke( DEBUGPARAM("SetStoreFiscalID") hRC, &Var, 1, nDISetStoreFiscalID, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetTrailerLine( 
    /*[in]*/ LONG LineNumber, 
    /*[in]*/ BSTR Text, 
    /*[in]*/ VARIANT_BOOL DoubleWidth, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( LineNumber );
    Vars[1].SetBSTR( Text, hRC );
    Vars[0].SetVARIANT_BOOL( DoubleWidth );
    return DoInvoke( DEBUGPARAM("SetTrailerLine") hRC, Vars, 3, nDISetTrailerLine, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetVatTable( 
    /*[out, retval]*/ long* pRC )
{
    return DoInvoke( DEBUGPARAM("SetVatTable") S_OK, NULL, 0, nDISetVatTable, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetVatValue( 
    /*[in]*/ LONG VatID, 
    /*[in]*/ BSTR VatValue, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetLONG( VatID );
    Vars[0].SetBSTR( VatValue, hRC );
    return DoInvoke( DEBUGPARAM("SetVatValue") hRC, Vars, 2, nDISetVatValue, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::VerifyItem( 
    /*[in]*/ BSTR ItemName, 
    /*[in]*/ LONG VatID, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetBSTR( ItemName, hRC );
    Vars[0].SetLONG( VatID );
    return DoInvoke( DEBUGPARAM("VerifyItem") hRC, Vars, 2, nDIVerifyItem, pRC );
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

STDMETHODIMP COPOSFiscalPrinter::PrintRecCash( 
    /*[in]*/ CURRENCY Amount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetCURRENCY( Amount );
    return DoInvoke( DEBUGPARAM("PrintRecCash") S_OK, &Var, 1, nDIPrintRecCash, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemFuel( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Price, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY UnitPrice, 
    /*[in]*/ BSTR UnitName, 
    /*[in]*/ CURRENCY SpecialTax, 
    /*[in]*/ BSTR SpecialTaxName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[8];
    HRESULT hRC = S_OK;
    Vars[7].SetBSTR( Description, hRC );
    Vars[6].SetCURRENCY( Price );
    Vars[5].SetLONG( Quantity );
    Vars[4].SetLONG( VatInfo );
    Vars[3].SetCURRENCY( UnitPrice );
    Vars[2].SetBSTR( UnitName, hRC );
    Vars[1].SetCURRENCY( SpecialTax );
    Vars[0].SetBSTR( SpecialTaxName, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecItemFuel") hRC, Vars, 8, nDIPrintRecItemFuel, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemFuelVoid( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Price, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY SpecialTax, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[4];
    HRESULT hRC = S_OK;
    Vars[3].SetBSTR( Description, hRC );
    Vars[2].SetCURRENCY( Price );
    Vars[1].SetLONG( VatInfo );
    Vars[0].SetCURRENCY( SpecialTax );
    return DoInvoke( DEBUGPARAM("PrintRecItemFuelVoid") hRC, Vars, 4, nDIPrintRecItemFuelVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecPackageAdjustment( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ BSTR Description, 
    /*[in]*/ BSTR VatAdjustment, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetLONG( AdjustmentType );
    Vars[1].SetBSTR( Description, hRC );
    Vars[0].SetBSTR( VatAdjustment, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecPackageAdjustment") hRC, Vars, 3, nDIPrintRecPackageAdjustment, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecPackageAdjustVoid( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ BSTR VatAdjustment, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    HRESULT hRC = S_OK;
    Vars[1].SetLONG( AdjustmentType );
    Vars[0].SetBSTR( VatAdjustment, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecPackageAdjustVoid") hRC, Vars, 2, nDIPrintRecPackageAdjustVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecRefundVoid( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG VatInfo, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[3];
    HRESULT hRC = S_OK;
    Vars[2].SetBSTR( Description, hRC );
    Vars[1].SetCURRENCY( Amount );
    Vars[0].SetLONG( VatInfo );
    return DoInvoke( DEBUGPARAM("PrintRecRefundVoid") hRC, Vars, 3, nDIPrintRecRefundVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecSubtotalAdjustVoid( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ CURRENCY Amount, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[2];
    Vars[1].SetLONG( AdjustmentType );
    Vars[0].SetCURRENCY( Amount );
    return DoInvoke( DEBUGPARAM("PrintRecSubtotalAdjustVoid") S_OK, Vars, 2, nDIPrintRecSubtotalAdjustVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecTaxID( 
    /*[in]*/ BSTR TaxID, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( TaxID, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecTaxID") hRC, &Var, 1, nDIPrintRecTaxID, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::SetCurrency( 
    /*[in]*/ LONG NewCurrency, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    Var.SetLONG( NewCurrency );
    return DoInvoke( DEBUGPARAM("SetCurrency") S_OK, &Var, 1, nDISetCurrency, pRC );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.7
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.8
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::ResetStatistics( 
    /*[in]*/ BSTR StatisticsBuffer, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( StatisticsBuffer, hRC );
    return DoInvoke( DEBUGPARAM("ResetStatistics") hRC, &Var, 1, nDIResetStatistics, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::RetrieveStatistics( 
    /*[in, out]*/ BSTR* pStatisticsBuffer, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR_InOutPtr( pStatisticsBuffer, hRC );
    return DoInvoke( DEBUGPARAM("RetrieveStatistics") hRC, &Var, 1, nDIRetrieveStatistics, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::UpdateStatistics( 
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

STDMETHODIMP COPOSFiscalPrinter::CompareFirmwareVersion( 
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

STDMETHODIMP COPOSFiscalPrinter::UpdateFirmware( 
    /*[in]*/ BSTR FirmwareFileName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Var;
    HRESULT hRC = S_OK;
    Var.SetBSTR( FirmwareFileName, hRC );
    return DoInvoke( DEBUGPARAM("UpdateFirmware") hRC, &Var, 1, nDIUpdateFirmware, pRC );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.10
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.11
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemAdjustmentVoid( 
    /*[in]*/ LONG AdjustmentType, 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG VatInfo, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[4];
    HRESULT hRC = S_OK;
    Vars[3].SetLONG( AdjustmentType );
    Vars[2].SetBSTR( Description, hRC );
    Vars[1].SetCURRENCY( Amount );
    Vars[0].SetLONG( VatInfo );
    return DoInvoke( DEBUGPARAM("PrintRecItemAdjustmentVoid") hRC, Vars, 4, nDIPrintRecItemAdjustmentVoid, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemVoid( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Price, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY UnitPrice, 
    /*[in]*/ BSTR UnitName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    HRESULT hRC = S_OK;
    Vars[5].SetBSTR( Description, hRC );
    Vars[4].SetCURRENCY( Price );
    Vars[3].SetLONG( Quantity );
    Vars[2].SetLONG( VatInfo );
    Vars[1].SetCURRENCY( UnitPrice );
    Vars[0].SetBSTR( UnitName, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecItemVoid") hRC, Vars, 6, nDIPrintRecItemVoid, pRC );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.12
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemRefund( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY UnitAmount, 
    /*[in]*/ BSTR UnitName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    HRESULT hRC = S_OK;
    Vars[5].SetBSTR( Description, hRC );
    Vars[4].SetCURRENCY( Amount );
    Vars[3].SetLONG( Quantity );
    Vars[2].SetLONG( VatInfo );
    Vars[1].SetCURRENCY( UnitAmount );
    Vars[0].SetBSTR( UnitName, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecItemRefund") hRC, Vars, 6, nDIPrintRecItemRefund, pRC );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::PrintRecItemRefundVoid( 
    /*[in]*/ BSTR Description, 
    /*[in]*/ CURRENCY Amount, 
    /*[in]*/ LONG Quantity, 
    /*[in]*/ LONG VatInfo, 
    /*[in]*/ CURRENCY UnitAmount, 
    /*[in]*/ BSTR UnitName, 
    /*[out, retval]*/ long* pRC )
{
    OposVariant Vars[6];
    HRESULT hRC = S_OK;
    Vars[5].SetBSTR( Description, hRC );
    Vars[4].SetCURRENCY( Amount );
    Vars[3].SetLONG( Quantity );
    Vars[2].SetLONG( VatInfo );
    Vars[1].SetCURRENCY( UnitAmount );
    Vars[0].SetBSTR( UnitName, hRC );
    return DoInvoke( DEBUGPARAM("PrintRecItemRefundVoid") hRC, Vars, 6, nDIPrintRecItemRefundVoid, pRC );
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

STDMETHODIMP COPOSFiscalPrinter::get_FreezeEvents( VARIANT_BOOL *pFreezeEvents )
{
    return GetOposProp( DEBUGPARAM("FreezeEvents") PIDX_FreezeEvents, pFreezeEvents );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::put_FreezeEvents( VARIANT_BOOL FreezeEvents )
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

STDMETHODIMP COPOSFiscalPrinter::get_ResultCode( long *pResultCode )
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

STDMETHODIMP COPOSFiscalPrinter::get_State( long *pState )
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

STDMETHODIMP COPOSFiscalPrinter::get_ControlObjectDescription( BSTR *pControlObjectDescription )
{
    *pControlObjectDescription = ::SysAllocString( _CW(g_ControlDescription) );
    DOTRACEV( ( _T("*ControlObjectDescription [Get]: <%hs>"), g_ControlDescription ) );
    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ControlObjectVersion( long *pControlObjectVersion )
{
    *pControlObjectVersion = g_ControlVersion;
    DOTRACEV( ( _T("*ControlObjectVersion [Get]: %d"), g_ControlVersion ) );
    return S_OK;
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_OpenResult( long *pOpenResult )
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

STDMETHODIMP COPOSFiscalPrinter::get_BinaryConversion( /*[out, retval]*/ LONG* pBinaryConversion )
{
    return GetOposProp( DEBUGPARAM("BinaryConversion") PIDX_BinaryConversion, pBinaryConversion, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_BinaryConversion( /*[in]*/ LONG BinaryConversion )
{
    IFDEBUG( _nBinaryConversion = BinaryConversion);
    return SetOposProp( DEBUGPARAM("BinaryConversion") PIDX_BinaryConversion, BinaryConversion, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPowerReporting( /*[out, retval]*/ LONG* pCapPowerReporting )
{
    return GetOposProp( DEBUGPARAM("CapPowerReporting") PIDX_CapPowerReporting, pCapPowerReporting, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CheckHealthText( /*[out, retval]*/ BSTR* pCheckHealthText )
{
    return GetOposProp( DEBUGPARAM("CheckHealthText") PIDX_CheckHealthText, pCheckHealthText, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_Claimed( /*[out, retval]*/ VARIANT_BOOL* pClaimed )
{
    return GetOposProp( DEBUGPARAM("Claimed") PIDX_Claimed, pClaimed, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DeviceEnabled( /*[out, retval]*/ VARIANT_BOOL* pDeviceEnabled )
{
    return GetOposProp( DEBUGPARAM("DeviceEnabled") PIDX_DeviceEnabled, pDeviceEnabled, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_DeviceEnabled( /*[in]*/ VARIANT_BOOL DeviceEnabled )
{
    return SetOposProp( DEBUGPARAM("DeviceEnabled") PIDX_DeviceEnabled, DeviceEnabled, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_OutputID( /*[out, retval]*/ LONG* pOutputID )
{
    return GetOposProp( DEBUGPARAM("OutputID") PIDX_OutputID, pOutputID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PowerNotify( /*[out, retval]*/ LONG* pPowerNotify )
{
    return GetOposProp( DEBUGPARAM("PowerNotify") PIDX_PowerNotify, pPowerNotify, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_PowerNotify( /*[in]*/ LONG PowerNotify )
{
    return SetOposProp( DEBUGPARAM("PowerNotify") PIDX_PowerNotify, PowerNotify, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PowerState( /*[out, retval]*/ LONG* pPowerState )
{
    return GetOposProp( DEBUGPARAM("PowerState") PIDX_PowerState, pPowerState, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ResultCodeExtended( /*[out, retval]*/ LONG* pResultCodeExtended )
{
    return GetOposProp( DEBUGPARAM("ResultCodeExtended") PIDX_ResultCodeExtended, pResultCodeExtended, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ServiceObjectDescription( /*[out, retval]*/ BSTR* pServiceObjectDescription )
{
    return GetOposProp( DEBUGPARAM("ServiceObjectDescription") PIDX_ServiceObjectDescription, pServiceObjectDescription, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ServiceObjectVersion( /*[out, retval]*/ LONG* pServiceObjectVersion )
{
    return GetOposProp( DEBUGPARAM("ServiceObjectVersion") PIDX_ServiceObjectVersion, pServiceObjectVersion, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DeviceDescription( /*[out, retval]*/ BSTR* pDeviceDescription )
{
    return GetOposProp( DEBUGPARAM("DeviceDescription") PIDX_DeviceDescription, pDeviceDescription, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DeviceName( /*[out, retval]*/ BSTR* pDeviceName )
{
    return GetOposProp( DEBUGPARAM("DeviceName") PIDX_DeviceName, pDeviceName, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_AmountDecimalPlaces( /*[out, retval]*/ LONG* pAmountDecimalPlaces )
{
    return GetOposProp( DEBUGPARAM("AmountDecimalPlaces") PIDXFptr_AmountDecimalPlaces, pAmountDecimalPlaces, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_AsyncMode( /*[out, retval]*/ VARIANT_BOOL* pAsyncMode )
{
    return GetOposProp( DEBUGPARAM("AsyncMode") PIDXFptr_AsyncMode, pAsyncMode, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_AsyncMode( /*[in]*/ VARIANT_BOOL AsyncMode )
{
    return SetOposProp( DEBUGPARAM("AsyncMode") PIDXFptr_AsyncMode, AsyncMode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapAdditionalLines( /*[out, retval]*/ VARIANT_BOOL* pCapAdditionalLines )
{
    return GetOposProp( DEBUGPARAM("CapAdditionalLines") PIDXFptr_CapAdditionalLines, pCapAdditionalLines, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapAmountAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapAmountAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapAmountAdjustment") PIDXFptr_CapAmountAdjustment, pCapAmountAdjustment, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapAmountNotPaid( /*[out, retval]*/ VARIANT_BOOL* pCapAmountNotPaid )
{
    return GetOposProp( DEBUGPARAM("CapAmountNotPaid") PIDXFptr_CapAmountNotPaid, pCapAmountNotPaid, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapCheckTotal( /*[out, retval]*/ VARIANT_BOOL* pCapCheckTotal )
{
    return GetOposProp( DEBUGPARAM("CapCheckTotal") PIDXFptr_CapCheckTotal, pCapCheckTotal, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapCoverSensor( /*[out, retval]*/ VARIANT_BOOL* pCapCoverSensor )
{
    return GetOposProp( DEBUGPARAM("CapCoverSensor") PIDXFptr_CapCoverSensor, pCapCoverSensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapDoubleWidth( /*[out, retval]*/ VARIANT_BOOL* pCapDoubleWidth )
{
    return GetOposProp( DEBUGPARAM("CapDoubleWidth") PIDXFptr_CapDoubleWidth, pCapDoubleWidth, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapDuplicateReceipt( /*[out, retval]*/ VARIANT_BOOL* pCapDuplicateReceipt )
{
    return GetOposProp( DEBUGPARAM("CapDuplicateReceipt") PIDXFptr_CapDuplicateReceipt, pCapDuplicateReceipt, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapFixedOutput( /*[out, retval]*/ VARIANT_BOOL* pCapFixedOutput )
{
    return GetOposProp( DEBUGPARAM("CapFixedOutput") PIDXFptr_CapFixedOutput, pCapFixedOutput, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapHasVatTable( /*[out, retval]*/ VARIANT_BOOL* pCapHasVatTable )
{
    return GetOposProp( DEBUGPARAM("CapHasVatTable") PIDXFptr_CapHasVatTable, pCapHasVatTable, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapIndependentHeader( /*[out, retval]*/ VARIANT_BOOL* pCapIndependentHeader )
{
    return GetOposProp( DEBUGPARAM("CapIndependentHeader") PIDXFptr_CapIndependentHeader, pCapIndependentHeader, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapItemList( /*[out, retval]*/ VARIANT_BOOL* pCapItemList )
{
    return GetOposProp( DEBUGPARAM("CapItemList") PIDXFptr_CapItemList, pCapItemList, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapJrnEmptySensor( /*[out, retval]*/ VARIANT_BOOL* pCapJrnEmptySensor )
{
    return GetOposProp( DEBUGPARAM("CapJrnEmptySensor") PIDXFptr_CapJrnEmptySensor, pCapJrnEmptySensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapJrnNearEndSensor( /*[out, retval]*/ VARIANT_BOOL* pCapJrnNearEndSensor )
{
    return GetOposProp( DEBUGPARAM("CapJrnNearEndSensor") PIDXFptr_CapJrnNearEndSensor, pCapJrnNearEndSensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapJrnPresent( /*[out, retval]*/ VARIANT_BOOL* pCapJrnPresent )
{
    return GetOposProp( DEBUGPARAM("CapJrnPresent") PIDXFptr_CapJrnPresent, pCapJrnPresent, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapNonFiscalMode( /*[out, retval]*/ VARIANT_BOOL* pCapNonFiscalMode )
{
    return GetOposProp( DEBUGPARAM("CapNonFiscalMode") PIDXFptr_CapNonFiscalMode, pCapNonFiscalMode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapOrderAdjustmentFirst( /*[out, retval]*/ VARIANT_BOOL* pCapOrderAdjustmentFirst )
{
    return GetOposProp( DEBUGPARAM("CapOrderAdjustmentFirst") PIDXFptr_CapOrderAdjustmentFirst, pCapOrderAdjustmentFirst, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPercentAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapPercentAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapPercentAdjustment") PIDXFptr_CapPercentAdjustment, pCapPercentAdjustment, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPositiveAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapPositiveAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapPositiveAdjustment") PIDXFptr_CapPositiveAdjustment, pCapPositiveAdjustment, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPowerLossReport( /*[out, retval]*/ VARIANT_BOOL* pCapPowerLossReport )
{
    return GetOposProp( DEBUGPARAM("CapPowerLossReport") PIDXFptr_CapPowerLossReport, pCapPowerLossReport, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPredefinedPaymentLines( /*[out, retval]*/ VARIANT_BOOL* pCapPredefinedPaymentLines )
{
    return GetOposProp( DEBUGPARAM("CapPredefinedPaymentLines") PIDXFptr_CapPredefinedPaymentLines, pCapPredefinedPaymentLines, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapReceiptNotPaid( /*[out, retval]*/ VARIANT_BOOL* pCapReceiptNotPaid )
{
    return GetOposProp( DEBUGPARAM("CapReceiptNotPaid") PIDXFptr_CapReceiptNotPaid, pCapReceiptNotPaid, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapRecEmptySensor( /*[out, retval]*/ VARIANT_BOOL* pCapRecEmptySensor )
{
    return GetOposProp( DEBUGPARAM("CapRecEmptySensor") PIDXFptr_CapRecEmptySensor, pCapRecEmptySensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapRecNearEndSensor( /*[out, retval]*/ VARIANT_BOOL* pCapRecNearEndSensor )
{
    return GetOposProp( DEBUGPARAM("CapRecNearEndSensor") PIDXFptr_CapRecNearEndSensor, pCapRecNearEndSensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapRecPresent( /*[out, retval]*/ VARIANT_BOOL* pCapRecPresent )
{
    return GetOposProp( DEBUGPARAM("CapRecPresent") PIDXFptr_CapRecPresent, pCapRecPresent, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapRemainingFiscalMemory( /*[out, retval]*/ VARIANT_BOOL* pCapRemainingFiscalMemory )
{
    return GetOposProp( DEBUGPARAM("CapRemainingFiscalMemory") PIDXFptr_CapRemainingFiscalMemory, pCapRemainingFiscalMemory, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapReservedWord( /*[out, retval]*/ VARIANT_BOOL* pCapReservedWord )
{
    return GetOposProp( DEBUGPARAM("CapReservedWord") PIDXFptr_CapReservedWord, pCapReservedWord, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetHeader( /*[out, retval]*/ VARIANT_BOOL* pCapSetHeader )
{
    return GetOposProp( DEBUGPARAM("CapSetHeader") PIDXFptr_CapSetHeader, pCapSetHeader, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetPOSID( /*[out, retval]*/ VARIANT_BOOL* pCapSetPOSID )
{
    return GetOposProp( DEBUGPARAM("CapSetPOSID") PIDXFptr_CapSetPOSID, pCapSetPOSID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetStoreFiscalID( /*[out, retval]*/ VARIANT_BOOL* pCapSetStoreFiscalID )
{
    return GetOposProp( DEBUGPARAM("CapSetStoreFiscalID") PIDXFptr_CapSetStoreFiscalID, pCapSetStoreFiscalID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetTrailer( /*[out, retval]*/ VARIANT_BOOL* pCapSetTrailer )
{
    return GetOposProp( DEBUGPARAM("CapSetTrailer") PIDXFptr_CapSetTrailer, pCapSetTrailer, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetVatTable( /*[out, retval]*/ VARIANT_BOOL* pCapSetVatTable )
{
    return GetOposProp( DEBUGPARAM("CapSetVatTable") PIDXFptr_CapSetVatTable, pCapSetVatTable, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpEmptySensor( /*[out, retval]*/ VARIANT_BOOL* pCapSlpEmptySensor )
{
    return GetOposProp( DEBUGPARAM("CapSlpEmptySensor") PIDXFptr_CapSlpEmptySensor, pCapSlpEmptySensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpFiscalDocument( /*[out, retval]*/ VARIANT_BOOL* pCapSlpFiscalDocument )
{
    return GetOposProp( DEBUGPARAM("CapSlpFiscalDocument") PIDXFptr_CapSlpFiscalDocument, pCapSlpFiscalDocument, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpFullSlip( /*[out, retval]*/ VARIANT_BOOL* pCapSlpFullSlip )
{
    return GetOposProp( DEBUGPARAM("CapSlpFullSlip") PIDXFptr_CapSlpFullSlip, pCapSlpFullSlip, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpNearEndSensor( /*[out, retval]*/ VARIANT_BOOL* pCapSlpNearEndSensor )
{
    return GetOposProp( DEBUGPARAM("CapSlpNearEndSensor") PIDXFptr_CapSlpNearEndSensor, pCapSlpNearEndSensor, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpPresent( /*[out, retval]*/ VARIANT_BOOL* pCapSlpPresent )
{
    return GetOposProp( DEBUGPARAM("CapSlpPresent") PIDXFptr_CapSlpPresent, pCapSlpPresent, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSlpValidation( /*[out, retval]*/ VARIANT_BOOL* pCapSlpValidation )
{
    return GetOposProp( DEBUGPARAM("CapSlpValidation") PIDXFptr_CapSlpValidation, pCapSlpValidation, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSubAmountAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapSubAmountAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapSubAmountAdjustment") PIDXFptr_CapSubAmountAdjustment, pCapSubAmountAdjustment, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSubPercentAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapSubPercentAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapSubPercentAdjustment") PIDXFptr_CapSubPercentAdjustment, pCapSubPercentAdjustment, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSubtotal( /*[out, retval]*/ VARIANT_BOOL* pCapSubtotal )
{
    return GetOposProp( DEBUGPARAM("CapSubtotal") PIDXFptr_CapSubtotal, pCapSubtotal, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapTrainingMode( /*[out, retval]*/ VARIANT_BOOL* pCapTrainingMode )
{
    return GetOposProp( DEBUGPARAM("CapTrainingMode") PIDXFptr_CapTrainingMode, pCapTrainingMode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapValidateJournal( /*[out, retval]*/ VARIANT_BOOL* pCapValidateJournal )
{
    return GetOposProp( DEBUGPARAM("CapValidateJournal") PIDXFptr_CapValidateJournal, pCapValidateJournal, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapXReport( /*[out, retval]*/ VARIANT_BOOL* pCapXReport )
{
    return GetOposProp( DEBUGPARAM("CapXReport") PIDXFptr_CapXReport, pCapXReport, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CheckTotal( /*[out, retval]*/ VARIANT_BOOL* pCheckTotal )
{
    return GetOposProp( DEBUGPARAM("CheckTotal") PIDXFptr_CheckTotal, pCheckTotal, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_CheckTotal( /*[in]*/ VARIANT_BOOL CheckTotal )
{
    return SetOposProp( DEBUGPARAM("CheckTotal") PIDXFptr_CheckTotal, CheckTotal, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CountryCode( /*[out, retval]*/ LONG* pCountryCode )
{
    return GetOposProp( DEBUGPARAM("CountryCode") PIDXFptr_CountryCode, pCountryCode, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CoverOpen( /*[out, retval]*/ VARIANT_BOOL* pCoverOpen )
{
    return GetOposProp( DEBUGPARAM("CoverOpen") PIDXFptr_CoverOpen, pCoverOpen, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DayOpened( /*[out, retval]*/ VARIANT_BOOL* pDayOpened )
{
    return GetOposProp( DEBUGPARAM("DayOpened") PIDXFptr_DayOpened, pDayOpened, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DescriptionLength( /*[out, retval]*/ LONG* pDescriptionLength )
{
    return GetOposProp( DEBUGPARAM("DescriptionLength") PIDXFptr_DescriptionLength, pDescriptionLength, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DuplicateReceipt( /*[out, retval]*/ VARIANT_BOOL* pDuplicateReceipt )
{
    return GetOposProp( DEBUGPARAM("DuplicateReceipt") PIDXFptr_DuplicateReceipt, pDuplicateReceipt, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_DuplicateReceipt( /*[in]*/ VARIANT_BOOL DuplicateReceipt )
{
    return SetOposProp( DEBUGPARAM("DuplicateReceipt") PIDXFptr_DuplicateReceipt, DuplicateReceipt, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ErrorLevel( /*[out, retval]*/ LONG* pErrorLevel )
{
    return GetOposProp( DEBUGPARAM("ErrorLevel") PIDXFptr_ErrorLevel, pErrorLevel, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ErrorOutID( /*[out, retval]*/ LONG* pErrorOutID )
{
    return GetOposProp( DEBUGPARAM("ErrorOutID") PIDXFptr_ErrorOutID, pErrorOutID, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ErrorState( /*[out, retval]*/ LONG* pErrorState )
{
    return GetOposProp( DEBUGPARAM("ErrorState") PIDXFptr_ErrorState, pErrorState, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ErrorStation( /*[out, retval]*/ LONG* pErrorStation )
{
    return GetOposProp( DEBUGPARAM("ErrorStation") PIDXFptr_ErrorStation, pErrorStation, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ErrorString( /*[out, retval]*/ BSTR* pErrorString )
{
    return GetOposProp( DEBUGPARAM("ErrorString") PIDXFptr_ErrorString, pErrorString, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_FlagWhenIdle( /*[out, retval]*/ VARIANT_BOOL* pFlagWhenIdle )
{
    return GetOposProp( DEBUGPARAM("FlagWhenIdle") PIDXFptr_FlagWhenIdle, pFlagWhenIdle, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_FlagWhenIdle( /*[in]*/ VARIANT_BOOL FlagWhenIdle )
{
    return SetOposProp( DEBUGPARAM("FlagWhenIdle") PIDXFptr_FlagWhenIdle, FlagWhenIdle, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_JrnEmpty( /*[out, retval]*/ VARIANT_BOOL* pJrnEmpty )
{
    return GetOposProp( DEBUGPARAM("JrnEmpty") PIDXFptr_JrnEmpty, pJrnEmpty, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_JrnNearEnd( /*[out, retval]*/ VARIANT_BOOL* pJrnNearEnd )
{
    return GetOposProp( DEBUGPARAM("JrnNearEnd") PIDXFptr_JrnNearEnd, pJrnNearEnd, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_MessageLength( /*[out, retval]*/ LONG* pMessageLength )
{
    return GetOposProp( DEBUGPARAM("MessageLength") PIDXFptr_MessageLength, pMessageLength, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_NumHeaderLines( /*[out, retval]*/ LONG* pNumHeaderLines )
{
    return GetOposProp( DEBUGPARAM("NumHeaderLines") PIDXFptr_NumHeaderLines, pNumHeaderLines, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_NumTrailerLines( /*[out, retval]*/ LONG* pNumTrailerLines )
{
    return GetOposProp( DEBUGPARAM("NumTrailerLines") PIDXFptr_NumTrailerLines, pNumTrailerLines, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_NumVatRates( /*[out, retval]*/ LONG* pNumVatRates )
{
    return GetOposProp( DEBUGPARAM("NumVatRates") PIDXFptr_NumVatRates, pNumVatRates, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PredefinedPaymentLines( /*[out, retval]*/ BSTR* pPredefinedPaymentLines )
{
    return GetOposProp( DEBUGPARAM("PredefinedPaymentLines") PIDXFptr_PredefinedPaymentLines, pPredefinedPaymentLines, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PrinterState( /*[out, retval]*/ LONG* pPrinterState )
{
    return GetOposProp( DEBUGPARAM("PrinterState") PIDXFptr_PrinterState, pPrinterState, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_QuantityDecimalPlaces( /*[out, retval]*/ LONG* pQuantityDecimalPlaces )
{
    return GetOposProp( DEBUGPARAM("QuantityDecimalPlaces") PIDXFptr_QuantityDecimalPlaces, pQuantityDecimalPlaces, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_QuantityLength( /*[out, retval]*/ LONG* pQuantityLength )
{
    return GetOposProp( DEBUGPARAM("QuantityLength") PIDXFptr_QuantityLength, pQuantityLength, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_RecEmpty( /*[out, retval]*/ VARIANT_BOOL* pRecEmpty )
{
    return GetOposProp( DEBUGPARAM("RecEmpty") PIDXFptr_RecEmpty, pRecEmpty, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_RecNearEnd( /*[out, retval]*/ VARIANT_BOOL* pRecNearEnd )
{
    return GetOposProp( DEBUGPARAM("RecNearEnd") PIDXFptr_RecNearEnd, pRecNearEnd, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_RemainingFiscalMemory( /*[out, retval]*/ LONG* pRemainingFiscalMemory )
{
    return GetOposProp( DEBUGPARAM("RemainingFiscalMemory") PIDXFptr_RemainingFiscalMemory, pRemainingFiscalMemory, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ReservedWord( /*[out, retval]*/ BSTR* pReservedWord )
{
    return GetOposProp( DEBUGPARAM("ReservedWord") PIDXFptr_ReservedWord, pReservedWord, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_SlipSelection( /*[out, retval]*/ LONG* pSlipSelection )
{
    return GetOposProp( DEBUGPARAM("SlipSelection") PIDXFptr_SlipSelection, pSlipSelection, 3 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_SlipSelection( /*[in]*/ LONG SlipSelection )
{
    return SetOposProp( DEBUGPARAM("SlipSelection") PIDXFptr_SlipSelection, SlipSelection, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_SlpEmpty( /*[out, retval]*/ VARIANT_BOOL* pSlpEmpty )
{
    return GetOposProp( DEBUGPARAM("SlpEmpty") PIDXFptr_SlpEmpty, pSlpEmpty, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_SlpNearEnd( /*[out, retval]*/ VARIANT_BOOL* pSlpNearEnd )
{
    return GetOposProp( DEBUGPARAM("SlpNearEnd") PIDXFptr_SlpNearEnd, pSlpNearEnd, 3 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_TrainingModeActive( /*[out, retval]*/ VARIANT_BOOL* pTrainingModeActive )
{
    return GetOposProp( DEBUGPARAM("TrainingModeActive") PIDXFptr_TrainingModeActive, pTrainingModeActive, 3 /*MinorVersion*/ );
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

STDMETHODIMP COPOSFiscalPrinter::get_ActualCurrency( /*[out, retval]*/ LONG* pActualCurrency )
{
    return GetOposProp( DEBUGPARAM("ActualCurrency") PIDXFptr_ActualCurrency, pActualCurrency, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_AdditionalHeader( /*[out, retval]*/ BSTR* pAdditionalHeader )
{
    return GetOposProp( DEBUGPARAM("AdditionalHeader") PIDXFptr_AdditionalHeader, pAdditionalHeader, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_AdditionalHeader( /*[in]*/ BSTR AdditionalHeader )
{
    return SetOposProp( DEBUGPARAM("AdditionalHeader") PIDXFptr_AdditionalHeader, AdditionalHeader, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_AdditionalTrailer( /*[out, retval]*/ BSTR* pAdditionalTrailer )
{
    return GetOposProp( DEBUGPARAM("AdditionalTrailer") PIDXFptr_AdditionalTrailer, pAdditionalTrailer, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_AdditionalTrailer( /*[in]*/ BSTR AdditionalTrailer )
{
    return SetOposProp( DEBUGPARAM("AdditionalTrailer") PIDXFptr_AdditionalTrailer, AdditionalTrailer, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapAdditionalHeader( /*[out, retval]*/ VARIANT_BOOL* pCapAdditionalHeader )
{
    return GetOposProp( DEBUGPARAM("CapAdditionalHeader") PIDXFptr_CapAdditionalHeader, pCapAdditionalHeader, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapAdditionalTrailer( /*[out, retval]*/ VARIANT_BOOL* pCapAdditionalTrailer )
{
    return GetOposProp( DEBUGPARAM("CapAdditionalTrailer") PIDXFptr_CapAdditionalTrailer, pCapAdditionalTrailer, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapChangeDue( /*[out, retval]*/ VARIANT_BOOL* pCapChangeDue )
{
    return GetOposProp( DEBUGPARAM("CapChangeDue") PIDXFptr_CapChangeDue, pCapChangeDue, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapEmptyReceiptIsVoidable( /*[out, retval]*/ VARIANT_BOOL* pCapEmptyReceiptIsVoidable )
{
    return GetOposProp( DEBUGPARAM("CapEmptyReceiptIsVoidable") PIDXFptr_CapEmptyReceiptIsVoidable, pCapEmptyReceiptIsVoidable, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapFiscalReceiptStation( /*[out, retval]*/ VARIANT_BOOL* pCapFiscalReceiptStation )
{
    return GetOposProp( DEBUGPARAM("CapFiscalReceiptStation") PIDXFptr_CapFiscalReceiptStation, pCapFiscalReceiptStation, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapFiscalReceiptType( /*[out, retval]*/ VARIANT_BOOL* pCapFiscalReceiptType )
{
    return GetOposProp( DEBUGPARAM("CapFiscalReceiptType") PIDXFptr_CapFiscalReceiptType, pCapFiscalReceiptType, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapMultiContractor( /*[out, retval]*/ VARIANT_BOOL* pCapMultiContractor )
{
    return GetOposProp( DEBUGPARAM("CapMultiContractor") PIDXFptr_CapMultiContractor, pCapMultiContractor, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapOnlyVoidLastItem( /*[out, retval]*/ VARIANT_BOOL* pCapOnlyVoidLastItem )
{
    return GetOposProp( DEBUGPARAM("CapOnlyVoidLastItem") PIDXFptr_CapOnlyVoidLastItem, pCapOnlyVoidLastItem, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPackageAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapPackageAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapPackageAdjustment") PIDXFptr_CapPackageAdjustment, pCapPackageAdjustment, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapPostPreLine( /*[out, retval]*/ VARIANT_BOOL* pCapPostPreLine )
{
    return GetOposProp( DEBUGPARAM("CapPostPreLine") PIDXFptr_CapPostPreLine, pCapPostPreLine, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapSetCurrency( /*[out, retval]*/ VARIANT_BOOL* pCapSetCurrency )
{
    return GetOposProp( DEBUGPARAM("CapSetCurrency") PIDXFptr_CapSetCurrency, pCapSetCurrency, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapTotalizerType( /*[out, retval]*/ VARIANT_BOOL* pCapTotalizerType )
{
    return GetOposProp( DEBUGPARAM("CapTotalizerType") PIDXFptr_CapTotalizerType, pCapTotalizerType, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ChangeDue( /*[out, retval]*/ BSTR* pChangeDue )
{
    return GetOposProp( DEBUGPARAM("ChangeDue") PIDXFptr_ChangeDue, pChangeDue, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_ChangeDue( /*[in]*/ BSTR ChangeDue )
{
    return SetOposProp( DEBUGPARAM("ChangeDue") PIDXFptr_ChangeDue, ChangeDue, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_ContractorId( /*[out, retval]*/ LONG* pContractorId )
{
    return GetOposProp( DEBUGPARAM("ContractorId") PIDXFptr_ContractorId, pContractorId, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_ContractorId( /*[in]*/ LONG ContractorId )
{
    return SetOposProp( DEBUGPARAM("ContractorId") PIDXFptr_ContractorId, ContractorId, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_DateType( /*[out, retval]*/ LONG* pDateType )
{
    return GetOposProp( DEBUGPARAM("DateType") PIDXFptr_DateType, pDateType, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_DateType( /*[in]*/ LONG DateType )
{
    return SetOposProp( DEBUGPARAM("DateType") PIDXFptr_DateType, DateType, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_FiscalReceiptStation( /*[out, retval]*/ LONG* pFiscalReceiptStation )
{
    return GetOposProp( DEBUGPARAM("FiscalReceiptStation") PIDXFptr_FiscalReceiptStation, pFiscalReceiptStation, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_FiscalReceiptStation( /*[in]*/ LONG FiscalReceiptStation )
{
    return SetOposProp( DEBUGPARAM("FiscalReceiptStation") PIDXFptr_FiscalReceiptStation, FiscalReceiptStation, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_FiscalReceiptType( /*[out, retval]*/ LONG* pFiscalReceiptType )
{
    return GetOposProp( DEBUGPARAM("FiscalReceiptType") PIDXFptr_FiscalReceiptType, pFiscalReceiptType, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_FiscalReceiptType( /*[in]*/ LONG FiscalReceiptType )
{
    return SetOposProp( DEBUGPARAM("FiscalReceiptType") PIDXFptr_FiscalReceiptType, FiscalReceiptType, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_MessageType( /*[out, retval]*/ LONG* pMessageType )
{
    return GetOposProp( DEBUGPARAM("MessageType") PIDXFptr_MessageType, pMessageType, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_MessageType( /*[in]*/ LONG MessageType )
{
    return SetOposProp( DEBUGPARAM("MessageType") PIDXFptr_MessageType, MessageType, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PostLine( /*[out, retval]*/ BSTR* pPostLine )
{
    return GetOposProp( DEBUGPARAM("PostLine") PIDXFptr_PostLine, pPostLine, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_PostLine( /*[in]*/ BSTR PostLine )
{
    return SetOposProp( DEBUGPARAM("PostLine") PIDXFptr_PostLine, PostLine, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_PreLine( /*[out, retval]*/ BSTR* pPreLine )
{
    return GetOposProp( DEBUGPARAM("PreLine") PIDXFptr_PreLine, pPreLine, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_PreLine( /*[in]*/ BSTR PreLine )
{
    return SetOposProp( DEBUGPARAM("PreLine") PIDXFptr_PreLine, PreLine, 6 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_TotalizerType( /*[out, retval]*/ LONG* pTotalizerType )
{
    return GetOposProp( DEBUGPARAM("TotalizerType") PIDXFptr_TotalizerType, pTotalizerType, 6 /*MinorVersion*/ );
}

//---------------------------------------------------------------------------

STDMETHODIMP COPOSFiscalPrinter::put_TotalizerType( /*[in]*/ LONG TotalizerType )
{
    return SetOposProp( DEBUGPARAM("TotalizerType") PIDXFptr_TotalizerType, TotalizerType, 6 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.7
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.8
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::get_CapStatisticsReporting( /*[out, retval]*/ VARIANT_BOOL* pCapStatisticsReporting )
{
    return GetOposProp( DEBUGPARAM("CapStatisticsReporting") PIDX_CapStatisticsReporting, pCapStatisticsReporting, 8 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapUpdateStatistics( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateStatistics )
{
    return GetOposProp( DEBUGPARAM("CapUpdateStatistics") PIDX_CapUpdateStatistics, pCapUpdateStatistics, 8 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.9
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::get_CapCompareFirmwareVersion( /*[out, retval]*/ VARIANT_BOOL* pCapCompareFirmwareVersion )
{
    return GetOposProp( DEBUGPARAM("CapCompareFirmwareVersion") PIDX_CapCompareFirmwareVersion, pCapCompareFirmwareVersion, 9 /*MinorVersion*/ );
}

//===========================================================================

STDMETHODIMP COPOSFiscalPrinter::get_CapUpdateFirmware( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateFirmware )
{
    return GetOposProp( DEBUGPARAM("CapUpdateFirmware") PIDX_CapUpdateFirmware, pCapUpdateFirmware, 9 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.10
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.11
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

STDMETHODIMP COPOSFiscalPrinter::get_CapPositiveSubtotalAdjustment( /*[out, retval]*/ VARIANT_BOOL* pCapPositiveSubtotalAdjustment )
{
    return GetOposProp( DEBUGPARAM("CapPositiveSubtotalAdjustment") PIDXFptr_CapPositiveSubtotalAdjustment, pCapPositiveSubtotalAdjustment, 11 /*MinorVersion*/ );
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!! Release 1.12
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


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

void COPOSFiscalPrinter::WriteTrace( LPCTSTR pFormat, ... )
{
    static TCHAR s_TraceFile[] = _T("C:\\OposCCO_FiscalPrinter.log");
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

#define OPOSCCOCLASS COPOSFiscalPrinter
#define RCSTRACENAME "OposCCO_FiscalPrinter"
#include "UseRCSTrace.h"

#endif

//===========================================================================
// Trace support functions...

//## If error value is non-zero, convert to message and display.
void COPOSFiscalPrinter::WriteTraceError( LPCTSTR pFormat, DWORD dwError )
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
void COPOSFiscalPrinter::WriteTraceMethods( DISPID* pDispID, int nMethods )
{
    for ( int i = 0; i < nMethods; i++, pDispID++ )
        if ( *pDispID == -123 )
            WriteTrace( _T("    %hs"), s_SOMethodNames[i] );
}

//## Output string.
void COPOSFiscalPrinter::WriteTraceString( LPCTSTR pszPrefix, BSTR bstr )
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
void COPOSFiscalPrinter::WriteTraceString( void* pString, int nLen, BOOL bAscii, int nCharSize,
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

//## Output currency helper. Don't use floating point.
//    Examples: -123456 => "-12.3456"
//              123     => "0.0123"
//              -1      => "-0.0001"
LPCTSTR COPOSFiscalPrinter::FormatCurrency( CY cyCurrency )
{
    static TCHAR s_pszCurrency[30];
    LONGLONG nn = cyCurrency.int64 >= 0 ? cyCurrency.int64 : -cyCurrency.int64;
    int i;
    for ( i = 29; i > 24 || nn != 0; )
    {
        if ( i == 24+1 )
            s_pszCurrency[--i] = '.';
        s_pszCurrency[--i] = (TCHAR)( nn % 10 ) + '0';
        nn /= 10;
    }
    if ( cyCurrency.int64 < 0 )
        s_pszCurrency[--i] = '-';
    return &s_pszCurrency[i];
}
#endif

// End FiscalPrinterImpl.cpp
