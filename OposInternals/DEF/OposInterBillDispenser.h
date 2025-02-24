//*************************************************************************
//** OPOS BillDispenser Internals
//
//   Bill Acceptor header file for OPOS Controls and Service Objects.
//
//*************************************************************************

typedef [helpstring("OPOS BillDispenser Internals")]
enum
{
    PIDXBdsp_AsyncMode               = 28001,
    PIDXBdsp_AsyncResultCode         = 28002,
    PIDXBdsp_AsyncResultCodeExtended = 28003,
    PIDXBdsp_CurrentExit             = 28004,
    PIDXBdsp_DeviceExits             = 28005,
    PIDXBdsp_DeviceStatus            = 28006,

    PIDXBdsp_CapDiscrepancy      = 28501,
    PIDXBdsp_CapFullSensor       = 28502,
    PIDXBdsp_CapJamSensor        = 28503,
    PIDXBdsp_CapNearFullSensor   = 28504,

    PIDXBdsp_CurrencyCashList    = 1028001,
    PIDXBdsp_CurrencyCode        = 1028002,
    PIDXBdsp_CurrencyCodeList    = 1028003,
    PIDXBdsp_ExitCashList        = 1028004
} OPOSBillDispenserInternals;
