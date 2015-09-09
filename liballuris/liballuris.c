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

static unsigned char out_buf[DEFAULT_SEND_BUF_LEN];
static unsigned char in_buf[DEFAULT_RECV_BUF_LEN];

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

/*!
 * Returns a constant NULL-terminated string with the ASCII name of a libusb
 * or liballuris error code. The caller must not free() the returned string.
 *
 * \param error_code The \ref error code to return the name of.
 * \returns The error name, or the string **UNKNOWN** if the value of
 * error_code is not a known error code.
 */
const char * liballuris_error_name (int error_code)
{
  enum libusb_error error = error_code;

  if (error < 0)
    return libusb_error_name (error);
  else
    {
      switch (error)
        {
        case LIBALLURIS_SUCCESS:
          return "LIBALLURIS_SUCCESS";
        case LIBALLURIS_MALFORMED_REPLY:
          return "LIBALLURIS_MALFORMED_REPLY";
        case LIBALLURIS_DEVICE_BUSY:
          return "LIBALLURIS_DEVICE_BUSY";
        case LIBALLURIS_TIMEOUT:
          return "LIBALLURIS_TIMEOUT";
        case LIBALLURIS_OUT_OF_RANGE:
          return "LIBALLURIS_OUT_OF_RANGE";
        }
    }
  return "**UNKNOWN**";
}

const char * liballuris_unit_enum2str (enum liballuris_unit unit)
{
  switch (unit)
    {
    case LIBALLURIS_UNIT_N:
      return "N";
    case LIBALLURIS_UNIT_cN:
      return "cN";
    case LIBALLURIS_UNIT_kg:
      return "kg";
    case LIBALLURIS_UNIT_g:
      return "g";
    case LIBALLURIS_UNIT_lb:
      return "lb";
    case LIBALLURIS_UNIT_oz:
      return "oz";
    }
  return "**UNKNOWN UNIT**";
}

enum liballuris_unit liballuris_unit_str2enum (const char *str)
{
  if (! strcmp (str, "N"))
    return LIBALLURIS_UNIT_N;
  else if (! strcmp (str, "cN"))
    return LIBALLURIS_UNIT_cN;
  else if (! strcmp (str, "kg"))
    return LIBALLURIS_UNIT_kg;
  else if (! strcmp (str, "g"))
    return LIBALLURIS_UNIT_g;
  else if (! strcmp (str, "lb"))
    return LIBALLURIS_UNIT_lb;
  else if (! strcmp (str, "oz"))
    return LIBALLURIS_UNIT_oz;
  else
    return -1;
}

static void print_buffer (unsigned char* buf, int len)
{
  int k;
  for (k=0; k < len; ++k)
    {
      printf ("0x%02x", buf[k]);
      if (k < (len - 1))
        printf (", ");
    }
  printf ("\n");
}

