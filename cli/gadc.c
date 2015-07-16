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
#include <argp.h>
#include <liballuris.h>

const char *argp_program_version =
  "gadc 0.1";

const char *argp_program_bug_address =
  "<software@alluris.de>";

static char doc[] =
  "Generic Alluris device control";

/* A description of the arguments we accept. */
static char args_doc[] = "--list | [--device=SERIAL] CMD1 CMD2";

/* The options we understand. */
static struct argp_option options[] =
{
  {"device",       'd', "SERIAL",   0, "Connect to specific alluris device", 0},
  {"list",         'l', 0,          0, "List accessible devices", 0},
  {0, 0, 0, 0, "Measurement:", 1 },
  {"value",        'v', 0,          0, "Value", 1 },
  {"pos-peak",     'p', 0,          0, "Positive peak", 1 },
  {"neg-peak",     'n', 0,          0, "Negative peak", 1 },
  {"sample",       's', "NUM",      0, "Capture NUM values", 1 },
  {0, 0, 0, 0, "Tare:", 2 },
  {"tare",         't', 0,          0, "Tare measurement", 2 },
  {"clear-pos",    1000, 0,         0, "Clear positive peak", 2},
  {"clear-neg",    1001, 0,         0, "Clear negative peak", 2},
  {0, 0, 0, 0, "Settings:", 3 },
  {"set-pos-limit", 1002, "P3",     0, "Param P3, positive limit", 3},
  {"set-neg-limit", 1003, "P4",     0, "Param P4, negative limit", 3},
  {"get-pos-limit", 1004, 0,        0, "Param P3, positive limit", 3},
  {"get-neg-limit", 1005, 0,        0, "Param P4, negative limit", 3},
  { 0,0,0,0,0,0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *device;
  libusb_context* ctx;
  libusb_device_handle* h;
};

// FIXME: Im Falle eines Fehlers, sollte libusb geschlossen und das Programm beendet werden
static void print_value (int ret, int value)
{
  if (ret != LIBUSB_SUCCESS)
    fprintf(stderr, "Error: '%s'\n", libusb_error_name (ret));
  else
    printf ("%i\n", value);
}

static void print_multiple (libusb_device_handle *dev_handle, int num)
{
  int block_size = num;
  if (block_size > 19)
    block_size = 19;

  int tempx[block_size];

  // FIXME: check if measurement is running before
  // enabling data stream. Abort if device is idle

  // enable streaming
  liballuris_cyclic_measurement (dev_handle, 1, block_size);

  while (num > 0)
    {
      //printf ("polling %i, %i left\n", block_size, num);
      int r = liballuris_poll_measurement (dev_handle, tempx, block_size);
      if (r == LIBUSB_SUCCESS)
        {
          int j = (num < block_size)? num : block_size;
          int k;
          for (k=0; k < j; ++k)
            printf ("%i\n", tempx[k]);
        }
      num = num - block_size;
    }

  // disable streaming
  liballuris_cyclic_measurement (dev_handle, 0, block_size);

  // empty read remaining data
  liballuris_clear_RX (dev_handle);
}

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  char *endptr = NULL;
  int r;
  int value, num_samples;

  if (key == 'l')
    {
      // list accessible devices and exit
      // FIXME: document that a running measurement prohibits reading the serial_number

#define MAX_NUM_DEVICES 4
      struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
      ssize_t cnt = liballuris_get_device_list (arguments->ctx, alluris_devs, MAX_NUM_DEVICES, 1);

      int k;
      printf ("Device list:\n");
      for (k=0; k < cnt; k++)
        printf ("  # %2i: %s %s\n", k+1, alluris_devs[k].product, alluris_devs[k].serial_number);

      if (!cnt)
        printf ("No accessible device found");

      // free device list
      liballuris_free_device_list (alluris_devs, MAX_NUM_DEVICES);

      exit (0);
    }

  if (key == 'd')
    {
      arguments->device = arg;
      return 0;
    }

  if (arguments->h)
    switch (key)
      {
      case 'v':
        r = liballuris_raw_value (arguments->h, &value);
        print_value (r, value);
        break;
      case 'p':
        r = liballuris_raw_pos_peak (arguments->h, &value);
        print_value (r, value);
        break;
      case 'n':
        r = liballuris_raw_neg_peak (arguments->h, &value);
        print_value (r, value);
        break;
      case 's':
        num_samples = strtol (arg, &endptr, 10);
        print_multiple (arguments->h, num_samples);
        break;
      case 't':
        r = liballuris_tare (arguments->h);
        break;
      case 1000:
        liballuris_clear_pos_peak (arguments->h);
        break;
      case 1001:
        liballuris_clear_neg_peak (arguments->h);
        break;
      case 1002:  //set-pos-limit
        value = strtol (arg, &endptr, 10);
        break;
      case 1003:  //set-neg-limit
        value = strtol (arg, &endptr, 10);
        break;
      case 1004:
        break;
      case 1005:
        break;
      default:
        return ARGP_ERR_UNKNOWN;
      }

  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char** argv)
{
  struct arguments arguments;

  arguments.device       = NULL;
  arguments.ctx          = NULL;
  arguments.h            = NULL;

  int r;

  r = libusb_init (&arguments.ctx);
  if (r < 0)
    {
      fprintf (stderr, "Couldn't init libusb %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  /* First parse to get optional device serial number or list devices*/
  argp_parse (&argp, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &arguments);

  //printf ("Device    = %s\n", arguments.device);

  // open device
  r = liballuris_open_device (arguments.ctx, arguments.device, &arguments.h);
  if (r)
    {
      fprintf (stderr, "Couldn't open device '%s': %s\n", arguments.device, libusb_error_name (r));
      return EXIT_FAILURE;
    }

  r = libusb_claim_interface (arguments.h, 0);
  if (r)
    {
      fprintf (stderr, "Couldn't claim interface: %s\n", libusb_error_name (r));
      return EXIT_FAILURE;
    }

  // Second parse, now execute the commands
  argp_parse (&argp, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &arguments);

  /************************************************************************************/

  /*
  int v;
  r = digits (arguments.h, &v);
  printf ("digits = %i\n", v);

  r = raw_pos_peak (arguments.h, &v);
  printf ("raw pos peak = %i\n", v);

  r = raw_neg_peak (arguments.h, &v);
  printf ("raw neg peak = %i\n", v);
  */

  //printf ("libusb_release_interface\n");
  libusb_release_interface (arguments.h, 0);
  //printf ("libusb_close\n");
  libusb_close (arguments.h);
  return EXIT_SUCCESS;
}
