/*

Copyright (C) 2015 Alluris GmbH & Co. KG <weber@alluris.de>

Generic Alluris device control (gadc)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  See ../COPYING
If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include "liballuris.h"

//int main(int argc, char* argv[])
int main()
{
  // Discover up to 4 Alluris devices
  struct alluris_device_description alluris_devs[4];
  int k;

  libusb_context *ctx = NULL;

  int r;
  ssize_t cnt;
  r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Couldn't init libusb %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  cnt = get_alluris_device_list (alluris_devs, 4);

  // list all found devices
  for (k=0; k<cnt; k++)
    printf ("Device %i: %s %s\n", k+1, alluris_devs[k].product, alluris_devs[k].serial_number);
  // free device list
  free_alluris_device_list (alluris_devs, 4);

  // open first device
  libusb_device_handle* h;
  r = open_alluris_device (NULL, &h);
  if (r)
    {
      fprintf (stderr, "Couldn't open device: %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  r = libusb_claim_interface (h, 0);
  if (r)
    {
      fprintf (stderr, "Couldn't claim interface: %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  /************************************************************************************/
  int v;
  r = digits (h, &v);
  printf ("digits = %i\n", v);
  r = raw_value (h, &v);
  printf ("raw value = %i\n", v);

  r = raw_pos_peak (h, &v);
  printf ("raw pos peak = %i\n", v);

  r = raw_neg_peak (h, &v);
  printf ("raw neg peak = %i\n", v);

  libusb_release_interface (h, 0);
  libusb_close (h);
  return EXIT_SUCCESS;
}
