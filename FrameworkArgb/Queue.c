/*++

SPDX-License-Identifier: MS-PL

Copyright (C) Microsoft Corporation, All Rights Reserved.
Copyright (C) Framework Computer Inc, All Rights Reserved.

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "queue.tmh"
#include "LampArray.h"

#ifdef _KERNEL_MODE
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL EvtIoDeviceControl;
#else
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL          EvtIoDeviceControl;
#endif

NTSTATUS
QueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
)
/*++
Routine Description:

    This function creates a default, parallel I/O queue to proces IOCTLs
    from hidclass.sys.

Arguments:

    Device - Handle to a framework device object.

    Queue - Output pointer to a framework I/O queue handle, on success.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDF_IO_QUEUE_CONFIG     queueConfig;
    WDF_OBJECT_ATTRIBUTES   queueAttributes;
    WDFQUEUE                queue;
    PQUEUE_CONTEXT          queueContext;

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel);

#ifdef _KERNEL_MODE
    queueConfig.EvtIoInternalDeviceControl = EvtIoDeviceControl;
#else
    //
    // HIDclass uses INTERNAL_IOCTL which is not supported by UMDF. Therefore
    // the hidumdf.sys changes the IOCTL type to DEVICE_CONTROL for next stack
    // and sends it down
    //
    queueConfig.EvtIoDeviceControl = EvtIoDeviceControl;
#endif

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
        &queueAttributes,
        QUEUE_CONTEXT);

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &queueAttributes,
        &queue);

    if (!NT_SUCCESS(status)) {
        TraceError("WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

    queueContext = GetQueueContext(queue);
    queueContext->Queue = queue;
    queueContext->DeviceContext = GetDeviceContext(Device);
    queueContext->OutputReport = 0;

    *Queue = queue;
    return status;
}

VOID
EvtIoDeviceControl(
    _In_  WDFQUEUE          Queue,
    _In_  WDFREQUEST        Request,
    _In_  size_t            OutputBufferLength,
    _In_  size_t            InputBufferLength,
    _In_  ULONG             IoControlCode
)
/*++
Routine Description:

    This event callback function is called when the driver receives an

    (KMDF) IOCTL_HID_Xxx code when handlng IRP_MJ_INTERNAL_DEVICE_CONTROL
    (UMDF) IOCTL_HID_Xxx, IOCTL_UMDF_HID_Xxx when handling IRP_MJ_DEVICE_CONTROL

Arguments:

    Queue - A handle to the queue object that is associated with the I/O request

    Request - A handle to a framework request object.

    OutputBufferLength - The length, in bytes, of the request's output buffer,
            if an output buffer is available.

    InputBufferLength - The length, in bytes, of the request's input buffer, if
            an input buffer is available.

    IoControlCode - The driver or system defined IOCTL associated with the request

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    BOOLEAN                 completeRequest = TRUE;
    WDFDEVICE               device = WdfIoQueueGetDevice(Queue);
    PDEVICE_CONTEXT         deviceContext = NULL;
    PQUEUE_CONTEXT          queueContext = GetQueueContext(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    deviceContext = GetDeviceContext(device);

    TraceInformation("%!FUNC! called with IoControlCode=%d", IoControlCode);

    switch (IoControlCode)
    {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:   // METHOD_NEITHER
        //
        // Retrieves the device's HID descriptor.
        //
        _Analysis_assume_(deviceContext->HidDescriptor.bLength != 0);
        status = RequestCopyFromBuffer(Request,
            &deviceContext->HidDescriptor,
            deviceContext->HidDescriptor.bLength);
        break;

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:   // METHOD_NEITHER
        //
        //Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
        //
        status = RequestCopyFromBuffer(Request,
            &queueContext->DeviceContext->HidDeviceAttributes,
            sizeof(HID_DEVICE_ATTRIBUTES));
        break;

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:   // METHOD_NEITHER
        //
        //Obtains the report descriptor for the HID device.
        //
        status = RequestCopyFromBuffer(Request,
            deviceContext->ReportDescriptor,
            deviceContext->HidDescriptor.DescriptorList[0].wReportLength);
        break;

    case IOCTL_HID_READ_REPORT:             // METHOD_NEITHER
        //
        // Returns a report from the device into a class driver-supplied
        // buffer.
        //
        status = ReadReport(queueContext, Request, &completeRequest);
        break;

    case IOCTL_HID_WRITE_REPORT:            // METHOD_NEITHER
        //
        // Transmits a class driver-supplied report to the device.
        //
        status = WriteReport(queueContext, Request);
        break;

#ifdef _KERNEL_MODE

    case IOCTL_HID_GET_FEATURE:             // METHOD_OUT_DIRECT

        status = GetFeature(queueContext, Request);
        break;

    case IOCTL_HID_SET_FEATURE:             // METHOD_IN_DIRECT

        status = SetFeature(queueContext, Request);
        break;

    case IOCTL_HID_GET_INPUT_REPORT:        // METHOD_OUT_DIRECT

        status = GetInputReport(queueContext, Request);
        break;

    case IOCTL_HID_SET_OUTPUT_REPORT:       // METHOD_IN_DIRECT

        status = SetOutputReport(queueContext, Request);
        break;

#else // UMDF specific

        //
        // HID minidriver IOCTL uses HID_XFER_PACKET which contains an embedded pointer.
        //
        //   typedef struct _HID_XFER_PACKET {
        //     PUCHAR reportBuffer;
        //     ULONG  reportBufferLen;
        //     UCHAR  reportId;
        //   } HID_XFER_PACKET, *PHID_XFER_PACKET;
        //
        // UMDF cannot handle embedded pointers when marshalling buffers between processes.
        // Therefore a special driver mshidumdf.sys is introduced to convert such IRPs to
        // new IRPs (with new IOCTL name like IOCTL_UMDF_HID_Xxxx) where:
        //
        //   reportBuffer - passed as one buffer inside the IRP
        //   reportId     - passed as a second buffer inside the IRP
        //
        // The new IRP is then passed to UMDF host and driver for further processing.
        //

    case IOCTL_UMDF_HID_GET_FEATURE:        // METHOD_NEITHER

        status = GetFeature(queueContext, Request);
        break;

    case IOCTL_UMDF_HID_SET_FEATURE:        // METHOD_NEITHER

        status = SetFeature(queueContext, Request);
        break;

    case IOCTL_UMDF_HID_GET_INPUT_REPORT:  // METHOD_NEITHER

        status = GetInputReport(queueContext, Request);
        break;

    case IOCTL_UMDF_HID_SET_OUTPUT_REPORT: // METHOD_NEITHER

        status = SetOutputReport(queueContext, Request);
        break;

#endif // _KERNEL_MODE

    case IOCTL_HID_GET_STRING:                      // METHOD_NEITHER

        status = GetString(Request);
        break;

    case IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT

        status = GetIndexedString(Request);
        break;

    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:  // METHOD_NEITHER
        //
        // This has the USBSS Idle notification callback. If the lower driver
        // can handle it (e.g. USB stack can handle it) then pass it down
        // otherwise complete it here as not inplemented. For a virtual
        // device, idling is not needed.
        //
        // Not implemented. fall through...
        //
    case IOCTL_HID_ACTIVATE_DEVICE:                 // METHOD_NEITHER
    case IOCTL_HID_DEACTIVATE_DEVICE:               // METHOD_NEITHER
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:             // METHOD_OUT_DIRECT
        //
        // We don't do anything for these IOCTLs but some minidrivers might.
        //
        // Not implemented. fall through...
        //
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    //
    // Complete the request. Information value has already been set by request
    // handlers.
    //
    if (completeRequest) {
        WdfRequestComplete(Request, status);
    }
    TraceInformation("%!FUNC! exiting with completeRequest=%d", completeRequest);
}

NTSTATUS
RequestCopyFromBuffer(
    _In_  WDFREQUEST        Request,
    _In_  PVOID             SourceBuffer,
    _When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
    _In_  size_t            NumBytesToCopyFrom
)
/*++

Routine Description:

    A helper function to copy specified bytes to the request's output memory

Arguments:

    Request - A handle to a framework request object.

    SourceBuffer - The buffer to copy data from.

    NumBytesToCopyFrom - The length, in bytes, of data to be copied.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDFMEMORY               memory;
    size_t                  outputBufferLength;

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        return status;
    }

    WdfMemoryGetBuffer(memory, &outputBufferLength);
    if (outputBufferLength < NumBytesToCopyFrom) {
        status = STATUS_INVALID_BUFFER_SIZE;
        TraceError("RequestCopyFromBuffer: buffer too small. Size %d, expect %d\n",
            (int)outputBufferLength, (int)NumBytesToCopyFrom);
        return status;
    }

    status = WdfMemoryCopyFromBuffer(memory,
        0,
        SourceBuffer,
        NumBytesToCopyFrom);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfMemoryCopyFromBuffer failed 0x%x\n", status);
        return status;
    }

    WdfRequestSetInformation(Request, NumBytesToCopyFrom);
    return status;
}

//
// HID minidriver IOCTL uses HID_XFER_PACKET which contains an embedded pointer.
//
//   typedef struct _HID_XFER_PACKET {
//     PUCHAR reportBuffer;
//     ULONG  reportBufferLen;
//     UCHAR  reportId;
//   } HID_XFER_PACKET, *PHID_XFER_PACKET;
//
// UMDF cannot handle embedded pointers when marshalling buffers between processes.
// Therefore a special driver mshidumdf.sys is introduced to convert such IRPs to
// new IRPs (with new IOCTL name like IOCTL_UMDF_HID_Xxxx) where:
//
//   reportBuffer - passed as one buffer inside the IRP
//   reportId     - passed as a second buffer inside the IRP
//
// The new IRP is then passed to UMDF host and driver for further processing.
//

NTSTATUS
RequestGetHidXferPacket_ToReadFromDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET* Packet
)
{
    //
    // Driver need to write to the output buffer (so that App can read from it)
    //
    //   Report Buffer: Output Buffer
    //   Report Id    : Input Buffer
    //

    NTSTATUS                status;
    WDFMEMORY               inputMemory;
    WDFMEMORY               outputMemory;
    size_t                  inputBufferLength;
    size_t                  outputBufferLength;
    PVOID                   inputBuffer;
    PVOID                   outputBuffer;

    //
    // Get report Id from input buffer
    //
    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveInputMemory failed 0x%x\n", status);
        return status;
    }
    inputBuffer = WdfMemoryGetBuffer(inputMemory, &inputBufferLength);

    if (inputBufferLength < sizeof(UCHAR)) {
        status = STATUS_INVALID_BUFFER_SIZE;
        TraceError("WdfRequestRetrieveInputMemory: invalid input buffer. size %d, expect %d\n",
            (int)inputBufferLength, (int)sizeof(UCHAR));
        return status;
    }

    Packet->reportId = *(PUCHAR)inputBuffer;

    //
    // Get report buffer from output buffer
    //
    status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        return status;
    }

    outputBuffer = WdfMemoryGetBuffer(outputMemory, &outputBufferLength);

    Packet->reportBuffer = (PUCHAR)outputBuffer;
    Packet->reportBufferLen = (ULONG)outputBufferLength;

    return status;
}

NTSTATUS
RequestGetHidXferPacket_ToWriteToDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET* Packet
)
{
    //
    // Driver need to read from the input buffer (which was written by App)
    //
    //   Report Buffer: Input Buffer
    //   Report Id    : Output Buffer Length
    //
    // Note that the report id is not stored inside the output buffer, as the
    // driver has no read-access right to the output buffer, and trying to read
    // from the buffer will cause an access violation error.
    //
    // The workaround is to store the report id in the OutputBufferLength field,
    // to which the driver does have read-access right.
    //

    NTSTATUS                status;
    WDFMEMORY               inputMemory;
    WDFMEMORY               outputMemory;
    size_t                  inputBufferLength;
    size_t                  outputBufferLength;
    PVOID                   inputBuffer;

    //
    // Get report Id from output buffer length
    //
    status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        return status;
    }
    WdfMemoryGetBuffer(outputMemory, &outputBufferLength);
    Packet->reportId = (UCHAR)outputBufferLength;

    //
    // Get report buffer from input buffer
    //
    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        TraceError("WdfRequestRetrieveInputMemory failed 0x%x\n", status);
        return status;
    }
    inputBuffer = WdfMemoryGetBuffer(inputMemory, &inputBufferLength);

    Packet->reportBuffer = (PUCHAR)inputBuffer;
    Packet->reportBufferLen = (ULONG)inputBufferLength;

    return status;
}

NTSTATUS
ReadReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request,
    _Always_(_Out_)
    BOOLEAN* CompleteRequest
)
/*++

Routine Description:

    Handles IOCTL_HID_READ_REPORT for the HID collection. Normally the request
    will be forwarded to a manual queue for further process. In that case, the
    caller should not try to complete the request at this time, as the request
    will later be retrieved back from the manually queue and completed there.
    However, if for some reason the forwarding fails, the caller still need
    to complete the request with proper error code immediately.

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to  Request Packet.

    CompleteRequest - A boolean output value, indicating whether the caller
            should complete the request or not

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;

TraceInformation("%!FUNC! Entry");

//
// forward the request to manual queue
//
status = WdfRequestForwardToIoQueue(
    Request,
    QueueContext->DeviceContext->ManualQueue);
if (!NT_SUCCESS(status)) {
    TraceError("WdfRequestForwardToIoQueue failed with 0x%x\n", status);
    *CompleteRequest = TRUE;
}
else {
    *CompleteRequest = FALSE;
}

return status;
}

NTSTATUS
WriteReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_WRITE_REPORT all the collection.

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to  Request Packet.

Return Value:

    NT status code.

--*/

