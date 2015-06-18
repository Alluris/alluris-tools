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
along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
    return r;
  cnt = get_alluris_device_list (alluris_devs, 4);

  // list all found devices
  for (k=0; k<cnt; k++)
    printf ("Device %i: %s %s\n", k+1, alluris_devs[k].product, alluris_devs[k].serial_number);
  // TODO: hier noch unref machen, oder in den struct nur ne Beschreibung, kein dev

  // open first device
  libusb_device* dev = get_alluris_device (NULL);
  libusb_device_handle* h;
  r = libusb_open (dev, &h);
  r = libusb_claim_interface (h, 0);

  if (r == LIBUSB_SUCCESS)
    {
      int v;
      r = raw_value (h, &v);
      printf ("value = %i\n", v);
    }
  else
    fprintf (stderr, "Couldn't open device: %s\n", libusb_error_name(r));

  libusb_close (h);
  return EXIT_SUCCESS;
}