// send/receive wrapper
static int liballuris_device_bulk_transfer (libusb_device_handle* dev_handle,
    const char* funcname,
    int send_len,
    unsigned int send_timeout,
    int reply_len,
    unsigned int receive_timeout)
{
  int actual;
  int r = 0;

  if (send_len > DEFAULT_SEND_BUF_LEN)
    {
      fprintf (stderr, "Error: Send len %i > receive buffer len %i. This looks like a programming error.\n", send_len, DEFAULT_SEND_BUF_LEN);
      exit (-1);
    }

  if (reply_len > DEFAULT_RECV_BUF_LEN)
    {
      fprintf (stderr, "Error: Reply len %i > receive buffer len %i. This looks like a programming error.\n", reply_len, DEFAULT_RECV_BUF_LEN);
      exit (-1);
    }

  if (send_len > 0)
    {
      // check length in out_buf
      assert (out_buf[1] == send_len);
      r = libusb_bulk_transfer (dev_handle, (0x1 | LIBUSB_ENDPOINT_OUT), out_buf, send_len, &actual, send_timeout);

#ifdef PRINT_DEBUG_MSG
      if ( r == LIBUSB_SUCCESS)
        {
          printf ("%s sent %2i/%2i bytes: ", funcname, actual, send_len);
          print_buffer (out_buf, actual);
        }
#endif

      if (r != LIBUSB_SUCCESS || actual != send_len)
        {
          fprintf(stderr, "Write error in '%s': '%s', wrote %i of %i bytes.\n", funcname, libusb_error_name(r), actual, send_len);
          return r;
        }
    }

  if (reply_len > 0)
    {
      r = libusb_bulk_transfer (dev_handle, 0x81 | LIBUSB_ENDPOINT_IN, in_buf, reply_len, &actual, receive_timeout);

#ifdef PRINT_DEBUG_MSG
      if ( r == LIBUSB_SUCCESS)
        {
          printf ("%s recv %2i/%2i bytes: ", funcname, actual, reply_len);
          print_buffer (in_buf, actual);
        }
#endif

      if (r != LIBUSB_SUCCESS || actual != reply_len)
        {
          if (r == LIBUSB_ERROR_OVERFLOW)
            {
              fprintf(stderr, "LIBUSB_ERROR_OVERFLOW in '%s': expected %i bytes but got more.\n", funcname, reply_len);
              // Attention! You can't rely that data was written in in_buf
              // See: http://libusb.sourceforge.net/api-1.0/packetoverflow.html
              return r;
            }
          else
            fprintf(stderr, "Read error in '%s': '%s', tried to read %i, got %i bytes.\n", funcname, libusb_error_name(r), reply_len, actual);
        }

      // check reply
      if (!r
          && send_len > 0
          && (in_buf[0] != out_buf[0]
              ||  in_buf[1] != actual))
        {
          fprintf(stderr, "Error: Malformed reply. Check physical connection and EMI.\n");
          return LIBALLURIS_MALFORMED_REPLY;
        }
    }
  return r;
}

/****************************************************************************************/

/*!
 * \brief List accessible alluris devices
 *
 * The product field in alluris_device_description is filled via the USB descriptor.
 * After this the device is opened and the serial_number is read from the device.
 * Thus this function only lists devices where the application has sufficient rights to open
 * and read from the device. Check permissions if a device isn't returned.
 *
 * The retrieved list has to be freed with \ref free_alluris_device_list before the application exits.
 * \param[out] alluris_devs pointer to storage for the device list
 * \param[in] length number of elements in alluris_devs
 * \param[in] read_serial Try to read the serial from devices
 * \return 0 if successful else error code
 * \sa liballuris_free_device_list
 */
int liballuris_get_device_list (libusb_context* ctx, struct alluris_device_description* alluris_devs, size_t length, char read_serial)
{
  size_t k = 0;
  for (k=0; k<length; ++k)
    alluris_devs[k].dev = NULL;

  size_t num_alluris_devices = 0;
  libusb_device **devs;
  ssize_t cnt = libusb_get_device_list (ctx, &devs);
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

                      if (read_serial)
                        // get serial number from device
                        liballuris_get_serial_number (h, alluris_devs[num_alluris_devices].serial_number, sizeof (alluris_devs[0].serial_number));

                      num_alluris_devices++;
                      libusb_release_interface (h, 0);
                      libusb_close (h);
                    }
                  else if (LIBUSB_ERROR_BUSY) //it's already in use
                    {
                      //FIXME: what should we do?
                    }
                  else
                    fprintf (stderr, "liballuris_get_device_list: Couldn't claim interface: %s\n", libusb_error_name(r));

                }
              else
                fprintf (stderr, "liballuris_get_device_list: Couldn't open device: %s\n", libusb_error_name(r));
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

/*!
 * \brief free list filled from get_alluris_device_list
 */
void liballuris_free_device_list (struct alluris_device_description* alluris_devs, size_t length)
{
  size_t k = 0;
  for (k=0; k < length; ++k)
    if (alluris_devs[k].dev)
      {
        libusb_unref_device (alluris_devs[k].dev);
        alluris_devs[k].dev = NULL;
      }
}

