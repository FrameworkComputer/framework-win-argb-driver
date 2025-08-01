/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "device.tmh"

//
// This is the default report descriptor for the virtual Hid device returned
// by the mini driver in response to IOCTL_HID_GET_REPORT_DESCRIPTOR.
//
HID_REPORT_DESCRIPTOR G_DefaultReportDescriptor[] = {
    0x05, 0x59,                                 // USAGE_PAGE (LightingAndIllumination)
    0x09, 0x01,                                 // USAGE (LampArray)
    0xa1, 0x01,                                 // COLLECTION (Application)
    0x85, LAMP_ARRAY_ATTRIBUTES_REPORT_ID,      //   REPORT_ID (1)
    0x09, 0x02,                                 //   USAGE (LampArrayAttributesReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x03,                                 //     USAGE (LampCount)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,               //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                                 //     REPORT_SIZE (16)
    0x95, 0x01,                                 //     REPORT_COUNT (1)
    0xb1, 0x03,                                 //     FEATURE (Cnst,Var,Abs)
    0x09, 0x04,                                 //     USAGE (BoundingBoxWidthInMicrometers)
    0x09, 0x05,                                 //     USAGE (BoundingBoxHeightInMicrometers)
    0x09, 0x06,                                 //     USAGE (BoundingBoxDepthInMicrometers)
    0x09, 0x07,                                 //     USAGE (LampArrayKind)
    0x09, 0x08,                                 //     USAGE (MinUpdateIntervalInMicroseconds)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0xff, 0x7f,               //     LOGICAL_MAXIMUM (2147483647)
    0x75, 0x20,                                 //     REPORT_SIZE (32)
    0x95, 0x05,                                 //     REPORT_COUNT (5)
    0xb1, 0x03,                                 //     FEATURE (Cnst,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0x85, LAMP_ATTRIBUTES_REQUEST_REPORT_ID,    //   REPORT_ID (2)
    0x09, 0x20,                                 //   USAGE (LampAttributesRequestReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x21,                                 //     USAGE (LampId)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,               //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                                 //     REPORT_SIZE (16)
    0x95, 0x01,                                 //     REPORT_COUNT (1)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0x85, LAMP_ATTRIBUTES_RESPONSE_REPORT_ID,   //   REPORT_ID (3)
    0x09, 0x22,                                 //   USAGE (LampAttributesResponseReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x21,                                 //     USAGE (LampId)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,               //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                                 //     REPORT_SIZE (16)
    0x95, 0x01,                                 //     REPORT_COUNT (1)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x23,                                 //     USAGE (PositionXInMicrometers)
    0x09, 0x24,                                 //     USAGE (PositionYInMicrometers)
    0x09, 0x25,                                 //     USAGE (PositionZInMicrometers)
    0x09, 0x27,                                 //     USAGE (UpdateLatencyInMicroseconds)
    0x09, 0x26,                                 //     USAGE (LampPurposes)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0xff, 0x7f,               //     LOGICAL_MAXIMUM (2147483647)
    0x75, 0x20,                                 //     REPORT_SIZE (32)
    0x95, 0x05,                                 //     REPORT_COUNT (5)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x28,                                 //     USAGE (RedLevelCount)
    0x09, 0x29,                                 //     USAGE (GreenLevelCount)
    0x09, 0x2a,                                 //     USAGE (BlueLevelCount)
    0x09, 0x2b,                                 //     USAGE (IntensityLevelCount)
    0x09, 0x2c,                                 //     USAGE (IsProgrammable)
    0x09, 0x2d,                                 //     USAGE (InputBinding)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                           //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x06,                                 //     REPORT_COUNT (6)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0x85, LAMP_MULTI_UPDATE_REPORT_ID,          //   REPORT_ID (4)
    0x09, 0x50,                                 //   USAGE (LampMultiUpdateReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x03,                                 //     USAGE (LampCount)
    0x09, 0x55,                                 //     USAGE (LampUpdateFlags)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x25, 0x08,                                 //     LOGICAL_MAXIMUM (8)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x02,                                 //     REPORT_COUNT (2)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x21,                                 //     USAGE (LampId)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,               //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                                 //     REPORT_SIZE (16)
    0x95, 0x08,                                 //     REPORT_COUNT (8)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                           //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x20,                                 //     REPORT_COUNT (32)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0x85, LAMP_RANGE_UPDATE_REPORT_ID,          //   REPORT_ID (5)
    0x09, 0x60,                                 //   USAGE (LampRangeUpdateReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x55,                                 //     USAGE (LampUpdateFlags)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x25, 0x08,                                 //     LOGICAL_MAXIMUM (8)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x01,                                 //     REPORT_COUNT (1)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x61,                                 //     USAGE (LampIdStart)
    0x09, 0x62,                                 //     USAGE (LampIdEnd)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,               //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                                 //     REPORT_SIZE (16)
    0x95, 0x02,                                 //     REPORT_COUNT (2)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0x09, 0x51,                                 //     USAGE (RedUpdateChannel)
    0x09, 0x52,                                 //     USAGE (GreenUpdateChannel)
    0x09, 0x53,                                 //     USAGE (BlueUpdateChannel)
    0x09, 0x54,                                 //     USAGE (IntensityUpdateChannel)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                           //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x04,                                 //     REPORT_COUNT (4)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0x85, LAMP_ARRAY_CONTROL_REPORT_ID,         //   REPORT_ID (6)
    0x09, 0x70,                                 //   USAGE (LampArrayControlReport)
    0xa1, 0x02,                                 //   COLLECTION (Logical)
    0x09, 0x71,                                 //     USAGE (AutonomousMode)
    0x15, 0x00,                                 //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                                 //     LOGICAL_MAXIMUM (1)
    0x75, 0x08,                                 //     REPORT_SIZE (8)
    0x95, 0x01,                                 //     REPORT_COUNT (1)
    0xb1, 0x02,                                 //     FEATURE (Data,Var,Abs)
    0xc0,                                       //   END_COLLECTION
    0xc0                                        // END_COLLECTION
};

