# Framework Windows ARGB Driver

Resources:

- Documentation
  - https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/dynamic-lighting-devices
  - https://www.usb.org/sites/default/files/hutrr84_-_lighting_and_illumination_page.pdf
  - https://www.usb.org/document-library/hid-usage-tables-16
- Drivers
    - https://github.com/microsoft/Windows-driver-samples/tree/main/hid/vhidmini2
      - Can use this as a base, pretends to be a HID device
- Firmware
    - https://github.com/hathach/tinyusb/blob/29ffd57237554b1f2339af543e3789ae04d3b29b/src/class/hid/hid_device.h#L495
    - https://github.com/microsoft/RP2040MacropadHidSample/blob/main/src/usb_descriptors.c#L76
  - https://github.com/microsoft/ArduinoHidForWindows
- Software
  - https://github.com/microsoft/Dynamic-Lighting-AutoRGB
  - https://github.com/microsoft/Windows-universal-samples/tree/main/Samples/LampArray

Implementation steps:

- [x] Make a UMDF driver
- [ ] Make it act as a HID device
  - [ ] https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/creating-umdf-hid-minidrivers
    - [x] Set INF directives
- [ ] Adjust HID report descriptor to Lighting and Illumination spec
- [ ] Test with Lighting Settings
- [ ] Hook up reports to CrosEC
- [ ] Sign it with EV Cert
- [ ] Run HLK tests
- [ ] Sign it with WHQL

## Development

Follow [Microsoft's instructions](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
to install Visual Studio, Windows SDK and WDK.
I tried to install the SDK and WDK using winget but it either failed or couldn't get detected by Visual Studio.

### Build

Use Visual Studio 2022 Community Edition.

### Install

```
sudo pnputil /add-driver .\FrameworkArgb\x64\Debug\FrameworkArgb\FrameworkArgb.inf /install
cp "C:\Program Files (x86)\Windows Kits\10\Tools\10.0.26100.0\x64\devcon.exe" .
sudo .\devcon install .\FrameworkArgb\x64\Debug\FrameworkArgb\FrameworkArgb.inf root\FrameworkArgb
```
