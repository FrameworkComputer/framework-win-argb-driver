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

#define  LAMP_ARRAY_ATTRIBUTES_REPORT_ID    0x01
#define  LAMP_ATTRIBUTES_REQUEST_REPORT_ID  0x02
#define  LAMP_ATTRIBUTES_RESPONSE_REPORT_ID 0x03
#define  LAMP_MULTI_UPDATE_REPORT_ID        0x04
#define  LAMP_RANGE_UPDATE_REPORT_ID        0x05
#define  LAMP_ARRAY_CONTROL_REPORT_ID       0x06

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

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        //
        // Get a pointer to the device context structure that we just associated
        // with the device object. We define this structure in the device.h
        // header file. DeviceGetContext is an inline function generated by
        // using the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro in device.h.
        // This function will do the type checking and return the device context.
        // If you pass a wrong object handle it will return NULL and assert if
        // run under framework verifier mode.
        //
        deviceContext = DeviceGetContext(device);

        //
        // Initialize the context.
        //
        deviceContext->Device = device;
        deviceContext->DeviceData = 0;

        hidAttributes = &deviceContext->HidDeviceAttributes;
        RtlZeroMemory(hidAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
        hidAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
        hidAttributes->VendorID = FWK_ARGB_HID_VID;
        hidAttributes->ProductID = FWK_ARGB_HID_PID;
        hidAttributes->VersionNumber = FWK_ARGB_HID_VERSION;

        //
        // Create a device interface so that applications can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_FrameworkArgb,
            NULL // ReferenceString
            );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = FrameworkArgbQueueInitialize(device);
        }
    }

    return status;
}
