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

## Documentation

Doxygen is used to build html and man pages. See ./doc

## USB access permissions

If you get an error "access denied" you can create a udev rule to change permissions.
In the example below anyone in the group "plugdev" can access the device:

```
$ cat /etc/udev/rules.d/90-alluris-usb.rules
ACTION=="add", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="fc30", MODE="0660", OWNER="root", GROUP="plugdev"
```

## Dependencies

* libusb 1.0 (http://www.libusb.org/)
* doxygen (for the manual)

### Debian GNU/Linux and derivatives

```
$ sudo apt-get install libusb-1.0-0-dev doxygen
```

###  Mac OS X Yosemite (10.10.4) Installation

You need to install Homebrew: http://brew.sh/, then run

```
$ brew install libusb argp-standalone
```

### Windows

Download the MinGW binaries from http://libusb.info/ and use MinGW+MSYS to build the project.
Using other compilers and/or build systems should also be feasible.

## Building

### Debian GNU/Linux and derivatives
The typical steps after cloning the git repo are

```
$ ./bootstrap
$ ./configure
$ make
$ make install
```

###Mac OS X

You firstly need to install some build-tools

```
$ brew install libtool automake
```


Then continue as follows:


```
$ ./bootstrap_mac
$ ./configure
$ make
$ make install
```

####Optional: For auto-building the documentation you'll also need [Doxygen](http://www.stack.nl/~dimitri/doxygen/)
This will require some additional steps.

If you install the Doxygen GUI binary, you have to link it for using it as CLI:

```
$ sudo ln -s /Applications/Doxygen.app/Contents/Resources/doxygen /usr/local/bin
```
The `dot`-dependency is also missing. Unfortunately, you need to change some permissions for the man-files before you can `brew` the containing `graphviz`-package.

```
$ sudo chmod 775 /usr/local/share/man/man3 /usr/local/share/man/man5 /usr/local/share/man/man7
$ brew install graphviz
```


## ToDo

* ~~Use GNU Build System~~ (Done)
* Implement full command set
