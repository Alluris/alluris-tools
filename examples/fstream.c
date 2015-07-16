/*

Copyright (C) 2015 Alluris GmbH & Co. KG <weber@alluris.de>

fstream -- f(ast)stream(ing)

Capture values in peak mode with 900Hz and output it as binary int32

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
#include <sys/poll.h>
#include "liballuris.h"

/* GNU Radio Companion
 * Use for example "nc -q0 host 9000 -e ./fstream"
 */

int main()
{
  libusb_context* ctx;
  libusb_device_handle* h;

  int r;
  r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Couldn't init libusb %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  r = liballuris_open_device (ctx, NULL, &h);
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

  struct pollfd fds;
  int tret;
  fds.fd = 0; /* this is STDIN */
  fds.events = POLLIN;

  char reply; //send "c" to abort capturing
  int block_size = 19;
  int tempx[block_size];

  // FIXME: check if measurement is running before
  // enabling data stream. Abort if device is idle

  // debug_en_lvl3 (h);
  // enable streaming
  liballuris_cyclic_measurement (h, 1, block_size);
  int k;

  do
    {
      int r = liballuris_poll_measurement (h, tempx, block_size);
      if (r == LIBUSB_SUCCESS)
        fwrite (tempx, 4, block_size, stdout);
      //~ for (k=0; k < block_size; ++k)
      //~ printf ("%i\n", tempx[k]);
      fflush (stdout);

      tret = poll (&fds, 1, 0);
      reply = 0;
      if (tret == 1)
        reply = getc(stdin);

    }
  while (reply != 'c');

  // disable streaming
  liballuris_cyclic_measurement (h, 0, block_size);

  // empty read remaining data
  liballuris_clear_RX (h);

  libusb_release_interface (h, 0);
  libusb_close (h);
  return EXIT_SUCCESS;
}
