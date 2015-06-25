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
 * \file liballuris.c
 * \brief Implementation of generic Alluris device driver
*/

#include "liballuris.h"

// minimum length of "in" is 2 bytes
static short char_to_uint16 (unsigned char* in)
{
  short ret;
  memcpy (&ret, in, 2);
  return ret;
}

// minimum length of "in" is 3 bytes
static int char_to_uint24 (unsigned char* in)
{
  int ret;
  memcpy (&ret, in, 3);
  return ret;
}

// minimum length of "in" is 3 bytes
static int char_to_int24 (unsigned char* in)
{
  int ret = char_to_uint24 (in);
  if (ret > 8388607)     // 2**23-1
    ret = ret- 16777216; // 2**24
  return ret;
}

// send/receive wrapper
static int device_bulk_transfer (libusb_device_handle* dev_handle,
                                 unsigned char* out_buf,
                                 int out_buf_length,
                                 unsigned char* in_buf,
                                 int in_buf_length)
{
  int actual;
  int r = 0;

  // check length in out_buf
  assert (out_buf [1] == out_buf_length);
  r = libusb_bulk_transfer (dev_handle, (0x1 | LIBUSB_ENDPOINT_OUT), out_buf, out_buf_length, &actual, SEND_TIMEOUT);

#ifdef PRINT_DEBUG_MSG
  if ( r == LIBUSB_SUCCESS)
    {
      int k_dbgs;
      printf ("Sent     %2i from %2i bytes: ", actual, out_buf_length);
      for (k_dbgs=0; k_dbgs < actual; ++k_dbgs)
        {
          printf ("%i", out_buf[k_dbgs]);
          if (k_dbgs < (actual - 1))
            printf (", ");
        }
      printf ("\n");
    }
#endif

  if (r != LIBUSB_SUCCESS || actual != out_buf_length)
    {
      fprintf(stderr, "Write Error: '%s', wrote %i of %i bytes.\n", libusb_error_name(r), actual, out_buf_length);
      return r;
    }

  r = libusb_bulk_transfer (dev_handle, 0x81 | LIBUSB_ENDPOINT_IN, in_buf, in_buf_length, &actual, RECEIVE_TIMEOUT);

#ifdef PRINT_DEBUG_MSG
  if ( r == LIBUSB_SUCCESS)
    {
      int k_dbgr;
      printf ("Received %2i from %2i bytes: ", actual, in_buf_length);
      for (k_dbgr=0; k_dbgr < actual; ++k_dbgr)
        {
          printf ("%i", in_buf[k_dbgr]);
          if (k_dbgr < (actual - 1))
            printf (", ");
        }
      printf ("\n");
    }
#endif

  if (r != LIBUSB_SUCCESS || actual != in_buf_length)
    {
      fprintf(stderr, "Read Error: '%s', tried to read %i, got %i bytes.\n", libusb_error_name(r), in_buf_length, actual);
      return r;
    }

  // check reply
  if (in_buf[0] != out_buf[0]
      || in_buf[1] != actual
      || in_buf[2] != out_buf[2])
    {
      fprintf(stderr, "Error: Malformed reply header. Check physical connection and EMI.\n");
      return LIBALLURIS_MALFORMED_REPLY;
    }

  return r;
}

/****************************************************************************************/

// List accessible alluris devices
// check permissions if a device isn't returned (see Readme.md)
int get_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length)
{
  size_t k = 0;
  for (k=0; k<length; ++k)
    alluris_devs[k].dev = NULL;

  size_t num_alluris_devices = 0;
  libusb_device **devs;
  ssize_t cnt = libusb_get_device_list (NULL, &devs);
  if (cnt > 0)
    {
      libusb_device *dev;
      int i = 0;
      while ((dev = devs[i++]) != NULL)
        {
          struct libusb_device_descriptor desc;
          int r = libusb_get_device_descriptor (dev, &desc);
          if (r < 0)
            {
              fprintf (stderr, "failed to get device descriptor: %s", libusb_error_name(r));
              continue;
            }

          //check for compatible devices
          if (desc.idVendor == 0x04d8 && desc.idProduct == 0xfc30)
            {
              alluris_devs[num_alluris_devices].dev = dev;

              //open device
              libusb_device_handle* h;
              r = libusb_open (dev, &h);
              if (r == LIBUSB_SUCCESS)
                {
                  r = libusb_claim_interface (h, 0);
                  if (r == LIBUSB_SUCCESS)
                    {

                      if(desc.iProduct)
                        r = libusb_get_string_descriptor_ascii(h, desc.iProduct, (unsigned char*)alluris_devs[num_alluris_devices].product, sizeof (alluris_devs[0].product));
                      else
                        strncpy (alluris_devs[num_alluris_devices].product, "No product information available", sizeof (alluris_devs[0].product));

                      // get serial number from device
                      serial_number (h, alluris_devs[num_alluris_devices].serial_number, sizeof (alluris_devs[0].serial_number));

                      num_alluris_devices++;
                      libusb_release_interface (h, 0);
                      libusb_close (h);
                    }
                  else
                    fprintf (stderr, "Couldn't claim interface: %s\n", libusb_error_name(r));

                }
              else
                fprintf (stderr, "Couldn't open device: %s\n", libusb_error_name(r));
            }
          else
            // unref non Alluris device
            libusb_unref_device (dev);

          if (num_alluris_devices == length)
            // maximum number of devices reached
            return length;
        }
    }
  return num_alluris_devices;
}