/*!
 * \brief Open device with specified serial_number or the first available if NULL
 * \param[in] serial_number of device or NULL
 * \param[out] h storage for handle to communicate with the device
 * \return 0 if successful else error code
 */
int liballuris_open_device (libusb_context* ctx, const char* serial_number, libusb_device_handle** h)
{
  int k;
  libusb_device *dev = NULL;
  struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
  int cnt = liballuris_get_device_list (ctx, alluris_devs, MAX_NUM_DEVICES, (serial_number != NULL));

  if (cnt >= 1)
    {
      if (serial_number == NULL)
        dev = alluris_devs[0].dev;
      else
        for (k=0; k < cnt; k++)
          if (! strncmp (serial_number, alluris_devs[k].serial_number, sizeof (alluris_devs[0].serial_number)))
            dev = alluris_devs[k].dev;
    }

  if (! dev)
    //no device found
    return LIBUSB_ERROR_NOT_FOUND;

  int ret = libusb_open (dev, h);
  liballuris_free_device_list (alluris_devs, MAX_NUM_DEVICES);
  return ret;
}


/*!
 * \brief Open device with specified bus and device id.
 * \param[in] bus id of device
 * \param[in] device id of device
 * \param[out] h storage for handle to communicate with the device
 * \return 0 if successful else error code
 */
int liballuris_open_device_with_id (libusb_context* ctx, int bus, int device, libusb_device_handle** h)
{
  int k;
  libusb_device *dev = NULL;
  struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
  int cnt = liballuris_get_device_list (ctx, alluris_devs, MAX_NUM_DEVICES, 0);

  if (cnt >= 1)
    {
      for (k=0; k < cnt; k++)
        if (   libusb_get_bus_number (alluris_devs[k].dev) == bus
               && libusb_get_device_address (alluris_devs[k].dev) == device)
          dev = alluris_devs[k].dev;
    }

  if (! dev)
    //no device found
    return LIBUSB_ERROR_NOT_FOUND;

  int ret = libusb_open (dev, h);
  liballuris_free_device_list (alluris_devs, MAX_NUM_DEVICES);
  return ret;
}