//
// This is the default HID descriptor returned by the mini driver
// in response to IOCTL_HID_GET_DEVICE_DESCRIPTOR. The size
// of report descriptor is currently the size of G_DefaultReportDescriptor.
//

HID_DESCRIPTOR              G_DefaultHidDescriptor = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
    {                                       //DescriptorList[0]
        0x22,                               //report descriptor type 0x22
        sizeof(G_DefaultReportDescriptor)   //total length of report descriptor
    }
};

NTSTATUS
FrameworkArgbCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_CONTEXT         deviceContext;
    PHID_DEVICE_ATTRIBUTES  hidAttributes;
    WDFDEVICE               device;
    NTSTATUS                status;

    TraceInformation("%!FUNC! Entry");

    //
    // Mark ourselves as a filter, which also relinquishes power policy ownership
    //
    WdfFdoInitSetFilter(DeviceInit);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (!NT_SUCCESS(status)) {
        TraceError("WdfDeviceCreate failed %!STATUS!", status);
        return status;
    }

    //
    // Get a pointer to the device context structure that we just associated
    // with the device object. We define this structure in the device.h
    // header file. DeviceGetContext is an inline function generated by
    // using the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro in device.h.
    // This function will do the type checking and return the device context.
    // If you pass a wrong object handle it will return NULL and assert if
    // run under framework verifier mode.
    //
    deviceContext = GetDeviceContext(device);

    //
    // Initialize the context.
    //
    deviceContext->Device = device;
    deviceContext->DeviceData = 0;
    deviceContext->CurrentLampId = 0;
    deviceContext->AutonomousMode = TRUE;
    // 8 LEDs in a circle
    // z is 0 for all LEDs, they're all in the same plane
    // Bottom LED
    deviceContext->LampPositions[0].x = 40000;
    deviceContext->LampPositions[0].y = 0;
    deviceContext->LampPositions[1].x = 60000;
    deviceContext->LampPositions[1].y = 20000;
    // Right LED
    deviceContext->LampPositions[2].x = 80000;
    deviceContext->LampPositions[2].y = 40000;
    deviceContext->LampPositions[3].x = 60000;
    deviceContext->LampPositions[3].y = 60000;
    // Top LED
    deviceContext->LampPositions[4].x = 40000;
    deviceContext->LampPositions[4].y = 80000;
    deviceContext->LampPositions[5].x = 20000;
    deviceContext->LampPositions[5].y = 60000;
    // Left LED
    deviceContext->LampPositions[6].x = 0;
    deviceContext->LampPositions[6].y = 40000;
    deviceContext->LampPositions[7].x = 20000;
    deviceContext->LampPositions[7].y = 20000;

    hidAttributes = &deviceContext->HidDeviceAttributes;
    RtlZeroMemory(hidAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
    hidAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
    hidAttributes->VendorID = FWK_ARGB_HID_VID;
    hidAttributes->ProductID = FWK_ARGB_HID_PID;
    hidAttributes->VersionNumber = FWK_ARGB_HID_VERSION;

    status = QueueCreate(device,
        &deviceContext->DefaultQueue);
    if (!NT_SUCCESS(status)) {
        TraceError("QueueCreate failed %!STATUS!", status);
        return status;
    }

    status = ManualQueueCreate(device,
        &deviceContext->ManualQueue);
    if (!NT_SUCCESS(status)) {
        TraceError("ManualQueueCreate failed %!STATUS!", status);
        return status;
    }

    //
    // Use default "HID Descriptor" and "HID Report Descriptor" (hardcoded)
    //
    deviceContext->HidDescriptor = G_DefaultHidDescriptor;
    deviceContext->ReportDescriptor = G_DefaultReportDescriptor;

    return status;
}

