/****************************************************************************
**
** CashChangerImpl.h -- Main control header file.
**
**     Date                   Modification                          Author
** -----------|----------------------------------------------------|----------
**  1999/03/20 Initial version.                                     C. Monroe
**  2004/03/22 Add significantly more tracing when Debug.
**  2005/12/14 Add Debug tracing of event delivery.
**  2008/01/15 Enhance handling of interfaces/vtables.
**               Add more parameter and error checking.
**  2008/08/30 More interface updates.
**  2013/09/29 Update license section.
**  2015/02/07 Update license section.
**  2015/02/07 Version 1.14.001.
**  2020/06/01 Version 1.15.000.                                    K. Fukuchi
**             Changed include guard from #ifndef __XXXX_H__ to #pragma once.
**  2021/12/02 Version 1.16.000.
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

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Debug build tracing macros.
/////////////////////////////////////////////////////////////////////////////

//~~~~ When Debug, will perform some tracing ~~~~
#ifdef _DEBUG
// Default is to use writes using RCS Trace components.
//   Source code does not include the files to recompile to use them.
//#define _FILETRACE // Uncomment to use C runtime writes to a file.
    #define DEBUGPARAM(x) x,
    #define IFDEBUG(x) x
    #define DOTRACE(x) WriteTrace x
    #define DOTRACESTRINGV(x,y) WriteTraceString(x,y)
    #define DOTRACEERR(x,y) WriteTraceError(x,y)
    #define DOTRACEMETHODS(x,y) WriteTraceMethods(x,y)

    #ifdef _FILETRACE
        #pragma message("===>>> Debug Build: Slow tracing to C:\\OposCCO_CashChanger.log")
        #define DOTRACEV(x) WriteTrace x
        #define DODUMPV(x,y)            // Dump of non-ASCII data not implemented.
        #define TRACEINIT
        #define TRACEEXIT
    #else
        #pragma message("===>>> Debug Build: Fast tracing to OposCCO_CashChanger.log")
        #define DOTRACEV(x) WriteTraceV x
        #define DODUMPV(x,y) WriteDumpV(x,y)
        #define TRACEINIT TraceInit()
        #define TRACEEXIT TraceExit()
    #endif

//~~~~ When Release mode, all trace calls between no-ops ~~~~
#else
    #define DEBUGPARAM(x)
    #define IFDEBUG(x)
    #define DOTRACE(x)
    #define DOTRACEV(x)
    #define DOTRACESTRINGV(x,y)
    #define DOTRACEERR(x,y)
    #define DOTRACEMETHODS(x,y)
    #define TRACEINIT
    #define TRACEEXIT
#endif


/////////////////////////////////////////////////////////////////////////////
// Includes.
/////////////////////////////////////////////////////////////////////////////

// Include various definitions.
#include "CashChangerDefs.h"

// Standard includes...
#include "Resource.h"

// Include our variant class.
#include "OposVariant.h"

// Include our connection point implementation.
#include "CashChangerCP.h"


/////////////////////////////////////////////////////////////////////////////
// Data.
/////////////////////////////////////////////////////////////////////////////

#define SO_DISP_COUNT 25
// Hidden window class and window name.
extern TCHAR g_WindowName[];
// Control Object description.
extern char  g_ControlDescription[];


/////////////////////////////////////////////////////////////////////////////
// Control class declaration.
/////////////////////////////////////////////////////////////////////////////

// The class...
class ATL_NO_VTABLE COPOSCashChanger :
    // Inherit from all these classes...
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IOPOSCashChanger, &IID_IOPOSCashChanger, &LIBID_OposCashChanger_CCO>,
    public CComControl<COPOSCashChanger>,
    public IPersistStreamInitImpl<COPOSCashChanger>,
    public IOleControlImpl<COPOSCashChanger>,
    public IOleObjectImpl<COPOSCashChanger>,
    public IOleInPlaceActiveObjectImpl<COPOSCashChanger>,
    public IViewObjectExImpl<COPOSCashChanger>,
    public IOleInPlaceObjectWindowlessImpl<COPOSCashChanger>,
    public IConnectionPointContainerImpl<COPOSCashChanger>,
    public IPersistStorageImpl<COPOSCashChanger>,
    public ISpecifyPropertyPagesImpl<COPOSCashChanger>,
    public IQuickActivateImpl<COPOSCashChanger>,
    public IDataObjectImpl<COPOSCashChanger>,
    public IProvideClassInfo2Impl<&CLSID_OPOSCashChanger, &DIID__IOPOSCashChangerEvents, &LIBID_OposCashChanger_CCO>,
    public IObjectSafetyImpl<COPOSCashChanger, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
    public IPropertyNotifySinkCP<COPOSCashChanger>,
    public CComCoClass<COPOSCashChanger, &CLSID_OPOSCashChanger>,
    public CProxy_IOPOSEvents< COPOSCashChanger >
{
public:
// Constructor / FinalConstruct / Destructor.
    COPOSCashChanger();
    HRESULT FinalConstruct();
    ~COPOSCashChanger();

//===========================================================================
// Data.
//===========================================================================

protected:
    // Are we open?  And what is the status text of our last open?
    bool _bOpened;
    long _nOpenResult;

    // Service Object's version numbers. Set during Open.
    long _nSOMajor;                     // Service Object major version.
    long _nSOMinor;                     // Service Object minor version.

    // Special-case Control Object maintained result code. Use for:
    //  - Open result code.
    //  - Tried to access SO method or property not supported by its version.
    long _nCOResultCode;                // CO-maintained result code.
    long SetRC( long nRC = -1 )
        { return _nCOResultCode = nRC; }

    // Dispatch pointer to the Service Object. Valid while open.
    IDispatch* _pService;

    // Event freeze control.
    int _nCOFreezeEventCount;   // Depth of container freeze requests. Zero = not frozen.
    bool _bFreezeState;         // State of OPOS FreezeEvents property.

    // Maximum method index.
    long _nMaxMethod;

    // Service Object Dispatch IDs.
    DISPID _DispIDs[SO_DISP_COUNT];

    // Count of active instances.
    static int s_nInstances;

//===========================================================================
// Data and functions for tracing when Debug.
//===========================================================================

#ifdef _DEBUG
public:
    void* _pTrace;
    LONG _nBinaryConversion;
    void TraceInit();
    void TraceExit();
    void WriteTrace( LPCTSTR pFormat, ... );
    void WriteTraceV( LPCTSTR pFormat, ... );
    void WriteDumpV( void* pData, int nLen );
    void WriteTraceString( LPCTSTR pszPrefix, BSTR bstr );
    void WriteTraceString( void* pString, int nLen, BOOL bAscii, int nCharSize,
        LPCTSTR pszFormat, ... );
    void WriteTraceError( LPCTSTR pFormat, DWORD dwError );
    void WriteTraceMethods( DISPID* pDispID, int nMethods );
#endif

//===========================================================================
// Functions (Internal use).
//===========================================================================

protected:
    // Get or set properties.
    HRESULT GetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, long*         pnValue, long nMinor = 0 );
    HRESULT SetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, long          nValue,  long nMinor = 0 );
    HRESULT GetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, VARIANT_BOOL* pnValue, long nMinor = 0 );
    HRESULT SetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, VARIANT_BOOL  nValue,  long nMinor = 0 );
    HRESULT GetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, BSTR*         psValue, long nMinor = 0 );
    HRESULT SetOposProp( DEBUGPARAM(LPCSTR pPropName) long nIndex, BSTR          sValue,  long nMinor = 0 );

    // Call SO's Invoke method.
    HRESULT DoInvoke( DEBUGPARAM(LPCSTR pFuncName) HRESULT hRC, OposVariant* pParms, unsigned nParms, int nDispIDIndex,
        long* pRC, bool bOpenCheck = true );

    // Set the SO's event freeze state. Based on control container's freeze requests.
    void SetCOFreezeEvents( VARIANT_BOOL bFreeze );

    // Override the IOleControlImpl handler for container request to freeze/unfreeze events.
    STDMETHOD(FreezeEvents)(BOOL bFreeze);

    // Declare the draw handler.
    HRESULT OnDraw(ATL_DRAWINFO& di);

//===========================================================================
// Various ATL-generated macros.
//===========================================================================

public:
    // Registry resource.
DECLARE_REGISTRY_RESOURCEID(IDR_OPOSCASHCHANGER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Define supported interfaces.
BEGIN_COM_MAP(COPOSCashChanger)
    //  Map COM dispatch to the current interface implementation.
    COM_INTERFACE_ENTRY_IID(IID_IDispatch, IOPOSCashChanger)
    //   Map each COM interface to the current interface implementation.
    COM_INTERFACE_ENTRY_IID(IID_IOPOSCashChanger, IOPOSCashChanger)
    COM_INTERFACE_ENTRY_IID(IID_IOPOSCashChanger_1_11, IOPOSCashChanger)
    COM_INTERFACE_ENTRY_IID(IID_IOPOSCashChanger_1_9, IOPOSCashChanger)
    COM_INTERFACE_ENTRY_IID(IID_IOPOSCashChanger_1_8, IOPOSCashChanger)
    COM_INTERFACE_ENTRY_IID(IID_IOPOSCashChanger_1_5, IOPOSCashChanger)

    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

    // Define property map.
BEGIN_PROP_MAP(COPOSCashChanger)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
END_PROP_MAP()

    // Define connection points.
BEGIN_CONNECTION_POINT_MAP(COPOSCashChanger)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    CONNECTION_POINT_ENTRY(DIID__IOPOSCashChangerEvents)
END_CONNECTION_POINT_MAP()

    // Define message map.
BEGIN_MSG_MAP(COPOSCashChanger)
    CHAIN_MSG_MAP(CComControl<COPOSCashChanger>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

    // Define view status for IViewObjectEx to pass to container.
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

//===========================================================================
// Bring in all of the OPOS event firing code.
//===========================================================================

protected:
    #include "CashChangerEvent.h"

//===========================================================================
// Control methods and property declarations.
//===========================================================================

public:
    STDMETHOD(SOProcessID)( /*[out, retval]*/ long* pProcessID );

