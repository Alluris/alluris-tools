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
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#ifndef liballuris_h
#define liballuris_h

//#define PRINT_DEBUG_MSG
//#define DEBUG_TIMING

//! Number of device which can be enumerated and simultaneously opened
#define MAX_NUM_DEVICES 4

//! Default timeout in milliseconds while writing to the device
#define DEFAULT_SEND_TIMEOUT 50

//! Default timeout in milliseconds while reading from the device
#define DEFAULT_RECEIVE_TIMEOUT 100

//! Default send buffer size. Should be multiple of wMaxPacketSize
#define DEFAULT_SEND_BUF_LEN 64
//! Default receive buffer size.
#define DEFAULT_RECV_BUF_LEN 256

//! liballuris specific errors
enum liballuris_error
{
  LIBALLURIS_SUCCESS         = 0, //!< No error
  LIBALLURIS_MALFORMED_REPLY = 1, //!< The received reply contains a malformed header. This should never happen, check EMI and physical connection
  //! \brief Device is in a state where it cannot process the request.
  //! The device cannot perform the requested operation.
  //! Some examples:
  //! - get digits or units while measurement is running
  //! - get or change upper or lower limit while measurement is running
  //! - USB API not available, for example on a FMI-S10 device
  LIBALLURIS_DEVICE_BUSY     = 2,
  LIBALLURIS_TIMEOUT         = 3, //!< No response or status change in given time
  //! \brief Parameter out of valid range or invalid mode/state
  //! For example set unit which isn't supported (for example 'oz' for 500N device)
  LIBALLURIS_OUT_OF_RANGE    = 4
};

//! measurement mode
enum liballuris_measurement_mode
{
  LIBALLURIS_MODE_STANDARD = 0, //!< 10Hz sampling rate
  LIBALLURIS_MODE_PEAK     = 1, //!< 900Hz sampling rate
  LIBALLURIS_MODE_PEAK_MAX = 2, //!< 900Hz sampling rate, maximum detection
  LIBALLURIS_MODE_PEAK_MIN = 3  //!< 900Hz sampling rate, minimum detection
};

//! memory mode
enum liballuris_memory_mode
{
  LIBALLURIS_MEM_MODE_DISABLED   = 0, //!< no memory active
  LIBALLURIS_MEM_MODE_SINGLE     = 1, //!< Store single value with key (S)
  LIBALLURIS_MEM_MODE_CONTINUOUS = 2  //!< Start/Stop continuous capturing to memory with key (S)
};

//! liballuris_unit
enum liballuris_unit
{
  LIBALLURIS_UNIT_N   = 0, //!< newton
  LIBALLURIS_UNIT_cN  = 1, //!< centinewton (only devices with range 5N or 10N)
  LIBALLURIS_UNIT_kg  = 2, //!< kg (only devices with range > 10N)
  LIBALLURIS_UNIT_g   = 3, //!< g (only devices with range 5N or 10N)
  LIBALLURIS_UNIT_lb  = 4, //!< lb (only devices with range > 10N)
  LIBALLURIS_UNIT_oz  = 5, //!< oz (only devices with range 5N or 10N)
};

//! liballuris_state
struct liballuris_state
{
  unsigned char dummy0                : 1; //!< dummy/unused
  unsigned char upper_limit_exceeded  : 1; //!< Force >= upper limit
  unsigned char lower_limit_underrun  : 1; //!< Force <= lower limit
  unsigned char some_peak_mode_active : 1; //!< Peak, Peak+ or Peak- active
  unsigned char peak_plus_active      : 1; //!< Peak+ is active
  unsigned char peak_minus_active     : 1; //!< Peak- is active

  //! \brief Capturing to memory in progress.
  //! This flag indicates memory write access. In continues mode this flag goes zero
  //! if the memory is full and storing to memory has stopped for this reason.
  //! \sa liballuris_memory_mode
  unsigned char mem_running           : 1;
  unsigned char dummy7                : 1; //!< dummy/unused
  unsigned char dummy8                : 1; //!< dummy/unused
  unsigned char dummy9                : 1; //!< dummy/unused
  unsigned char overload              : 1; //!< Absolute limit (abs(F) > 150% of nominal range) is exceeded
  unsigned char fracture              : 1; //!< Only W20/W40
  unsigned char dummy12               : 1; //!< dummy/unused

  //! \brief 1 = Memory function active (P21=1 or P21=2)
  //! \sa liballuris_memory_mode
  unsigned char mem_active            : 1;

  //! \brief Store with fixed rate or single values.
  //! - 1 = Store values with display rate (3, 5 or 10Hz) in memory (P21=2)
  //! - 0 = Store single value on keypress (P21=1)
  //! \sa liballuris_memory_mode
  unsigned char mem_conti             : 1;
  unsigned char dummy15               : 1; //!< dummy/unused
  unsigned char grenz_option          : 1; //!< FIXME
  unsigned char dummy17               : 1; //!< dummy/unused
  unsigned char dummy18               : 1; //!< dummy/unused
  unsigned char dummy19               : 1; //!< dummy/unused
  unsigned char dummy20               : 1; //!< dummy/unused
  unsigned char dummy21               : 1; //!< dummy/unused
  unsigned char dummy22               : 1; //!< dummy/unused
  unsigned char measuring             : 1; //!< Measurement is running
};

//! __liballuris_state__
union __liballuris_state__
{
  struct liballuris_state bits;           //!< bit access
  unsigned int _int;                      //!< uint32 access
};

