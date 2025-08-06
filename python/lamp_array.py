#!/usr/bin/env python3
import os
import struct

# pip install hidapi
import hid

LIGHTING_USAGE_PAGE                = 0x59

LAMP_ARRAY_ATTRIBUTES_REPORT_ID    = 0x01
LAMP_ATTRIBUTES_REQUEST_REPORT_ID  = 0x02
LAMP_ATTRIBUTES_RESPONSE_REPORT_ID = 0x03
LAMP_MULTI_UPDATE_REPORT_ID        = 0x04
LAMP_RANGE_UPDATE_REPORT_ID        = 0x05
LAMP_ARRAY_CONTROL_REPORT_ID       = 0x06
CONTROL_COLLECTION_REPORT_ID       = 0x07

LAMP_ARRAY_ATTRIBUTES_FORMAT = '=HIIIIi'
LAMP_ATTRIBUTES_FORMAT = '=HIIIIIBBBBBB'

FWK_VID = 0x32AC
ARGB_PID = 0x0033

def print_lamp_details(device):
    positions_x = []
    positions_y = []

    h = hid.device()
    h.open_path(device['path'])

    array_attributes = h.get_feature_report(LAMP_ARRAY_ATTRIBUTES_REPORT_ID, 0x100)
    # Cut off report ID in first byte and convert to bytearray
    array_attributes = bytes(array_attributes[1:])

    (cnt, width, height, depth, kind, interval) = \
            struct.unpack(LAMP_ARRAY_ATTRIBUTES_FORMAT, array_attributes)

    print("Lamp Array")
    print(f"  Count:               {cnt}")
    print(f"  Width:               {width}um")
    print(f"  Height:              {height}um")
    print(f"  Depth:               {depth}um")
    if kind == 1:
        print(f"  Kind:                Keyboard")
    elif kind == 2:
        print(f"  Kind:                Mouse")
    elif kind == 7:
        print(f"  Kind:                Chassis")
    else:
        print(f"  Kind:                {kind}")
    print(f"  Min Update Interval: {interval}ms")

    h.send_feature_report([LAMP_ATTRIBUTES_REQUEST_REPORT_ID, 0x00, 0x00])
    for _ in range(cnt):
        lamp_attributes = h.get_feature_report(LAMP_ATTRIBUTES_RESPONSE_REPORT_ID, 0x100)
        # Cut off report ID in first byte and convert to bytearray
        lamp_attributes = bytes(lamp_attributes[1:])
        # print(lamp_attributes.hex())
        # print(len(lamp_attributes))
        (lamp_id, pos_x, pos_y, pos_z, latency, purpose, red, green, blue, intensity, programmable, input_binding) = \
                struct.unpack(LAMP_ATTRIBUTES_FORMAT, lamp_attributes)
        print("  Lamp")
        print(f"    ID:                  {lamp_id}")
        print(f"    Position:            ({pos_x},{pos_y},{pos_z})")
        print(f"    Latency:             {latency}ms")
        print(f"    Purpose:             {depth}")
        print(f"    Red Count:           {red}")
        print(f"    Green Count:         {green}")
        print(f"    Blue Count:          {blue}")
        print(f"    Intensity Count:     {intensity}")
        print(f"    Programmable:        {programmable}")
        print(f"    Input Binding:       {input_binding}")
        positions_x.append(pos_x)
        positions_y.append(pos_y)

    return (positions_x, positions_y)


def main():
    verbose = False
    devices = find_devs(show=False, verbose=verbose)
    for device in devices:
        try:
            positions = print_lamp_details(device)
            import matplotlib.pyplot as plt
            plt.scatter(x=positions[0], y=positions[1])
            plt.show()
        except struct.error as e:
            if verbose:
                print(e)
            continue
        except OSError as e:
            if verbose:
                print(e)
            continue

def format_bcd(fw_ver):
    fw_ver_major = (fw_ver & 0xFF00) >> 8
    fw_ver_minor = (fw_ver & 0x00F0) >> 4
    fw_ver_patch = (fw_ver & 0x000F)
    return f"{fw_ver_major}.{fw_ver_minor}.{fw_ver_patch}"


def find_devs(show, verbose):
    if verbose:
        show = True

    devices = []
    for device_dict in hid.enumerate():
        vid = device_dict["vendor_id"]
        pid = device_dict["product_id"]
        product = device_dict["product_string"]
        manufacturer = device_dict["manufacturer_string"]
        sn = device_dict['serial_number']
        interface = device_dict['interface_number']
        path = device_dict['path']
        usage_page = device_dict['usage_page']
        fw_ver = device_dict["release_number"]

        # For some reason on Linux it'll always show usage_page==0
        if (os.name == 'nt' and usage_page == LIGHTING_USAGE_PAGE) or verbose:
            if show:
                print(f"Manufacturer: {manufacturer}")
                print(f"Product:      {product}")
                print("FW Version:   {}".format(format_bcd(fw_ver)))
                print(f"Serial No:    {sn}")

            if verbose:
                print(f"VID/PID:      {vid:04X}:{pid:04X}")
                print(f"Interface:    {interface}")
                print(f"Usage Page:   {usage_page:04X}")
                # TODO: print Usage Page
                print("")


        if (vid == FWK_VID and pid == ARGB_PID) or usage_page == LIGHTING_USAGE_PAGE:
            devices.append(device_dict)

    return devices


if __name__ == "__main__":
    main()