void liballuris_clear_RX (libusb_device_handle* dev_handle, unsigned int timeout)
{
  unsigned char data[64];
  int actual;
  int r = libusb_bulk_transfer (dev_handle, 0x81 | LIBUSB_ENDPOINT_IN, data, 64, &actual, timeout);
#ifdef PRINT_DEBUG_MSG
  printf ("clear_RX: libusb_bulk_transfer returned '%s', actual = %i\n", libusb_error_name(r), actual);
#endif
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
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[out] buf output location for the serial number. Only populated when the return code is 0.
 * \param[in] length length of buffer in bytes
 * \return 0 if successful else error code. LIBALLURIS_DEVICE_BUSY if measurement is running
 */
int liballuris_get_serial_number (libusb_device_handle *dev_handle, char* buf, size_t length)
{
  out_buf[0] = 0x08;
  out_buf[1] = 3;
  out_buf[2] = 6;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    {
      short tmp = char_to_uint16 (in_buf + 3);
      if (tmp == -1)
        return LIBALLURIS_DEVICE_BUSY;
      else
        snprintf (buf, length, "%c.%i", in_buf[5] + 'A', tmp);
    }
  return ret;
}

int liballuris_get_firmware (libusb_device_handle *dev_handle, int dev, char* buf, size_t length)
{
  if (dev < 0 || dev > 1)
    return LIBALLURIS_OUT_OF_RANGE;

  out_buf[0] = 0x08;
  out_buf[1] = 3;
  out_buf[2] = dev;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    snprintf (buf, length, "V%i.%02i.%03i", in_buf[5], in_buf[4], in_buf[3]);
  return ret;
}

/*!
 * \brief Query the number of digits for the interpretation of the raw fixed-point numbers
 *
 * All liballuris_get_functions returns fixed-point numbers or strings. This function queries the number
 * of digits after the radix point. For example if liballuris_get_value(..) returns 123 and digits returns 1
 * the real value is 12.3
 *
 * Query this value is only possible if the measurement is not running,
 * else LIBALLURIS_DEVICE_BUSY is returned.
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[out] v output location for the returned number of digits. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa liballuris_get_value (libusb_device_handle *dev_handle, int* v)
 */
int liballuris_get_digits (libusb_device_handle *dev_handle, int* v)
{
  out_buf[0] = 0x08;
  out_buf[1] = 3;
  out_buf[2] = 3;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    {
      *v = char_to_int24 (in_buf + 3);
      if (*v == -1)
        return LIBALLURIS_DEVICE_BUSY;
    }
  return ret;
}

int liballuris_get_F_max (libusb_device_handle *dev_handle, int* fmax)
{
  out_buf[0] = 0x08;
  out_buf[1] = 3;
  out_buf[2] = 2;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    {
      *fmax = char_to_int24 (in_buf + 3);
      if (*fmax == -1)
        return LIBALLURIS_DEVICE_BUSY;
    }
  return ret;
}

/*!
 * \brief Query the current measurement value
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[out] value output location for the measurement value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int liballuris_get_value (libusb_device_handle *dev_handle, int* value)
{
  out_buf[0] = 0x46;
  out_buf[1] = 3;
  out_buf[2] = 3;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *value = char_to_int24 (in_buf + 3);
  return ret;
}

/*!
 * \brief Query positive peak value
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[out] peak output location for the peak value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int liballuris_get_pos_peak (libusb_device_handle *dev_handle, int* peak)
{
  out_buf[0] = 0x46;
  out_buf[1] = 3;
  out_buf[2] = 4;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *peak = char_to_int24 (in_buf + 3);
  return ret;
}

/*!
 * \brief Query negative peak value
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[out] peak output location for the peak value. Only populated when the return code is 0.
 * \return 0 if successful else error code
 * \sa digits (libusb_device_handle *dev_handle, int* v)
 */
int liballuris_get_neg_peak (libusb_device_handle *dev_handle, int* peak)
{
  out_buf[0] = 0x46;
  out_buf[1] = 3;
  out_buf[2] = 5;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *peak = char_to_int24 (in_buf + 3);
  return ret;
}

int liballuris_read_state (libusb_device_handle *dev_handle, struct liballuris_state* state, unsigned int timeout)
{
  out_buf[0] = 0x46;
  out_buf[1] = 3;
  out_buf[2] = 2;

#ifdef PRINT_DEBUG_MSG
  printf ("liballuris_read_state timeout=%i\n", timeout);
#endif

  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 6, timeout);
  if (ret == LIBALLURIS_SUCCESS)
    {
      union __liballuris_state__ tmp;
      tmp._int = char_to_int24 (in_buf + 3);
      if (tmp._int == -1)
        return LIBALLURIS_DEVICE_BUSY;
      else
        *state = tmp.bits;
    }
  return ret;
}

void liballuris_print_state (libusb_device_handle *dev_handle, struct liballuris_state state)
{
  printf ("[%c] upper limit exceeded\n",               (state.upper_limit_exceeded)? 'X': ' ');
  printf ("[%c] lower limit underrun\n",               (state.lower_limit_underrun)? 'X': ' ');
  printf ("[%c] peak  mode active\n",                  (state.some_peak_mode_active)? 'X': ' ');
  printf ("[%c] peak+ mode active\n",                  (state.peak_plus_active)? 'X': ' ');
  printf ("[%c] peak- mode active\n",                  (state.peak_minus_active)? 'X': ' ');
  printf ("[%c] Store to memory in progress\n",        (state.mem_running)? 'X': ' ');
  printf ("[%c] overload (abs(F) > 150%%)\n",          (state.overload)? 'X': ' ');
  printf ("[%c] fracture detected (only W20/W40)\n",   (state.fracture)? 'X': ' ');
  printf ("[%c] mem active (P21=1 or P21=2)\n",        (state.mem_active)? 'X': ' ');
  printf ("[%c] mem-conti (store with displayrate)\n", (state.mem_conti)? 'X': ' ');
  printf ("[%c] grenz_option\n",                       (state.grenz_option)? 'X': ' ');
  printf ("[%c] measurement running\n",                (state.measuring)? 'X': ' ');
}

