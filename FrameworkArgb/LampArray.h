/*++

Module Name:

    LampArray.h

Abstract:

    This file contains the queue definitions.

Environment:

    User-mode Driver Framework 2

--*/
#include "public.h"
#include <windows.h>
#include <wdf.h>
#include <hidport.h>  // located in $(DDK_INC_PATH)/wdm

#include "Device.h"

EXTERN_C_START

ULONG
GetLampArrayAttributesReport(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext,
    _In_        ULONG           MaxLen
);

ULONG
GetLampAttributesResponseReport(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext,
    _In_        ULONG           MaxLen
);


void SetLampAttributesId(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
);

void SetMultipleLamps(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
);

void SetLampRange(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
);

void SetAutonomousMode(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
);

EXTERN_C_END
