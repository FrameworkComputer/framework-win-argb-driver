/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    User-mode Driver Framework 2

--*/

#include "public.h"
#include <hidport.h>  // located in $(DDK_INC_PATH)/wdm


typedef UCHAR HID_REPORT_DESCRIPTOR, * PHID_REPORT_DESCRIPTOR;


//
// These are the device attributes returned by the mini driver in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//
#define FWK_ARGB_HID_VID        0x32AC
#define FWK_ARGB_HID_PID        0x0033
#define FWK_ARGB_HID_VERSION    0x0101

EXTERN_C_START

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    WDFDEVICE               Device;
    WDFQUEUE                DefaultQueue;
    WDFQUEUE                ManualQueue;
    HID_DEVICE_ATTRIBUTES   HidDeviceAttributes;
    BYTE                    DeviceData;
    HID_DESCRIPTOR          HidDescriptor;
    PHID_REPORT_DESCRIPTOR  ReportDescriptor;
    BOOLEAN                 ReadReportDescFromRegistry;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
FrameworkArgbCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EXTERN_C_END