/*!
 * \brief Enable or disable cyclic measurements
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \param[in] enable
 * \param[in] packetlen 1..19
 * \return 0 if successful else error code
 */
int liballuris_cyclic_measurement (libusb_device_handle *dev_handle, char enable, size_t length)
{
  if (length < 0 || length > 19)
    {
      fprintf (stderr, "Error: packetlen %i out of range 1..19.\n", length);
      return LIBALLURIS_OUT_OF_RANGE;
    }

  out_buf[0] = 0x01;
  out_buf[1] = 4;
  out_buf[2] = (enable)? 2:0;
  out_buf[3] = length;

  //printf ("liballuris_cyclic_measurement enable=%i\n", enable);
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 4, DEFAULT_SEND_TIMEOUT, 4, DEFAULT_RECEIVE_TIMEOUT);
  return ret;
}

/*
 * Increased receive timeout:
 * The sampling frequency can be selected between 10Hz and 990Hz
 * Therefore the maximum delay until the measurement completes is 1/10Hz = 100ms.
 * *2 = 200ms
 */

int liballuris_poll_measurement (libusb_device_handle *dev_handle, int* buf, size_t length)
{
  size_t len = 5 + length * 3;
  // FIXME: set timeout dynamically from len and sampling rate 10Hz/900Hz
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 0, 0, len, 2400);
  size_t k;
  for (k=0; k<length; k++)
    buf[k] = char_to_int24 (in_buf + 5 + k*3);

  return ret;
}

/*!
 * \brief tare measurement
 * FIXME: Genaue Funktion untersuchen
 */
int liballuris_tare (libusb_device_handle *dev_handle)
{
  out_buf[0] = 0x15;
  out_buf[1] = 3;
  out_buf[2] = 0;
  return liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
}

int liballuris_clear_pos_peak (libusb_device_handle *dev_handle)
{
  out_buf[0] = 0x15;
  out_buf[1] = 3;
  out_buf[2] = 1;
  return liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
}

int liballuris_clear_neg_peak (libusb_device_handle *dev_handle)
{
  out_buf[0] = 0x15;
  out_buf[1] = 3;
  out_buf[2] = 2;
  return liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
}

/*!
 * \brief Start measurement
 *
 * You may have to wait up to 500ms until you can read stable values with
 * liballuris_get_value().
 *
 * \param[in] dev_handle a handle for the device to communicate with
 * \return 0 if successful else error code
 * \sa liballuris_stop_measurement
 */
int liballuris_start_measurement (libusb_device_handle *dev_handle)
{
  out_buf[0] = 0x1C;
  out_buf[1] = 3;
  out_buf[2] = 1; //start
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);


  if (ret == LIBALLURIS_SUCCESS)
    {
      // The device may take up to 800ms until the measurment is running.
      // (for example if a automatic tare is parametrized at start of measurement)
      // -> wait for it
      int timeout = 30; // 30 * 50ms
      struct liballuris_state state;

      do
        {
          timeout--;
          ret = liballuris_read_state (dev_handle, &state, 600);
          if (! state.measuring)
            usleep (50000);
        }
      while (!ret && timeout && !state.measuring);

#ifdef PRINT_DEBUG_MSG
      printf ("liballuris_start_measurement timeout left = %i\n", timeout);
#endif

      if (! timeout)
        ret = LIBALLURIS_TIMEOUT;
    }

  return ret;
}

