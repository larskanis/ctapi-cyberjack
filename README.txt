
=== short excerpt from the original documentation
=== for complete documentation please see the file
=== ctapi-cyberjack/doc/README.txt

1. Overview

   This driver for the REINER SCT cyberJack pinpad/e-com USB
   family of chipcard readers implements the CT-API 1.1 interface,
   as well as the PC/SC interface of pcsc-lite.

   It is implemented 100% in userspace. This means no trouble with
   different kernel versions, compiling/patching the kernel etc.

   All accesses are done via the usb devfs in /proc/bus/usb (or
   /dev/bus/usb for udev-based systems).

   Permission handling is done only via udev. The cyberjack.rules
   if installed to /etc/udev/rules.d is automatically called by
   udev when a reader is plugged in. This scripts dynamically
   updates the permissions of the respective device, so users in
   the group cyberjack are able to access it.

   For more information about the smart card reader itself please
   visit http://www.reiner-sct.com/. There is also a shop where
   the the readers can be ordered online.
     __________________________________________________________

2. Readers supported by this driver

   The following Reiner-SCT readers are supported:
   

                Product                     VendorID:ProductID
   REINER SCT cyberJack pinpad USB              0c4d:0100
   REINER SCT cyberJack e-com USB               0c4d:0100
   
   REINER SCT cyberJack pinpad(a) USB           0c4d:0300
   
   REINER SCT cyberJack e-com(a)                0c4d:0400
   REINER SCT cyberJack e-com plus DUO          0c4d:0400
   REINER SCT cyberJack e-com plus BIO          0c4d:0400
   REINER SCT cyberJack e-com plus RFID         0c4d:0400
   REINER SCT cyberJack e-com plus              0c4d:0400
   REINER SCT cyberJack Secoder                 0c4d:0400
   
   REINER SCT cyberJack e-com(f)                0c4d:0401
   REINER SCT cyberJack e-com BIO               0c4d:0401
   

   You can use the lsusb command to list all devices connected to
   the USB bus of your machine. It will print out the vendor and
   device ID of all your devices, like :

   Bus Nr Device Nr VeID:PrID Bus 002 Device 002: ID 0451:1446
   Texas Instruments, Inc. TUSB2040/2070 Hub Bus 002 Device 006:
   ID 0c4b:0300

   The REINER SCT VendorID is 0c4b. ProductID's can be looked up
   in the table above.
     __________________________________________________________