/*!
 * \brief composition of libusb device and Alluris device information
 *
 * product and serial_number should help to identify a specific Alluris device if more than one
 * device is connected via USB.
 * \sa get_alluris_device_list, open_alluris_device, free_alluris_device_list
 */
struct alluris_device_description
{
  libusb_device* dev;     //!< pointer to a structure representing a USB device detected on the system
  char product[30];       //!< product identification, for example "FMI-S Force-Gauge"
  char serial_number[30]; //!< serial number of device, for example "P.25412"
};

#ifdef __cplusplus
extern "C"
{
#endif

const char * liballuris_error_name (int error_code);
const char * liballuris_unit_enum2str (enum liballuris_unit unit);
enum liballuris_unit liballuris_unit_str2enum (const char *str);

int liballuris_get_device_list (libusb_context* ctx, struct alluris_device_description* alluris_devs, size_t length, char read_serial);
int liballuris_open_device (libusb_context* ctx, const char* serial_number, libusb_device_handle** h);
int liballuris_open_device_with_id (libusb_context* ctx, int bus, int device, libusb_device_handle** h);
void liballuris_free_device_list (struct alluris_device_description* alluris_devs, size_t length);

void liballuris_clear_RX (libusb_device_handle* dev_handle, unsigned int timeout);

int liballuris_get_serial_number (libusb_device_handle *dev_handle, char* buf, size_t length);
int liballuris_get_firmware (libusb_device_handle *dev_handle, int dev, char* buf, size_t length);
int liballuris_get_next_calibration_date (libusb_device_handle *dev_handle, int* v);
int liballuris_read_flash (libusb_device_handle *dev_handle, int adr, unsigned short *v);
int liballuris_get_calibration_date (libusb_device_handle *dev_handle, unsigned short* v);
int liballuris_get_calibration_number (libusb_device_handle *dev_handle, char* buf, size_t length);
int liballuris_get_uncertainty (libusb_device_handle *dev_handle, double* v);

int liballuris_get_digits (libusb_device_handle *dev_handle, int* v);
int liballuris_get_resolution (libusb_device_handle *dev_handle, int* v);
int liballuris_get_F_max (libusb_device_handle *dev_handle, int* fmax);

int liballuris_get_value (libusb_device_handle *dev_handle, int* value);
int liballuris_get_pos_peak (libusb_device_handle *dev_handle, int* peak);
int liballuris_get_neg_peak (libusb_device_handle *dev_handle, int* peak);

/* read and print state */
int liballuris_read_state (libusb_device_handle *dev_handle, struct liballuris_state* state, unsigned int timeout);
void liballuris_print_state (struct liballuris_state state);

int liballuris_cyclic_measurement (libusb_device_handle *dev_handle, char enable, size_t length);
int liballuris_poll_measurement (libusb_device_handle *dev_handle, int* buf, size_t length);
int liballuris_poll_measurement_no_wait (libusb_device_handle *dev_handle, int* buf, size_t length, size_t *actual_num_values);

int liballuris_tare (libusb_device_handle *dev_handle);
int liballuris_clear_pos_peak (libusb_device_handle *dev_handle);
int liballuris_clear_neg_peak (libusb_device_handle *dev_handle);

int liballuris_start_measurement (libusb_device_handle *dev_handle);
int liballuris_stop_measurement (libusb_device_handle *dev_handle);

int liballuris_set_upper_limit (libusb_device_handle *dev_handle, int limit);
int liballuris_set_lower_limit (libusb_device_handle *dev_handle, int limit);

int liballuris_get_upper_limit (libusb_device_handle *dev_handle, int* limit);
int liballuris_get_lower_limit (libusb_device_handle *dev_handle, int* limit);

int liballuris_set_mode (libusb_device_handle *dev_handle, enum liballuris_measurement_mode mode);
int liballuris_get_mode (libusb_device_handle *dev_handle, enum liballuris_measurement_mode *mode);

int liballuris_set_mem_mode (libusb_device_handle *dev_handle, enum liballuris_memory_mode mode);
int liballuris_get_mem_mode (libusb_device_handle *dev_handle, enum liballuris_memory_mode *mode);

int liballuris_set_unit (libusb_device_handle *dev_handle, enum liballuris_unit unit);
int liballuris_get_unit (libusb_device_handle *dev_handle, enum liballuris_unit *unit);

int liballuris_set_digout (libusb_device_handle *dev_handle, int v);
int liballuris_get_digout (libusb_device_handle *dev_handle, int *v);

int liballuris_restore_factory_defaults (libusb_device_handle *dev_handle);
int liballuris_power_off (libusb_device_handle *dev_handle);

int liballuris_read_memory (libusb_device_handle *dev_handle, int adr, int* mem_value);
int liballuris_delete_memory (libusb_device_handle *dev_handle);
int liballuris_get_mem_count (libusb_device_handle *dev_handle, int* v);

int liballuris_get_mem_statistics (libusb_device_handle *dev_handle, int* stats, size_t length);

int liballuris_sim_keypress (libusb_device_handle *dev_handle, unsigned char mask);

int liballuris_set_peak_level (libusb_device_handle *dev_handle, int v);
int liballuris_get_peak_level (libusb_device_handle *dev_handle, int *v);

int liballuris_set_autostop (libusb_device_handle *dev_handle, int v);
int liballuris_get_autostop (libusb_device_handle *dev_handle, int *v);

int liballuris_set_key_lock (libusb_device_handle *dev_handle, char active);

#ifdef __cplusplus
}
#endif

#endif
