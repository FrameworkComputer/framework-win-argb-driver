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
- [ ] Adjust HID report descriptor to Lighting and Illumination spec
- [ ] Test with Lighting Settings
- [ ] Hook up reports to CrosEC
- [ ] Sign it with EV Cert
- [ ] Run HLK tests
- [ ] Sign it with WHQL
