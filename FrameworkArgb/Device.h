/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    User-mode Driver Framework 2

--*/
#pragma once
#include "public.h"
#include <windows.h>
#include <wdf.h>
#include <hidport.h>  // located in $(DDK_INC_PATH)/wdm
#include "LampArrayStructs.h"

EXTERN_C_START

typedef UCHAR HID_REPORT_DESCRIPTOR, * PHID_REPORT_DESCRIPTOR;

DRIVER_INITIALIZE                   DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD           EvtDeviceAdd;
EVT_WDF_TIMER                       EvtTimerFunc;
EVT_WDF_OBJECT_CONTEXT_CLEANUP      FrameworkArgbEvtDeviceCleanup;

#define MAX_LAMPARRAY_LAMP_COUNT    256
//#define LAMPARRAY_WIDTH             80000   // 80mm
//#define LAMPARRAY_HEIGHT            80000   // 80mm
//#define LAMPARRAY_DEPTH             20000   // 20mm
#define LAMPARRAY_KIND              0x07    // LampArrayKindChassis
#define LAMPARRAY_UPDATE_INTERVAL   100000  // 10ms

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
    HANDLE                  CrosEcHandle;
    UINT16                  CurrentLampId;
	BOOLEAN  		        AutonomousMode;
    UINT16                  LampCount;
    UINT32                  Width;
    UINT32                  Height;
    UINT32                  Depth;
    Position                LampPositions[MAX_LAMPARRAY_LAMP_COUNT];
    HID_DESCRIPTOR          HidDescriptor;
    PHID_REPORT_DESCRIPTOR  ReportDescriptor;
} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

//
// This macro will generate an inline function called GetDeviceContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

typedef struct _QUEUE_CONTEXT
{
    WDFQUEUE                Queue;
    PDEVICE_CONTEXT         DeviceContext;
    UCHAR                   OutputReport;

} QUEUE_CONTEXT, * PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, GetQueueContext);

NTSTATUS
QueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
);

typedef struct _MANUAL_QUEUE_CONTEXT
{
    WDFQUEUE                Queue;
    PDEVICE_CONTEXT         DeviceContext;
    WDFTIMER                Timer;

} MANUAL_QUEUE_CONTEXT, * PMANUAL_QUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(MANUAL_QUEUE_CONTEXT, GetManualQueueContext);

NTSTATUS
ManualQueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
);

NTSTATUS
ReadReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request,
    _Always_(_Out_)
    BOOLEAN* CompleteRequest
);

NTSTATUS
WriteReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
);

NTSTATUS
GetFeature(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
);

NTSTATUS
SetFeature(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
);

NTSTATUS
GetInputReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
);

NTSTATUS
SetOutputReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
);

NTSTATUS
GetString(
    _In_  WDFREQUEST        Request
);

NTSTATUS
GetIndexedString(
    _In_  WDFREQUEST        Request
);

NTSTATUS
GetStringId(
    _In_  WDFREQUEST        Request,
    _Out_ ULONG* StringId,
    _Out_ ULONG* LanguageId
);

NTSTATUS
RequestCopyFromBuffer(
    _In_  WDFREQUEST        Request,
    _In_  PVOID             SourceBuffer,
    _When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
    _In_  size_t            NumBytesToCopyFrom
);

NTSTATUS
RequestGetHidXferPacket_ToReadFromDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET* Packet
);

NTSTATUS
RequestGetHidXferPacket_ToWriteToDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET* Packet
);

NTSTATUS
CheckRegistryForLedConfig(
    _In_ WDFDEVICE Device
);

NTSTATUS
ReadLedConfigFromRegistry(
    _In_  WDFDEVICE Device,
    _Out_ UINT16 *LampCount,
    _Out_ UINT8 *LedArrangement
);

//
// Misc definitions
//
#define CONTROL_FEATURE_REPORT_ID   0x01

//
// These are the device attributes returned by the mini driver in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//
#define FWK_ARGB_HID_VID        0x32AC
#define FWK_ARGB_HID_PID        0x0033
#define FWK_ARGB_HID_VERSION    0x0100

#define LAMP_ARRAY_ATTRIBUTES_REPORT_ID    0x01
#define LAMP_ATTRIBUTES_REQUEST_REPORT_ID  0x02
#define LAMP_ATTRIBUTES_RESPONSE_REPORT_ID 0x03
#define LAMP_MULTI_UPDATE_REPORT_ID        0x04
#define LAMP_RANGE_UPDATE_REPORT_ID        0x05
#define LAMP_ARRAY_CONTROL_REPORT_ID       0x06
#define CONTROL_COLLECTION_REPORT_ID       0x07

#define MAXIMUM_STRING_LENGTH           (126 * sizeof(WCHAR))
#define FWK_ARGB_MANUFACTURER_STRING    L"Framework"
#define FWK_ARGB_PRODUCT_STRING         L"ARGB Driver"
#define FWK_ARGB_SERIAL_NUMBER_STRING   L""
#define FWK_ARGB_DEVICE_STRING          L"ARGB Device"
#define FWK_ARGB_DEVICE_STRING_INDEX    5

//
// SetFeature request requires that the feature report buffer size be exactly 
// same as the size of report described in the hid report descriptor (
// excluding the report ID). Since FWK_ARGB_CONTROL_INFO includes report ID,
// we subtract one from the size.
//
#define FEATURE_REPORT_SIZE_CB      ((USHORT)(sizeof(FWK_ARGB_CONTROL_INFO) - 1))
#define INPUT_REPORT_SIZE_CB        ((USHORT)(sizeof(FWK_ARGB_INPUT_REPORT) - 1))
#define OUTPUT_REPORT_SIZE_CB       ((USHORT)(sizeof(FWK_ARGB_OUTPUT_REPORT) - 1))

//
// Function to initialize the device and its callbacks
//
NTSTATUS
FrameworkArgbCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EXTERN_C_END
