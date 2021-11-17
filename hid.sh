#!/bin/bash

modprobe libcomposite
cd /sys/kernel/config/usb_gadget/
mkdir -p g0
cd g0

echo 0x0e6f > idVendor # Linux Foundation
echo 0x0180 > idProduct # Multifunction Composite Gadget
echo 0x0572 > bcdDevice # v1.0.0
echo 0x0200 > bcdUSB # USB2
echo 0x00   > bDeviceClass
echo 0x00   > bDeviceSubClass
echo 0x00   > bDeviceProtocol
mkdir -p strings/0x409
echo "fedcbabcdef" > strings/0x409/serialnumber
echo "PDP CO.,LTD." > strings/0x409/manufacturer
echo "Faceoff Wired Pro Controller for Nintendo Switch" > strings/0x409/product

create_function() {
  mkdir -p functions/hid.usb$1
  echo 0 > functions/hid.usb$1/protocol
  echo 0 > functions/hid.usb$1/subclass
  echo 64 > functions/hid.usb$1/report_length # 8 bytes. 8 bits per byte
  echo -ne \\x05\\x01\\x09\\x05\\xA1\\x01\\x15\\x00\\x25\\x01\\x35\\x00\\x45\\x01\\x75\\x01\\x95\\x0E\\x05\\x09\\x19\\x01\\x29\\x0E\\x81\\x02  \\x95\\x02\\x81\\x01\\x05\\x01\\x25\\x07\\x46\\x3B\\x01\\x75\\x04\\x95  \\x01\\x65\\x14\\x09\\x39\\x81\\x42\\x65\\x00\\x95\\x01\\x81\\x01\\x26  \\xFF\\x00\\x46\\xFF\\x00\\x09\\x30\\x09\\x31\\x09\\x32\\x09\\x35\\x75  \\x08\\x95\\x04\\x81\\x02\\x75\\x08\\x95\\x01\\x81\\x01\\xC0 >  functions/hid.usb$1/report_desc
  # report descriptor from PDP CO.,LTD Faceoff Wired Pro Controller for Nintendo Switch
}

create_function 0 # /dev/hidg0
create_function 1 # /dev/hidg1
create_function 2 # /dev/hidg2
create_function 3 # /dev/hidg3

create_config() {
  mkdir -p configs/c.$1/strings/0x409
  echo "Gamepad Configuration" > configs/c.$1/strings/0x409/configuration
  echo 0x80 > configs/c.$1/bmAttributes
  echo 500 > configs/c.$1/MaxPower
}

link_config() {
    ln -s functions/hid.usb$2 configs/c.$1/
}

create_config 1 # create USB port 1 config

link_config 1 0 # link /dev/hidg0 to USB port 1
link_config 1 1 # link /dev/hidg1 to USB port 1
link_config 1 2 # link /dev/hidg2 to USB port 1
link_config 1 3 # link /dev/hidg3 to USB port 1

sudo ls /sys/class/udc > UDC # Setup USB gadget to USB OTG port

sleep 10 # Ensures that it is set up before we give it data
