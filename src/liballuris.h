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

/*!
 * \file liballuris.h
 * \brief Header for generic Alluris device driver
*/

/*! \mainpage liballuris API Reference
 *
 * Generic driver for Alluris devices with USB measurement interface.
 *
 * - \ref liballuris.h
 * - \ref liballuris.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libusb.h"

#ifndef liballuris_h
#define liballuris_h

//#define PRINT_DEBUG_MSG
//! Number of device which can be enumerated and simultaneously opened
#define MAX_NUM_DEVICES 4
//! Timeout in milliseconds while writing to the device
#define SEND_TIMEOUT 10

/*!
 * \brief Timeout in milliseconds while reading from the device
 *
 * The sampling frequency can be selected between 10Hz and 990Hz
 * Therefore the maximum delay until the measurement completes is 1/10Hz = 100ms.
 * Additional 10% -> 110ms
 */
#define RECEIVE_TIMEOUT 110

//! liballuris specific errors
enum liballuris_error
{
  LIBALLURIS_SUCCESS = 0,
  LIBALLURIS_MALFORMED_REPLY = 1, //!< The received reply contains a malformed header. This should never happen, check EMI and physical connection
  LIBALLURIS_DEVICE_BUSY = 2      //!< Device is in a state where it cannot process the request
};

/*!
 * \brief composition of libusdb device and alluris device informations
 *
 * product and serial_number should help to identify a specific alluris device if more than one
 * device is connected via USB.
 * \sa get_alluris_device_list, open_alluris_device, free_alluris_device_list
 */
struct alluris_device_description
{
  libusb_device* dev;     //!< pointer to a structure representing a USB device detected on the system
  char product[30];       //!< product identification, for exmample "FMI-S Force-Gauge"
  char serial_number[30]; //!< serial number of device, for example "P.25412"
};

int get_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length, char read_serial);
int open_alluris_device (const char* serial_number, libusb_device_handle** h);
void free_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length);

int serial_number (libusb_device_handle *dev_handle, char* buf, size_t length);
int digits (libusb_device_handle *dev_handle, int* v);
int raw_value (libusb_device_handle *dev_handle, int* value);
int raw_pos_peak (libusb_device_handle *dev_handle, int* peak);
int raw_neg_peak (libusb_device_handle *dev_handle, int* peak);

int cyclic_measurement (libusb_device_handle *dev_handle, char enable, size_t length);
int poll_measurement (libusb_device_handle *dev_handle, int* buf, size_t length);

int tare (libusb_device_handle *dev_handle);
int clear_pos_peak (libusb_device_handle *dev_handle);
int clear_neg_peak (libusb_device_handle *dev_handle);
#endif
