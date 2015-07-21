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
#include <libusb-1.0/libusb.h>

#ifndef liballuris_h
#define liballuris_h

//#define PRINT_DEBUG_MSG
//! Number of device which can be enumerated and simultaneously opened
#define MAX_NUM_DEVICES 4
//! Timeout in milliseconds while writing to the device
#define SEND_TIMEOUT 100

/*!
 * \brief Timeout in milliseconds while reading from the device
 *
 * The sampling frequency can be selected between 10Hz and 990Hz
 * Therefore the maximum delay until the measurement completes is 1/10Hz = 100ms.
 * Additional 50% -> 150ms
 * FIXME: temporary increased to 500ms
 */
#define RECEIVE_TIMEOUT 500

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

int liballuris_device_bulk_transfer (libusb_device_handle* dev_handle,
                                     unsigned char* out_buf,
                                     int out_buf_length,
                                     unsigned char* in_buf,
                                     int in_buf_length);

int liballuris_get_device_list (libusb_context* ctx, struct alluris_device_description* alluris_devs, size_t length, char read_serial);
int liballuris_open_device (libusb_context* ctx, const char* serial_number, libusb_device_handle** h);
int liballuris_open_device_with_id (libusb_context* ctx, int bus, int device, libusb_device_handle** h);
void liballuris_free_device_list (struct alluris_device_description* alluris_devs, size_t length);

void liballuris_clear_RX (libusb_device_handle* dev_handle);

int liballuris_serial_number (libusb_device_handle *dev_handle, char* buf, size_t length);
int liballuris_digits (libusb_device_handle *dev_handle, int* v);
int liballuris_raw_value (libusb_device_handle *dev_handle, int* value);
int liballuris_raw_pos_peak (libusb_device_handle *dev_handle, int* peak);
int liballuris_raw_neg_peak (libusb_device_handle *dev_handle, int* peak);

int liballuris_cyclic_measurement (libusb_device_handle *dev_handle, char enable, size_t length);
int liballuris_poll_measurement (libusb_device_handle *dev_handle, int* buf, size_t length);

int liballuris_tare (libusb_device_handle *dev_handle);
int liballuris_clear_pos_peak (libusb_device_handle *dev_handle);
int liballuris_clear_neg_peak (libusb_device_handle *dev_handle);

int liballuris_start_measurement (libusb_device_handle *dev_handle);
int liballuris_stop_measurement (libusb_device_handle *dev_handle);

int liballuris_set_pos_limit (libusb_device_handle *dev_handle, int limit);
int liballuris_set_neg_limit (libusb_device_handle *dev_handle, int limit);

int liballuris_get_pos_limit (libusb_device_handle *dev_handle, int* limit);
int liballuris_get_neg_limit (libusb_device_handle *dev_handle, int* limit);

#endif
