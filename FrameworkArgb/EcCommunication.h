/*++

SPDX-License-Identifier: MS-PL

Copyright (C) Framework Computer Inc, All Rights Reserved.

Module Name:

    EcCommunication.h

Abstract:

    This file contains definitions for accessing the EC via the crosecbus
    KMDF driver.

Environment:

    User-mode Driver Framework 2

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <handleapi.h>
#include <windows.h>
#include <wdf.h>
#include "trace.h"

#include "ec_commands.h"

#define FILE_DEVICE_CROS_EMBEDDED_CONTROLLER 0x80EC

#define IOCTL_CROSEC_XCMD \
	CTL_CODE(FILE_DEVICE_CROS_EMBEDDED_CONTROLLER, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_CROSEC_RDMEM CTL_CODE(FILE_DEVICE_CROS_EMBEDDED_CONTROLLER, 0x802, METHOD_BUFFERED, FILE_READ_DATA)

#define CROSEC_CMD_MAX_REQUEST  0x100
#define CROSEC_CMD_MAX_RESPONSE 0x100
#define CROSEC_MEMMAP_SIZE      0xFF

NTSTATUS ConnectToEc(
	_Inout_ HANDLE* Handle
);

#define EC_RES_SUCCESS 0
#define EC_INVALID_COMMAND 1
#define EC_ERROR 2
#define EC_INVALID_PARAMETER 3
#define EC_ACCESS_DENIED 4
#define EC_INVALID_RESPONSE 5
#define EC_INVALID_VERSION 6
#define EC_INVALID_CHECKSUM 7

#define CROS_EC_CMD_MAX_REQUEST (0x100-8)

typedef struct _CROSEC_COMMAND {
	UINT32 version;
	UINT32 command;
	UINT32 outlen;
	UINT32 inlen;
	UINT32 result;
	UINT8 data[CROS_EC_CMD_MAX_REQUEST];
} * PCROSEC_COMMAND, CROSEC_COMMAND;

typedef struct _CROSEC_READMEM {
	ULONG offset;
	ULONG bytes;
	UCHAR buffer[CROSEC_MEMMAP_SIZE];
} * PCROSEC_READMEM, CROSEC_READMEM;


#include <pshpack1.h>
#define CROS_EC_CMD_MAX_KEY_COUNT 64
typedef struct {
	UINT8 r;
	UINT8 g;
	UINT8 b;
} Rgb;

typedef struct {
	UINT8 StartKey;
	UINT8 Length;
	Rgb Colors[CROS_EC_CMD_MAX_KEY_COUNT];
} EC_REQUEST_RGB_KBD_SET_COLOR;

#include <poppack.h>

int CrosEcSendCommand(
	HANDLE Handle,
	UINT16 command,
	UINT8 version,
	LPVOID outdata,
	unsigned int outlen,
	LPVOID indata,
	unsigned int inlen
	);
int CrosEcReadMemU8(HANDLE Handle, unsigned int offset, UINT8* dest);

#ifdef __cplusplus
}
#endif
