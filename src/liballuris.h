/*

Copyright (C) 2015 Alluris GmbH & Co. KG <weber@alluris.de>

This file is part of liballuris.

Liballuris is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Liballuris is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with liballuris. See ../COPYING.LESSER
If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libusb.h"

#ifndef liballuris_h
#define liballuris_h

//#define PRINT_DEBUG_MSG
#define MAX_NUM_DEVICES 4
#define SEND_TIMEOUT 10
#define RECEIVE_TIMEOUT 10

enum liballuris_error
{
  LIBALLURIS_SUCCESS = 0,
  LIBALLURIS_MALFORMED_REPLY = 1,
  LIBALLURIS_DEVICE_BUSY = 2
};

struct alluris_device_description
{
  libusb_device* dev;
  char product[30];
  char serial_number[30];
};

int get_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length);
int open_alluris_device (const char* serial_number, libusb_device_handle** h);
void free_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length);

int serial_number (libusb_device_handle *dev_handle, char* buf, size_t length);
int digits (libusb_device_handle *dev_handle, int* v);
int raw_value (libusb_device_handle *dev_handle, int* v);
int raw_pos_peak (libusb_device_handle *dev_handle, int* v);
int raw_neg_peak (libusb_device_handle *dev_handle, int* v);
#endif
