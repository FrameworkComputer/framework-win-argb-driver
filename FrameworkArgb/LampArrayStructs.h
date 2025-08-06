/*++

Copyright (C) Microsoft Corporation, All Rights Reserved.
Copyright (C) Framework Computer Inc, All Rights Reserved.

Module Name:

    LampArrayStructs.h

Abstract:

    This file contains the queue definitions.

Environment:

    User-mode Driver Framework 2

--*/
#pragma once
#include <windows.h>

EXTERN_C_START

#include <pshpack1.h>

typedef struct {
    UINT8 red;
    UINT8 green;
    UINT8 blue;
    UINT8 intensity;
} LampColor;

typedef struct {
    UINT32 x;
    UINT32 y;
    UINT32 z;
} Position;

typedef struct {
    UINT16 lampCount;

    UINT32 width;
    UINT32 height;
    UINT32 depth;

    UINT32 lampArrayKind;
    UINT32 minUpdateInterval;
} LampArrayAttributesReport;

typedef struct {
    UINT16 lampId;
} LampAttributesRequestReport;

#define LAMP_PURPOSE_CONTROL        0x01
#define LAMP_PURPOSE_ACCENT         0x02
#define LAMP_PURPOSE_BRANDING       0x04
#define LAMP_PURPOSE_STATUS         0x08
#define LAMP_PURPOSE_ILLUMINATION   0x10
#define LAMP_PURPOSE_PRESENTATION   0x20

typedef struct {
    UINT16 lampId;

    Position lampPosition;

    UINT32 updateLatency;
    UINT32 lampPurposes;

    UINT8 redLevelCount;
    UINT8 greenLevelCount;
    UINT8 blueLevelCount;
    UINT8 intensityLevelCount;

    UINT8 isProgrammable;
    UINT8 inputBinding;
} LampAttributesResponseReport;

typedef struct {
    UINT8 lampCount;
    UINT8 flags;
    UINT16 lampIds[8];

    LampColor colors[8];
} LampMultiUpdateReport;

typedef struct {
    UINT8 flags;
    UINT16 lampIdStart;
    UINT16 lampIdEnd;

    LampColor color;
} LampRangeUpdateReport;

typedef struct {
    UINT8 autonomousMode;
} LampArrayControlReport;

#include <poppack.h>

EXTERN_C_END