NTSTATUS
GetStringId(
    _In_  WDFREQUEST        Request,
    _Out_ ULONG* StringId,
    _Out_ ULONG* LanguageId
)
/*++

Routine Description:

    Helper routine to decode IOCTL_HID_GET_INDEXED_STRING and IOCTL_HID_GET_STRING.

Arguments:

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    ULONG                   inputValue;

#ifdef _KERNEL_MODE

    WDF_REQUEST_PARAMETERS  requestParameters;

    //
    // IOCTL_HID_GET_STRING:                      // METHOD_NEITHER
    // IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT
    //
    // The string id (or string index) is passed in Parameters.DeviceIoControl.
    // Type3InputBuffer. However, Parameters.DeviceIoControl.InputBufferLength
    // was not initialized by hidclass.sys, therefore trying to access the
    // buffer with WdfRequestRetrieveInputMemory will fail
    //
    // Another problem with IOCTL_HID_GET_INDEXED_STRING is that METHOD_OUT_DIRECT
    // expects the input buffer to be Irp->AssociatedIrp.SystemBuffer instead of
    // Type3InputBuffer. That will also fail WdfRequestRetrieveInputMemory.
    //
    // The solution to the above two problems is to get Type3InputBuffer directly
    //
    // Also note that instead of the buffer's content, it is the buffer address
    // that was used to store the string id (or index)
    //

    WDF_REQUEST_PARAMETERS_INIT(&requestParameters);
    WdfRequestGetParameters(Request, &requestParameters);

    inputValue = PtrToUlong(
        requestParameters.Parameters.DeviceIoControl.Type3InputBuffer);

    status = STATUS_SUCCESS;

#else

    WDFMEMORY               inputMemory;
    size_t                  inputBufferLength;
    PVOID                   inputBuffer;

    //
    // mshidumdf.sys updates the IRP and passes the string id (or index) through
    // the input buffer correctly based on the IOCTL buffer type
    //

    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveInputMemory failed 0x%x\n", status);
        return status;
    }
    inputBuffer = WdfMemoryGetBuffer(inputMemory, &inputBufferLength);

    //
    // make sure buffer is big enough.
    //
    if (inputBufferLength < sizeof(ULONG))
    {
        status = STATUS_INVALID_BUFFER_SIZE;
        TraceError("GetStringId: invalid input buffer. size %d, expect %d\n",
            (int)inputBufferLength, (int)sizeof(ULONG));
        return status;
    }

    inputValue = (*(PULONG)inputBuffer);

#endif

    //
    // The least significant two bytes of the INT value contain the string id.
    //
    * StringId = (inputValue & 0x0ffff);

    //
    // The most significant two bytes of the INT value contain the language
    // ID (for example, a value of 1033 indicates English).
    //
    *LanguageId = (inputValue >> 16);

    return status;
}


NTSTATUS
GetIndexedString(
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_GET_INDEXED_STRING

Arguments:

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    ULONG                   languageId, stringIndex;

    status = GetStringId(Request, &stringIndex, &languageId);

    // While we don't use the language id, some minidrivers might.
    //
    UNREFERENCED_PARAMETER(languageId);

    if (NT_SUCCESS(status)) {

        if (stringIndex != FWK_ARGB_DEVICE_STRING_INDEX)
        {
            status = STATUS_INVALID_PARAMETER;
            TraceError("GetString: unkown string index %d\n", stringIndex);
            return status;
        }

        status = RequestCopyFromBuffer(Request, FWK_ARGB_DEVICE_STRING, sizeof(FWK_ARGB_DEVICE_STRING));
    }
    return status;
}


NTSTATUS
GetString(
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_GET_STRING.

Arguments:

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    ULONG                   languageId, stringId;
    size_t                  stringSizeCb;
    PWSTR                   string;

    status = GetStringId(Request, &stringId, &languageId);

    // While we don't use the language id, some minidrivers might.
    //
    UNREFERENCED_PARAMETER(languageId);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    switch (stringId) {
    case HID_STRING_ID_IMANUFACTURER:
        stringSizeCb = sizeof(FWK_ARGB_MANUFACTURER_STRING);
        string = FWK_ARGB_MANUFACTURER_STRING;
        break;
    case HID_STRING_ID_IPRODUCT:
        stringSizeCb = sizeof(FWK_ARGB_PRODUCT_STRING);
        string = FWK_ARGB_PRODUCT_STRING;
        break;
    case HID_STRING_ID_ISERIALNUMBER:
        stringSizeCb = sizeof(FWK_ARGB_SERIAL_NUMBER_STRING);
        string = FWK_ARGB_SERIAL_NUMBER_STRING;
        break;
    default:
        status = STATUS_INVALID_PARAMETER;
        TraceError("GetString: unkown string id %d\n", stringId);
        return status;
    }

    status = RequestCopyFromBuffer(Request, string, stringSizeCb);
    return status;
}
