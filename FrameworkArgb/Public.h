/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    driver and application

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_FrameworkArgb,
    0x1cb4f6fe,0xb974,0x4e4f,0xbf,0xb9,0x43,0x1e,0x71,0xbb,0x01,0xa8);
// {1cb4f6fe-b974-4e4f-bfb9-431e71bb01a8}
