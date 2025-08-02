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
#include "EcCommunication.h"

void CrosEcSetColorRange(HANDLE Handle, UINT16 LampIdStart, UINT16 LampIdEnd, LampColor *Color)
{
    EC_REQUEST_RGB_KBD_SET_COLOR cmd{};
    cmd.StartKey = (UINT8)  LampIdStart;
    cmd.Length = (UINT8) (LampIdEnd - LampIdStart) + 1;
    for (UINT8 i = 0; i < cmd.Length; i++) {
        if (Color->intensity == 1) {
            cmd.Colors[i].r = Color->red;
            cmd.Colors[i].g = Color->green;
            cmd.Colors[i].b = Color->blue;
        }
        // TODO: Handle intensity, but I don't think it's normally used
        // cmd.Colors[i].intensity = Color->intensity;
    }
    if (Color->intensity != 1 && Color->intensity != 1) {
        TraceError("%!FUNC! Intensity is %d, not 0 or 1, should handle that", Color->intensity);
    }

    CrosEcSendCommand(
        Handle,
        EC_CMD_RGBKBD_SET_COLOR,
        0,
        &cmd,
        sizeof(cmd),
        &cmd,
        sizeof(cmd)
    );
}

void CrosEcSetColor(HANDLE Handle, UINT16 LampId, LampColor *Color)
{
    CrosEcSetColorRange(Handle, LampId, LampId, Color);
}

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
        DeviceContext->LampCount,
        DeviceContext->Width,
        DeviceContext->Height,
        DeviceContext->Depth,
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
    DeviceContext->CurrentLampId = CurrentLampId + 1 >= DeviceContext->LampCount ? CurrentLampId : CurrentLampId + 1;

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
        CrosEcSetColor(DeviceContext->CrosEcHandle, report->lampIds[i], &report->colors[i]);
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
    CrosEcSetColorRange(DeviceContext->CrosEcHandle, report->lampIdStart, report->lampIdEnd, &report->color);
}

void SetAutonomousMode(
    _In_        PUCHAR          ReportBuffer,
    _In_ _Out_  PDEVICE_CONTEXT DeviceContext
    )
{
    LampArrayControlReport* report = (LampArrayControlReport*)ReportBuffer;

    TraceInformation("%!FUNC! AutonomousMode: %d", report->autonomousMode);
	DeviceContext->AutonomousMode = report->autonomousMode != 0;
    // Not doing anything here, unless we want to have a default animation
}
