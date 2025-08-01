/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "LampArray.tmh"
#include "LampArray.h"

ULONG
GetLampArrayAttributesReport(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext,
    _In_        ULONG           MaxLen
    )
{
    ULONG Size = sizeof(LampArrayAttributesReport);
    if (Size > MaxLen) {
		TraceError("%!FUNC!: output buffer too small. Size %d, expect %d\n",
			Size, MaxLen);
        return 0;
    }

	DeviceContext = DeviceContext;

    LampArrayAttributesReport report = {
        LAMPARRAY_LAMP_COUNT,
        LAMPARRAY_WIDTH,
        LAMPARRAY_HEIGHT,
        LAMPARRAY_DEPTH,
        LAMPARRAY_KIND,
        LAMPARRAY_UPDATE_INTERVAL
    };

    RtlCopyMemory(ReportBuffer, &report, Size);

    return Size;
}

ULONG
GetLampAttributesResponseReport(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext,
    _In_        ULONG           MaxLen
    )
{
	UINT16 CurrentLampId = DeviceContext->CurrentLampId;
    ULONG Size = sizeof(LampAttributesResponseReport);
    if (Size > MaxLen) {
		TraceError("%!FUNC!: output buffer too small. Size %d, expect %d\n",
			Size, MaxLen);
        return 0;
    }

    LampAttributesResponseReport report = {
        CurrentLampId,                                          // LampId
        DeviceContext->LampPositions[CurrentLampId],            // Lamp position
        LAMPARRAY_UPDATE_INTERVAL,                              // Lamp update interval
        LAMP_PURPOSE_ACCENT,                                    // Lamp purpose
        255,                                                    // Red level count
        255,                                                    // Blue level count
        255,                                                    // Green level count
        1,                                                      // Intensity
        1,                                                      // Is Programmable
        0,                                                      // InputBinding
    };

    RtlCopyMemory(ReportBuffer, &report, sizeof(LampAttributesResponseReport));
    DeviceContext->CurrentLampId = CurrentLampId + 1 >= LAMPARRAY_LAMP_COUNT ? CurrentLampId : CurrentLampId + 1;

	return Size;

}


void SetLampAttributesId(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
    )
{
    LampAttributesRequestReport* report = (LampAttributesRequestReport*)ReportBuffer;
	DeviceContext->CurrentLampId = report->lampId;
}

void SetMultipleLamps(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
    )
{
    LampMultiUpdateReport* report = (LampMultiUpdateReport*)ReportBuffer;
	TraceInformation("%!FUNC! LampCount: %d", report->lampCount);

    for (int i = 0; i < report->lampCount; i++) {
		TraceInformation("%!FUNC! LampId: %d, Color: #%02X%02X%02X Intensity: %d",
			report->lampIds[i],
			report->colors[i].red,
			report->colors[i].green,
			report->colors[i].blue,
			report->colors[i].intensity
		);
		DeviceContext = DeviceContext;
        // NeopixelSetColor(report->lampIds[i], report->colors[i]);
    }
}

void SetLampRange(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
    )
{
    LampRangeUpdateReport* report = (LampRangeUpdateReport*)ReportBuffer;
    TraceInformation("%!FUNC! LampIdStart: %d, LampIdEnd: %d, Color: #%02X%02X%02X Intensity: %d",
        report->lampIdStart,
        report->lampIdEnd,
        report->color.red,
        report->color.green,
        report->color.blue,
		report->color.intensity
    );
    DeviceContext = DeviceContext;
    // CrosEcSetColorRange(report->lampIdStart, report->lampIdEnd, report->color);
}

void SetAutonomousMode(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
    )
{
    LampArrayControlReport* report = (LampArrayControlReport*)ReportBuffer;

    TraceInformation("%!FUNC! AutonomousMode: %d", report->autonomousMode);
	DeviceContext->AutonomousMode = report->autonomousMode != 0;
    // CrosEcSetEffect(report->autonomousMode ? AUTONOMOUS_LIGHTING_EFFECT : HID, AUTONOMOUS_LIGHTING_COLOR);
}