{
    NTSTATUS                status;
    HID_XFER_PACKET         packet;
    ULONG                   reportSize;

    UNREFERENCED_PARAMETER(QueueContext);

    TraceInformation("%!FUNC! Entry");

    status = RequestGetHidXferPacket_ToWriteToDevice(
        Request,
        &packet);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    TraceError("WriteReport: with ReportID %d and size: %d\n", packet.reportId, packet.reportBufferLen);
    status = STATUS_INVALID_PARAMETER;
    reportSize = 0;

    //
    // set status and information
    //
    WdfRequestSetInformation(Request, reportSize);
    return status;
}

HRESULT
GetFeature(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_GET_FEATURE for all the collection.

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to  Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    HID_XFER_PACKET         packet;
    ULONG                   reportSize;
	PUCHAR                  responseBuffer;

    TraceInformation("%!FUNC! Entry");

    status = RequestGetHidXferPacket_ToReadFromDevice(
        Request,
        &packet);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    reportSize = 0;
	responseBuffer = packet.reportBuffer + sizeof(packet.reportId);

    switch (packet.reportId) {
    case LAMP_ARRAY_ATTRIBUTES_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ARRAY_ATTRIBUTES_REPORT_ID");
        reportSize = GetLampArrayAttributesReport(responseBuffer, QueueContext->DeviceContext, packet.reportBufferLen);
        break;
    case LAMP_ATTRIBUTES_REQUEST_REPORT_ID:
		TraceError("%!FUNC! LAMP_ATTRIBUTES_REQUEST_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
	case LAMP_ATTRIBUTES_RESPONSE_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ATTRIBUTES_RESPONSE_REPORT_ID");
        reportSize = GetLampAttributesResponseReport(responseBuffer, QueueContext->DeviceContext, packet.reportBufferLen);
		break;
	case LAMP_MULTI_UPDATE_REPORT_ID:
		TraceError("%!FUNC! LAMP_MULTI_UPDATE_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
	case LAMP_RANGE_UPDATE_REPORT_ID:
		TraceError("%!FUNC! LAMP_RANGE_UPDATE_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
	case LAMP_ARRAY_CONTROL_REPORT_ID:
		TraceError("%!FUNC! LAMP_ARRAY_CONTROL_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
    case CONTROL_COLLECTION_REPORT_ID:
		TraceError("%!FUNC! CONTROL_COLLECTION_REPORT_ID - Unsupported");
        break;
    default:
        //
        // If collection ID is not for control collection then handle
        // this request just as you would for a regular collection.
        //
		TraceError("%!FUNC! invalid report id %d\n", packet.reportId);
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Since output buffer is for write only (no read allowed by UMDF in output
    // buffer), any read from output buffer would be reading garbage), so don't
    // let app embed custom control code in output buffer. The minidriver can
    // support multiple features using separate report ID instead of using
    // custom control code. Since this is targeted at report ID 1, we know it
    // is a request for getting attributes.
    //
    // While KMDF does not enforce the rule (disallow read from output buffer),
    // it is good practice to not do so.
    // Since this device has one report ID, hidclass would pass on the report
    // ID in the buffer (it wouldn't if report descriptor did not have any report
    // ID). However, since UMDF allows only writes to an output buffer, we can't
    // "read" the report ID from "output" buffer. There is no need to read the
    // report ID since we get it other way as shown above, however this is
    // something to keep in mind.
    //

    //
    // Report how many bytes were copied
    //
    if (reportSize == 0) {
		TraceError("%!FUNC! empty buffer. Not returning anything");
        return STATUS_INVALID_BUFFER_SIZE;
	}

	TraceError("%!FUNC! Sending %d bytes", reportSize);
    WdfRequestSetInformation(Request, reportSize);
    return status;
}

NTSTATUS
SetFeature(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_SET_FEATURE for all the collection.
    For control collection (custom defined collection) it handles
    the user-defined control codes for sideband communication

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    HID_XFER_PACKET         packet;
    ULONG                   reportSize;
    PUCHAR                  responseBuffer;

    TraceInformation("%!FUNC! Entry");

    status = RequestGetHidXferPacket_ToWriteToDevice(
        Request,
        &packet);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // No response
    reportSize = 0;
	responseBuffer = packet.reportBuffer + sizeof(packet.reportId);

    switch (packet.reportId) {
    case LAMP_ARRAY_ATTRIBUTES_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ARRAY_ATTRIBUTES_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
    case LAMP_ATTRIBUTES_REQUEST_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ATTRIBUTES_REQUEST_REPORT_ID");
		SetLampAttributesId(responseBuffer, QueueContext->DeviceContext);
        break;
	case LAMP_ATTRIBUTES_RESPONSE_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ATTRIBUTES_RESPONSE_REPORT_ID - Unsupported");
        return STATUS_INVALID_PARAMETER;
	case LAMP_MULTI_UPDATE_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_MULTI_UPDATE_REPORT_ID");
		SetMultipleLamps(responseBuffer, QueueContext->DeviceContext);
		break;
	case LAMP_RANGE_UPDATE_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_RANGE_UPDATE_REPORT_ID");
		SetLampRange(responseBuffer, QueueContext->DeviceContext);
		break;
	case LAMP_ARRAY_CONTROL_REPORT_ID:
		TraceInformation("%!FUNC! LAMP_ARRAY_CONTROL_REPORT_ID");
		SetAutonomousMode(responseBuffer, QueueContext->DeviceContext);
		break;
    default:
        //
        // If collection ID is not for control collection then handle
        // this request just as you would for a regular collection.
        //
		TraceError("%!FUNC! invalid report id %d\n", packet.reportId);
        return STATUS_INVALID_PARAMETER;
    }

    WdfRequestSetInformation(Request, reportSize);
    return status;
}

NTSTATUS
GetInputReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_GET_INPUT_REPORT for all the collection.

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    HID_XFER_PACKET         packet;
    ULONG                   reportSize;

    TraceInformation("%!FUNC! Entry");

    UNREFERENCED_PARAMETER(QueueContext);

    status = RequestGetHidXferPacket_ToReadFromDevice(
        Request,
        &packet);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    TraceError("GetInputReport: with ReportID %d and size: %d\n", packet.reportId, packet.reportBufferLen);
    status = STATUS_INVALID_PARAMETER;
    reportSize = 0;

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, reportSize);
    return status;
}


NTSTATUS
SetOutputReport(
    _In_  PQUEUE_CONTEXT    QueueContext,
    _In_  WDFREQUEST        Request
)
/*++

Routine Description:

    Handles IOCTL_HID_SET_OUTPUT_REPORT for all the collection.

Arguments:

    QueueContext - The object context associated with the queue

    Request - Pointer to Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status;
    HID_XFER_PACKET         packet;
    ULONG                   reportSize;

    TraceInformation("%!FUNC! Entry");

    UNREFERENCED_PARAMETER(QueueContext);

    status = RequestGetHidXferPacket_ToWriteToDevice(
        Request,
        &packet);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    TraceError("GetInputReport: with ReportID %d and size: %d\n", packet.reportId, packet.reportBufferLen);
    status = STATUS_INVALID_PARAMETER;
    reportSize = 0;

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, reportSize);
    return status;
}

NTSTATUS
ManualQueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
)
/*++
Routine Description:

    This function creates a manual I/O queue to receive IOCTL_HID_READ_REPORT
    forwarded from the device's default queue handler.

    It also creates a periodic timer to check the queue and complete any pending
    request with data from the device. Here timer expiring is used to simulate
    a hardware event that new data is ready.

    The workflow is like this:

    - Hidclass.sys sends an ioctl to the miniport to read input report.

    - The request reaches the driver's default queue. As data may not be avaiable
      yet, the request is forwarded to a second manual queue temporarily.

    - Later when data is ready (as simulated by timer expiring), the driver
      checks for any pending request in the manual queue, and then completes it.

    - Hidclass gets notified for the read request completion and return data to
      the caller.

    On the other hand, for IOCTL_HID_WRITE_REPORT request, the driver simply
    sends the request to the hardware (as simulated by storing the data at
    DeviceContext->DeviceData) and completes the request immediately. There is
    no need to use another queue for write operation.

Arguments:

    Device - Handle to a framework device object.

    Queue - Output pointer to a framework I/O queue handle, on success.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDF_IO_QUEUE_CONFIG     queueConfig;
    WDF_OBJECT_ATTRIBUTES   queueAttributes;
    WDFQUEUE                queue;
    PMANUAL_QUEUE_CONTEXT   queueContext;
    WDF_TIMER_CONFIG        timerConfig;
    WDF_OBJECT_ATTRIBUTES   timerAttributes;
    ULONG                   timerPeriodInSeconds = 5;

    WDF_IO_QUEUE_CONFIG_INIT(
        &queueConfig,
        WdfIoQueueDispatchManual);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
        &queueAttributes,
        MANUAL_QUEUE_CONTEXT);

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &queueAttributes,
        &queue);

    if (!NT_SUCCESS(status)) {
        TraceError("WdfIoQueueCreate failed 0x%x\n", status);
        return status;
    }

    queueContext = GetManualQueueContext(queue);
    queueContext->Queue = queue;
    queueContext->DeviceContext = GetDeviceContext(Device);

    WDF_TIMER_CONFIG_INIT_PERIODIC(
        &timerConfig,
        EvtTimerFunc,
        timerPeriodInSeconds * 1000);

    WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
    timerAttributes.ParentObject = queue;
    status = WdfTimerCreate(&timerConfig,
        &timerAttributes,
        &queueContext->Timer);

    if (!NT_SUCCESS(status)) {
        TraceError("WdfTimerCreate failed 0x%x\n", status);
        return status;
    }

    WdfTimerStart(queueContext->Timer, WDF_REL_TIMEOUT_IN_SEC(1));

    *Queue = queue;

    return status;
}

void
EvtTimerFunc(
    _In_  WDFTIMER          Timer
)
/*++
Routine Description:

    This periodic timer callback routine checks the device's manual queue and
    completes any pending request with data from the device.

Arguments:

    Timer - Handle to a timer object that was obtained from WdfTimerCreate.

Return Value:

    VOID

--*/
{
    NTSTATUS                status;
    WDFQUEUE                queue;
    PMANUAL_QUEUE_CONTEXT   queueContext;
    WDFREQUEST              request;

    TraceInformation("%!FUNC! Entry");

    queue = (WDFQUEUE)WdfTimerGetParentObject(Timer);
    queueContext = GetManualQueueContext(queue);

    //
    // see if we have a request in manual queue
    //
    status = WdfIoQueueRetrieveNextRequest(
        queueContext->Queue,
        &request);

    if (STATUS_NO_MORE_ENTRIES == status) {
        //
        // No request in the queue, nothing to do
        //
        TraceInformation("No request in manual queue\n");
        return;
    } else if (!NT_SUCCESS(status)) {
        TraceError("WdfIoQueueRetrieveNextRequest failed 0x%x\n", status);
        return;
	}

    TraceInformation("Has request in manual queue\n");

    WdfRequestComplete(request, status);
}