void free_alluris_device_list (struct alluris_device_description* alluris_devs, size_t length)
{
  size_t k = 0;
  for (k=0; k < length; ++k)
    if (alluris_devs[k].dev)
      {
        libusb_unref_device (alluris_devs[k].dev);
        alluris_devs[k].dev = NULL;
      }
}

// Open device with specified serial_number or the first available if NULL
int open_alluris_device (const char* serial_number, libusb_device_handle** h)
{
  int k;
  libusb_device *dev = NULL;
  struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
  int cnt = get_alluris_device_list (alluris_devs, MAX_NUM_DEVICES);

  if (cnt >= 1)
    {
      if (serial_number == NULL)
        dev = alluris_devs[0].dev;
      else
        for (k=0; k < cnt; k++)
          if (! strncmp (serial_number, alluris_devs[k].serial_number, sizeof (alluris_devs[0].serial_number)))
            dev = alluris_devs[k].dev;
    }
  else
    //no device found
    return LIBUSB_ERROR_NOT_FOUND;

  int ret = libusb_open (dev, h);
  free_alluris_device_list (alluris_devs, MAX_NUM_DEVICES);
  return ret;
}

/*!
 * \brief Query the serial number
 *
 * Return the serial number of the device which is also laser-engraved on the back.
 * For example "P.25412"
 *
 * Query the serial number is only possible if the measurement is not running,
 * else LIBALLURIS_DEVICE_BUSY is returned.
 *
 * \param dev_handle a handle for the device to communicate with
 * \param buf output location for the serial number. Only populated when the return code is 0.
 * \param length length of buffer in bytes
 * \return 0 if successful else error code. LIBALLURIS_DEVICE_BUSY if measurement is running
 */
int serial_number (libusb_device_handle *dev_handle, char* buf, size_t length)
{
  unsigned char data[6];
  data[0] = 0x08;
  data[1] = 3;
  data[2] = 6;
  int ret = device_bulk_transfer (dev_handle, data, 3, data, 6);
  if (ret == LIBUSB_SUCCESS)
    {
      short tmp = char_to_uint16 (data + 3);
      if (tmp == -1)
        return LIBALLURIS_DEVICE_BUSY;
      else
        snprintf (buf, length, "%c.%i", data[5] + 'A', tmp);
    }
  return ret;
}

/*!
 * \brief Query the number of digits for the interpretation of the raw fixed-point numbers
 *
 * All raw_* functions returns fixed-point numbers. This function queries the number
 * of digits after the radix point. For example if raw_value returns 123 and digits returns 1
 * the real value is 12.3
 *
 * Query this value is only possible if the measurement is not running,
 * else LIBALLURIS_DEVICE_BUSY is returned.
 *
 * \param dev_handle a handle for the device to communicate with
 * \param v output location for the returned number of digits. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa raw_value (libusb_device_handle *dev_handle, int* v)
 */
int digits (libusb_device_handle *dev_handle, int* v)
{
  unsigned char data[6];
  data[0] = 0x08;
  data[1] = 3;
  data[2] = 3;
  int ret = device_bulk_transfer (dev_handle, data, 3, data, 6);
  if (ret == LIBUSB_SUCCESS)
    {
      int tmp = char_to_int24 (data + 3);
      if (tmp == -1)
        return LIBALLURIS_DEVICE_BUSY;
      else
        *v = tmp;
    }
  return ret;
}

/*!
 * \brief Query the current measurement value
 *
 * \param dev_handle a handle for the device to communicate with
 * \param v output location for the measurement value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int raw_value (libusb_device_handle *dev_handle, int* v)
{
  unsigned char data[6];
  data[0] = 0x46;
  data[1] = 3;
  data[2] = 3;
  int ret = device_bulk_transfer (dev_handle, data, 3, data, 6);
  if (ret == LIBUSB_SUCCESS)
    *v = char_to_int24 (data + 3);
  return ret;
}

/*!
 * \brief Query positive peak value
 *
 * \param dev_handle a handle for the device to communicate with
 * \param v output location for the peak value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int raw_pos_peak (libusb_device_handle *dev_handle, int* v)
{
  unsigned char data[6];
  data[0] = 0x46;
  data[1] = 3;
  data[2] = 4;
  int ret = device_bulk_transfer (dev_handle, data, 3, data, 6);
  if (ret == LIBUSB_SUCCESS)
    *v = char_to_int24 (data + 3);
  return ret;
}

/*!
 * \brief Query negative peak value
 *
 * \param dev_handle a handle for the device to communicate with
 * \param v output location for the peak value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int raw_neg_peak (libusb_device_handle *dev_handle, int* v)
{
  unsigned char data[6];
  data[0] = 0x46;
  data[1] = 3;
  data[2] = 5;
  int ret = device_bulk_transfer (dev_handle, data, 3, data, 6);
  if (ret == LIBUSB_SUCCESS)
    *v = char_to_int24 (data + 3);
  return ret;
}
