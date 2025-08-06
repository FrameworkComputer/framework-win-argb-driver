/*++

SPDX-License-Identifier: MS-PL

Copyright (C) Microsoft Corporation, All Rights Reserved.
Copyright (C) Framework Computer Inc, All Rights Reserved.

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    User-mode Driver Framework 2

--*/

EXTERN_C_START

NTSTATUS
QueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
);
NTSTATUS
ManualQueueCreate(
    _In_  WDFDEVICE         Device,
    _Out_ WDFQUEUE* Queue
);

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL FrameworkArgbEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP FrameworkArgbEvtIoStop;

EXTERN_C_END