// Release 1.2
    STDMETHOD(get_OpenResult)( /*[out, retval]*/ LONG* pOpenResult );
    STDMETHOD(get_BinaryConversion)( /*[out, retval]*/ LONG* pBinaryConversion );
    STDMETHOD(put_BinaryConversion)( /*[in]*/ LONG BinaryConversion );
    STDMETHOD(get_CheckHealthText)( /*[out, retval]*/ BSTR* pCheckHealthText );
    STDMETHOD(get_Claimed)( /*[out, retval]*/ VARIANT_BOOL* pClaimed );
    STDMETHOD(get_DeviceEnabled)( /*[out, retval]*/ VARIANT_BOOL* pDeviceEnabled );
    STDMETHOD(put_DeviceEnabled)( /*[in]*/ VARIANT_BOOL DeviceEnabled );
    STDMETHOD(get_FreezeEvents)( /*[out, retval]*/ VARIANT_BOOL* pFreezeEvents );
    STDMETHOD(put_FreezeEvents)( /*[in]*/ VARIANT_BOOL FreezeEvents );
    STDMETHOD(get_ResultCode)( /*[out, retval]*/ LONG* pResultCode );
    STDMETHOD(get_ResultCodeExtended)( /*[out, retval]*/ LONG* pResultCodeExtended );
    STDMETHOD(get_State)( /*[out, retval]*/ LONG* pState );
    STDMETHOD(get_ControlObjectDescription)( /*[out, retval]*/ BSTR* pControlObjectDescription );
    STDMETHOD(get_ControlObjectVersion)( /*[out, retval]*/ LONG* pControlObjectVersion );
    STDMETHOD(get_ServiceObjectDescription)( /*[out, retval]*/ BSTR* pServiceObjectDescription );
    STDMETHOD(get_ServiceObjectVersion)( /*[out, retval]*/ LONG* pServiceObjectVersion );
    STDMETHOD(get_DeviceDescription)( /*[out, retval]*/ BSTR* pDeviceDescription );
    STDMETHOD(get_DeviceName)( /*[out, retval]*/ BSTR* pDeviceName );
    STDMETHOD(CheckHealth)( 
        /*[in]*/ LONG Level, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(ClaimDevice)( 
        /*[in]*/ LONG Timeout, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(Close)( 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(DirectIO)( 
        /*[in]*/ LONG Command, 
        /*[in, out]*/ LONG* pData, 
        /*[in, out]*/ BSTR* pString, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(Open)( 
        /*[in]*/ BSTR DeviceName, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(ReleaseDevice)( 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(get_AsyncMode)( /*[out, retval]*/ VARIANT_BOOL* pAsyncMode );
    STDMETHOD(put_AsyncMode)( /*[in]*/ VARIANT_BOOL AsyncMode );
    STDMETHOD(get_AsyncResultCode)( /*[out, retval]*/ LONG* pAsyncResultCode );
    STDMETHOD(get_AsyncResultCodeExtended)( /*[out, retval]*/ LONG* pAsyncResultCodeExtended );
    STDMETHOD(get_CapDiscrepancy)( /*[out, retval]*/ VARIANT_BOOL* pCapDiscrepancy );
    STDMETHOD(get_CapEmptySensor)( /*[out, retval]*/ VARIANT_BOOL* pCapEmptySensor );
    STDMETHOD(get_CapFullSensor)( /*[out, retval]*/ VARIANT_BOOL* pCapFullSensor );
    STDMETHOD(get_CapNearEmptySensor)( /*[out, retval]*/ VARIANT_BOOL* pCapNearEmptySensor );
    STDMETHOD(get_CapNearFullSensor)( /*[out, retval]*/ VARIANT_BOOL* pCapNearFullSensor );
    STDMETHOD(get_CurrencyCashList)( /*[out, retval]*/ BSTR* pCurrencyCashList );
    STDMETHOD(get_CurrencyCode)( /*[out, retval]*/ BSTR* pCurrencyCode );
    STDMETHOD(put_CurrencyCode)( /*[in]*/ BSTR CurrencyCode );
    STDMETHOD(get_CurrencyCodeList)( /*[out, retval]*/ BSTR* pCurrencyCodeList );
    STDMETHOD(get_CurrentExit)( /*[out, retval]*/ LONG* pCurrentExit );
    STDMETHOD(put_CurrentExit)( /*[in]*/ LONG CurrentExit );
    STDMETHOD(get_DeviceExits)( /*[out, retval]*/ LONG* pDeviceExits );
    STDMETHOD(get_DeviceStatus)( /*[out, retval]*/ LONG* pDeviceStatus );
    STDMETHOD(get_ExitCashList)( /*[out, retval]*/ BSTR* pExitCashList );
    STDMETHOD(get_FullStatus)( /*[out, retval]*/ LONG* pFullStatus );
    STDMETHOD(DispenseCash)( 
        /*[in]*/ BSTR CashCounts, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(DispenseChange)( 
        /*[in]*/ LONG Amount, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(ReadCashCounts)( 
        /*[out]*/ BSTR* pCashCounts, 
        /*[out]*/ VARIANT_BOOL* pDiscrepancy, 
        /*[out, retval]*/ long* pRC );
// Release 1.3
    STDMETHOD(get_CapPowerReporting)( /*[out, retval]*/ LONG* pCapPowerReporting );
    STDMETHOD(get_PowerNotify)( /*[out, retval]*/ LONG* pPowerNotify );
    STDMETHOD(put_PowerNotify)( /*[in]*/ LONG PowerNotify );
    STDMETHOD(get_PowerState)( /*[out, retval]*/ LONG* pPowerState );
// Release 1.4
// Release 1.5
    STDMETHOD(get_DataCount)( /*[out, retval]*/ LONG* pDataCount );
    STDMETHOD(get_DataEventEnabled)( /*[out, retval]*/ VARIANT_BOOL* pDataEventEnabled );
    STDMETHOD(put_DataEventEnabled)( /*[in]*/ VARIANT_BOOL DataEventEnabled );
    STDMETHOD(ClearInput)( 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(get_CapDeposit)( /*[out, retval]*/ VARIANT_BOOL* pCapDeposit );
    STDMETHOD(get_CapDepositDataEvent)( /*[out, retval]*/ VARIANT_BOOL* pCapDepositDataEvent );
    STDMETHOD(get_CapPauseDeposit)( /*[out, retval]*/ VARIANT_BOOL* pCapPauseDeposit );
    STDMETHOD(get_CapRepayDeposit)( /*[out, retval]*/ VARIANT_BOOL* pCapRepayDeposit );
    STDMETHOD(get_DepositAmount)( /*[out, retval]*/ LONG* pDepositAmount );
    STDMETHOD(get_DepositCashList)( /*[out, retval]*/ BSTR* pDepositCashList );
    STDMETHOD(get_DepositCodeList)( /*[out, retval]*/ BSTR* pDepositCodeList );
    STDMETHOD(get_DepositCounts)( /*[out, retval]*/ BSTR* pDepositCounts );
    STDMETHOD(get_DepositStatus)( /*[out, retval]*/ LONG* pDepositStatus );
    STDMETHOD(BeginDeposit)( 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(EndDeposit)( 
        /*[in]*/ LONG Success, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(FixDeposit)( 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(PauseDeposit)( 
        /*[in]*/ LONG Control, 
        /*[out, retval]*/ long* pRC );
// Release 1.6
// Release 1.7
// Release 1.8
    STDMETHOD(get_CapStatisticsReporting)( /*[out, retval]*/ VARIANT_BOOL* pCapStatisticsReporting );
    STDMETHOD(get_CapUpdateStatistics)( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateStatistics );
    STDMETHOD(ResetStatistics)( 
        /*[in]*/ BSTR StatisticsBuffer, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(RetrieveStatistics)( 
        /*[in, out]*/ BSTR* pStatisticsBuffer, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(UpdateStatistics)( 
        /*[in]*/ BSTR StatisticsBuffer, 
        /*[out, retval]*/ long* pRC );
// Release 1.9
    STDMETHOD(get_CapCompareFirmwareVersion)( /*[out, retval]*/ VARIANT_BOOL* pCapCompareFirmwareVersion );
    STDMETHOD(get_CapUpdateFirmware)( /*[out, retval]*/ VARIANT_BOOL* pCapUpdateFirmware );
    STDMETHOD(CompareFirmwareVersion)( 
        /*[in]*/ BSTR FirmwareFileName, 
        /*[out]*/ LONG* pResult, 
        /*[out, retval]*/ long* pRC );
    STDMETHOD(UpdateFirmware)( 
        /*[in]*/ BSTR FirmwareFileName, 
        /*[out, retval]*/ long* pRC );
// Release 1.10
// Release 1.11
    STDMETHOD(get_CapJamSensor)( /*[out, retval]*/ VARIANT_BOOL* pCapJamSensor );
    STDMETHOD(get_CapRealTimeData)( /*[out, retval]*/ VARIANT_BOOL* pCapRealTimeData );
    STDMETHOD(get_CurrentService)( /*[out, retval]*/ LONG* pCurrentService );
    STDMETHOD(put_CurrentService)( /*[in]*/ LONG CurrentService );
    STDMETHOD(get_RealTimeDataEnabled)( /*[out, retval]*/ VARIANT_BOOL* pRealTimeDataEnabled );
    STDMETHOD(put_RealTimeDataEnabled)( /*[in]*/ VARIANT_BOOL RealTimeDataEnabled );
    STDMETHOD(get_ServiceCount)( /*[out, retval]*/ LONG* pServiceCount );
    STDMETHOD(get_ServiceIndex)( /*[out, retval]*/ LONG* pServiceIndex );
    STDMETHOD(AdjustCashCounts)( 
        /*[in]*/ BSTR CashCounts, 
        /*[out, retval]*/ long* pRC );
// Release 1.12
// Release 1.13
// Release 1.14
// Release 1.15
};


// End CashChangerImpl.h