int liballuris_stop_measurement (libusb_device_handle *dev_handle)
{
  out_buf[0] = 0x1C;
  out_buf[1] = 3;
  out_buf[2] = 0; //stop
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);

  if (ret == LIBUSB_SUCCESS)
    {
      // the device may take approximately 100ms (1/10Hz) until the measurment is stopped.
      int timeout = 20; // 20 * 20ms
      struct liballuris_state state;
      do
        {
          timeout--;
          ret = liballuris_read_state (dev_handle, &state, 500);
          if (state.measuring)
            usleep (20000);
        }
      while (!ret && timeout && state.measuring);

#ifdef PRINT_DEBUG_MSG
      printf ("liballuris_stop_measurement timeout left = %i\n", timeout);
#endif

      if (! timeout)
        ret = LIBALLURIS_TIMEOUT;
    }

  return ret;
}

int liballuris_set_upper_limit (libusb_device_handle *dev_handle, int limit)
{
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state, DEFAULT_RECEIVE_TIMEOUT);
  if (ret)
    return ret;

  if (state.measuring)
    return LIBALLURIS_DEVICE_BUSY;

  out_buf[0] = 0x18;
  out_buf[1] = 6;
  out_buf[2] = 0; //maximum
  memcpy (out_buf+3, (unsigned char *) &limit, 3);

  // Increased receive timeout due to EEPROM write operation
  return liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 6, DEFAULT_SEND_TIMEOUT, 6, 500);
}

int liballuris_set_lower_limit (libusb_device_handle *dev_handle, int limit)
{
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state, DEFAULT_RECEIVE_TIMEOUT);
  if (ret)
    return ret;

  if (state.measuring)
    return LIBALLURIS_DEVICE_BUSY;

  out_buf[0] = 0x18;
  out_buf[1] = 6;
  out_buf[2] = 1; //minimum
  memcpy (out_buf+3, (unsigned char *) &limit, 3);

  // Increased receive timeout due to EEPROM write operation
  return liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 6, DEFAULT_SEND_TIMEOUT, 6, 500);
}

int liballuris_get_upper_limit (libusb_device_handle *dev_handle, int* limit)
{
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state, DEFAULT_RECEIVE_TIMEOUT);
  if (ret)
    return ret;

  if (state.measuring)
    return LIBALLURIS_DEVICE_BUSY;

  out_buf[0] = 0x19;
  out_buf[1] = 6;
  out_buf[2] = 0; //maximum
  ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 6, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *limit = char_to_int24 (in_buf + 3);
  return ret;
}

int liballuris_get_lower_limit (libusb_device_handle *dev_handle, int* limit)
{
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state, DEFAULT_RECEIVE_TIMEOUT);
  if (ret)
    return ret;

  if (state.measuring)
    return LIBALLURIS_DEVICE_BUSY;

  out_buf[0] = 0x19;
  out_buf[1] = 6;
  out_buf[2] = 1; //minimum
  ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 6, DEFAULT_SEND_TIMEOUT, 6, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *limit = char_to_int24 (in_buf + 3);
  return ret;
}

int liballuris_set_mode (libusb_device_handle *dev_handle, enum liballuris_measurement_mode mode)
{
  if (mode < 0 || mode > 3)
    {
      fprintf (stderr, "Error: mode %i out of range 0..3\n", mode);
      return LIBALLURIS_OUT_OF_RANGE;
    }
  out_buf[0] = 0x04;
  out_buf[1] = 3;
  out_buf[2] = mode;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);

  if (in_buf[2] != mode)
    return LIBALLURIS_DEVICE_BUSY;

  return ret;
}

int liballuris_get_mode (libusb_device_handle *dev_handle, enum liballuris_measurement_mode *mode)
{
  out_buf[0] = 0x05;
  out_buf[1] = 2;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 2, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *mode = in_buf[2];
  return ret;
}

