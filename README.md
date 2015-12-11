# alluris-tools

Collection of tools and generic driver for Alluris devices with USB measurement interface.

These are:
* FMI-S30 and FMI-S50
* FMI-B30 and FMI-B50
* CTT-200 and CTT-300
* FMT-W30 and FMT-W40

Error LIBALLURIS_DEVICE_BUSY is returned from liballuris if the device isn't supported.

## License

The library itself (liballuris.c and liballuris.h) is covered under the LGPL, gadc under the GPL.
See the comment header in the appropriate source files as well as COPYING and COPYING.LESSER.

## Documentation

Doxygen is used to build html and man pages. See ./doc and http://alluris.github.io/alluris-tools/

## USB access permissions

If you get the error "access denied" you may create a udev rule to set permissions.
In the example below anyone in the group "plugdev" can access the device:

```
$ cat /etc/udev/rules.d/90-alluris-usb.rules
ACTION=="add", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="fc30", MODE="0660", OWNER="root", GROUP="plugdev"
```

## Building

### Dependencies

* libusb 1.0 (http://www.libusb.org/)
* doxygen (for the manual)
* bats (https://github.com/sstephenson/bats if you want to run the tests)

### Debian GNU/Linux and derivatives

Install dependencies:

```
$ sudo apt-get install libusb-1.0-0-dev doxygen
```

then

```
$ ./bootstrap
$ ./configure
$ make
$ sudo make install
```

### Mac OS X Yosemite (10.10.4)

You may install Homebrew: http://brew.sh/, then run

```
$ brew install libtool automake libusb argp-standalone
```

then

```
$ ./bootstrap_mac
$ ./configure
$ make
$ make install
```

#### Optional: For auto-building the documentation you'll also need [Doxygen](http://www.stack.nl/~dimitri/doxygen/)
This will require some additional steps.

If you install the Doxygen GUI binary, you have to link it for using it as CLI:

```
$ sudo ln -s /Applications/Doxygen.app/Contents/Resources/doxygen /usr/local/bin
```
The `dot`-dependency is also missing. You need to change some permissions for the man-files before you can `brew` the containing `graphviz`-package.

```
$ sudo chmod 775 /usr/local/share/man/man3 /usr/local/share/man/man5 /usr/local/share/man/man7
$ brew install graphviz
```

### Windows

Download the MinGW binaries for libusb from http://libusb.info/ and use MinGW + MSYS to build the project.
Using other compilers and/or build systems should also be feasible.

## ToDo

* ~~Use GNU Build System~~ (Done)
* ~~Fix liballuris_set_mem_mode, liballuris_get_mem_mode~~ (Done)
* Overhaul liballuris_print_state, document liballuris_state
* ~~Test unit cN and oz on 5N or 10N device~~ (Done)
* Add bats test for gadc  --read-memory and --delete-memory to test/gadc_mem_mode.bats
* Check needed delay after "keypress" command
* Add bats test and doxygen documentation for new commands
+ get_next_calibration_date
+ read_flash
+ get_calibration_date
+ get_calibration_number
+ get_uncertainty
+ set_peak_level
+ get_peak_level
+ set_autostop
+ get_autostop
+ set_key_lock

* Implement full command set (see below)

### Not yet implemented low level commands

* ~~ID_READ_MEMORY~~
* ~~ID_DELETE_MEMORY~~
* ~~ID_STATISTIK~~
* ~~ID_TASTENDRUCK~~
* ID_DIGIN_CONFIG
* ID_STATUS
* ID_SET_SIGN
* ID_GET_SIGN
* ID_SET_DATA_RATIO
