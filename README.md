# liballuris
Generic driver for Alluris devices with USB measurement interface.

These are:
* FMI-S30 and FMI-S50
* FMI-B30 and FMI-B50
* CTT-200 and CTT-300
* FMT-W30 and FMT-W40

## License

The library itself (liballuris.c and liballuris.h) is covered under the LGPL, gadc under the GPL.
See the comment header in the appropriate source files as well as COPYING and COPYING.LESSER.

## USB access permissions

If you get an error "access denied" you can create a udev rule to change permissions.
In the example below anyone in the group "plugdev" can access the device:

```
$ cat /etc/udev/rules.d/90-alluris-usb.rules
ACTION=="add", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="fc30", MODE="0660", OWNER="root", GROUP="plugdev"
```

## Dependencies

* libusb 1.0 (http://www.libusb.org/)

##  Mac OS X Yosemite (10.10.4) Installation

You first need to install MacPorts for the argp-Library: https://www.macports.org/, then run
```
$ port install argp-standalone
```

Secondly you need to install Homebrew for the libusb-Library: http://brew.sh/, then run
```
$ brew install libusb
```

Now you can compile the driver in src-dir
```
$ make -f Makefile.mac
```