int liballuris_set_mem_mode (libusb_device_handle *dev_handle, enum liballuris_memory_mode mode)
{
  if (mode < 0 || mode > 2)
    {
      fprintf (stderr, "Error: memory mode %i out of range 0..2\n", mode);
      return LIBALLURIS_OUT_OF_RANGE;
    }
  out_buf[0] = 0x1D;
  out_buf[1] = 3;
  out_buf[2] = mode;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, 600);

  if (in_buf[2] != mode)
    return LIBALLURIS_DEVICE_BUSY;

  return ret;
}

int liballuris_get_mem_mode (libusb_device_handle *dev_handle, enum liballuris_memory_mode *mode)
{
  out_buf[0] = 0x1E;
  out_buf[1] = 2;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 2, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *mode = in_buf[2];

  // workaround for a firmware bug in versions < FIXME: add version number!
  // check if we get a second reply
  int actual;
  int temp_ret = libusb_bulk_transfer (dev_handle, 0x81 | LIBUSB_ENDPOINT_IN, in_buf, 3, &actual, 20);
  if (temp_ret == LIBALLURIS_SUCCESS)
    {
      // discard first reply
      *mode = in_buf[2];
    }
  else
    // bug is fixed and we got a timeout (no superfluous reply)
    ;

  return ret;
}

int liballuris_set_unit (libusb_device_handle *dev_handle, enum liballuris_unit unit)
{
  if (unit < 0 || unit > 5)
    {
      fprintf (stderr, "Error: unit %i out of range 0..5\n", unit);
      return LIBALLURIS_OUT_OF_RANGE;
    }

  // F_max dependent mapping
  int fmax;
  int ret = liballuris_get_F_max (dev_handle, &fmax);
  if (ret)
    return ret;

  out_buf[0] = 0x1A;
  out_buf[1] = 3;

  if (fmax <= 10) //mapping from chapter 3.14.3
    {
      if (unit == 2 || unit == 4) //kg and lb not available on 5N and 10N devices
        return LIBALLURIS_OUT_OF_RANGE;
      if (unit == 3 || unit == 5)
        unit--;
    }
  else if (unit == 1 || unit == 3 || unit == 5) //g and oz not available on devices with fmax > 10N
    return LIBALLURIS_OUT_OF_RANGE;

  out_buf[2] = unit;
  ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, 500);

  if (in_buf[2] != unit)
    return LIBALLURIS_DEVICE_BUSY;

  return ret;
}

int liballuris_get_unit (libusb_device_handle *dev_handle, enum liballuris_unit *unit)
{
  // F_max dependent mapping
  int fmax;
  int ret = liballuris_get_F_max (dev_handle, &fmax);
  if (ret)
    return ret;

  out_buf[0] = 0x1B;
  out_buf[1] = 2;
  ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 2, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *unit = in_buf[2];

  // mapping from chapter 3.15.3
  if (fmax <= 10 && (*unit == 2 || *unit == 4))
    *unit = *unit + 1;

  return ret;
}

int liballuris_set_digout (libusb_device_handle *dev_handle, int v)
{
  if (v < 0 || v > 7) //only 3 bits
    return LIBALLURIS_OUT_OF_RANGE;

  out_buf[0] = 0x21;
  out_buf[1] = 3;
  out_buf[2] = v;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 3, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);

  if (in_buf[2] != v)
    return LIBALLURIS_DEVICE_BUSY;

  return ret;
}

int liballuris_get_digout (libusb_device_handle *dev_handle, int *v)
{
  out_buf[0] = 0x22;
  out_buf[1] = 2;
  int ret = liballuris_device_bulk_transfer (dev_handle, __FUNCTION__, 2, DEFAULT_SEND_TIMEOUT, 3, DEFAULT_RECEIVE_TIMEOUT);
  if (ret == LIBALLURIS_SUCCESS)
    *v = in_buf[2];
  return ret;
}
