# Framework Windows ARGB Driver

This driver implements a HID device with support for the ["Lighting And IlluminationPage"](https://www.usb.org/sites/default/files/hutrr84_-_lighting_and_illumination_page.pdf)
for the ARGB connector on the Framework Desktop, that is commonly used with an RGB fan.

Applications/Drivers that can interface with this type of HID device can control the connected LEDs.
Additionally Windows has a "Dynamic Lighting" API that can be controlled via Windows settings and that third party applications can interact with.

## Applications

### Windows Dynamic Lighting

In Windows Settings => Personalization => Dynamic Lighting you can control the LEDs.

Applications:

- Microsoft Samples
  - [LampArray sample](https://github.com/microsoft/Windows-universal-samples/tree/main/Samples/LampArray) has multiple different ways for the user to control the LEDs
  - [Dynamic-Lighting-AutoRGB](https://github.com/microsoft/Dynamic-Lighting-AutoRGB) sets lighting based on average screen color

References:

- [End User Documentation](https://support.microsoft.com/en-us/windows/control-dynamic-lighting-devices-in-windows-8e8f22e3-e820-476c-8f9d-9ffc7b6ffcd2)
- [Developer Documentation](https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/dynamic-lighting-devices)
- Windows UWP API
  - [Overview](https://learn.microsoft.com/en-us/windows/uwp/devices-sensors/lighting-dynamic-lamparray)
  - [Windows.Devices.Lights](https://learn.microsoft.com/en-us/uwp/api/windows.devices.lights?view=winrt-26100)
  - [Windows.Devices.Lights.Effects](https://learn.microsoft.com/en-us/uwp/api/windows.devices.lights.effects?view=winrt-26100)
  - [Windows.Devices.Enumeration](https://learn.microsoft.com/en-us/uwp/api/windows.devices.enumeration.devicewatcher?view=winrt-26100)
- [Game Dev Documentation](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/lighting/gc-lighting-toc)

## Configuration

When the driver loads, the Windows Dynamic Lighting interface (or any HID
application) asks it about the LED configuration - how many and where in 3D
space.
This is so that they can know how to apply 2D or 3D animations to them.

By default the driver loads with an 8 LED configuration arranged in a circle of
20mm diameter. This matches the ARGB fan that Framework offers for the
Framework Desktop.

To customize the configuration, set/edit the following registry entries.
They are all under: `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\ROOT\HIDCLASS\0000\Device Parameters`

This configuration must be done after the driver is installed, then you disable the device, set the registry entries and then re-enable the device.

| Name             | Type  | Explanation                     |
|------------------|-------|---------------------------------|
| ReadFromRegistry | DWORD | If 1, the other values are read |
| LedCount         | DWORD | How many LEDs in total          |
| LedArrangement   | DWORD | How the LEDs are arranged       |

LedArrangement can have the following values:

- 0: Circular, layers of 8 (e.g. when 16 LEDs in total, two layers of 8 LEDs)
- 1: Circular, single layer
- 2: Linear (e.g. LED strip)
- 3: Square Matrix (works best with a square number of LEDs)

For example to control just 4 LEDs on the fan:

```
sudo reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\ROOT\HIDCLASS\0000\Device Parameters" /v ReadFromRegistry /t REG_DWORD /d 1
sudo reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\ROOT\HIDCLASS\0000\Device Parameters" /v LedCount /t REG_DWORD /d 4
sudo reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\ROOT\HIDCLASS\0000\Device Parameters" /v LedArrangement /t REG_DWORD /d 0
```

## Development

### Build

Follow [Microsoft's instructions](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
to install Visual Studio, Windows SDK and WDK.
I tried to install the SDK and WDK using winget but it either failed or couldn't get detected by Visual Studio.

Use Visual Studio 2022 Community Edition and build the project.

### Install

```
# Software device right now
cp "C:\Program Files (x86)\Windows Kits\10\Tools\10.0.26100.0\x64\devcon.exe" .
sudo .\devcon install .\FrameworkArgb\x64\Debug\FrameworkArgb\FrameworkArgb.inf root\FrameworkArgb

# Soon with ACPI device in updated BIOS
sudo pnputil /add-driver .\FrameworkArgb\x64\Debug\FrameworkArgb\FrameworkArgb.inf /install
```
