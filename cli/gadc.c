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
#include <signal.h>
#include <liballuris.h>

char do_exit = 0;

const char *argp_program_version =
  "gadc 0.2.1 using " PACKAGE_NAME " " PACKAGE_VERSION;

const char *argp_program_bug_address =
  "<software@alluris.de>";

static char doc[] =
  "Generic Alluris device control";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] =
{
  {0, 0, 0, 0, "Device discovery and connection:", 1},
  {"list",         'l', 0,             0, "List accessible (stopped and not claimed) devices"},
  {"serial",       1009, "SERIAL",     0, "Connect to specific alluris device using serial number. This only works if the device is stopped."},
  {NULL,           'b',  "Bus,Device", 0, "Connect to specific alluris device using bus and device id"},

  {0, 0, 0, 0, "Measurement:", 2 },
  {"start",        1006, 0,            0, "Start"},
  {"stop",         1007, 0,            0, "Stop"},
  {"value",        'v', 0,             0, "Value"},
  {"pos-peak",     'p', 0,             0, "Positive peak"},
  {"neg-peak",     'n', 0,             0, "Negative peak"},
  {"sample",       's', "NUM",         0, "Capture NUM values (Inf if NUM==0)"},

  {0, 0, 0, 0, "Tare:", 3 },
  {"tare",         't', 0,             0, "Tare measurement"},
  {"clear-pos",    1000, 0,            0, "Clear positive peak"},
  {"clear-neg",    1001, 0,            0, "Clear negative peak"},

  {0, 0, 0, 0, "Settings:", 4 },
  {"set-upper-limit", 1002, "P3",      0, "Param P3, upper limit"},
  {"set-lower-limit", 1003, "P4",      0, "Param P4, lower limit"},
  {"get-upper-limit", 1004, 0,         0, "Param P3, upper limit"},
  {"get-lower-limit", 1005, 0,         0, "Param P4, lower limit"},
  {"set-mode",      1012, "MODE",      0, "Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-"},
  {"get-mode",      1013, 0,           0, "Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-"},
  {"set-mem-mode",  1014, "MODE",      0, "Memory mode 0=disabled, 1=single, 2=continuous"},
  {"get-mem-mode",  1015, 0,           0, "Memory mode 0=disabled, 1=single, 2=continuous"},
  {"get-unit",      1016, 0,           0, "Unit"},
  {"set-unit",      1017, "U",         0, "Unit 'N', 'cN', 'kg', 'g', 'lb', 'oz'"},
  {"factory-defaults",1022, 0,         0, "Restore factory defaults"},
  {"set-auto-stop", 1032, "S",         0, "Set auto-stop 0..30"},
  {"get-auto-stop", 1033, 0,           0, "Get auto-stop"},

  {0, 0, 0, 0, "Get fixed attributes:", 5 },
  {"digits",       1008, 0,            0, "Digits of used fixed-point numbers"},
  {"resolution",   1030, 0,            0, "Resolution (1, 2 or 5)"},
  {"fmax",         1018, 0,            0, "Fmax"},

  {0, 0, 0, 0, "Misc:", 6 },
  {"state",        1010, 0,            0, "Read RAM state"},
  {"sleep",        1011, "T",          0, "Sleep T milliseconds"},
  {"set-digout",   1019, "MASK",       0, "Set state of the 3 digital outputs = MASK (firmware >= V4.03.008/V5.03.008)"},
  {"get-digout",   1020, 0,            0, "Get state of the 3 digital outputs (firmware >= V4.03.008/V5.03.008)"},
  {"get-digin",    1034, 0,            0, "Get state of the digital input (firmware >= V4.04.007/V5.04.007)"},
  {"get-firmware", 1021, 0,            0, "Get firmware of communication and measurement processor"},
  {"power-off",    1023, 0,            0, "Power off the device"},
  {"delete-memory",1024, 0,            0, "Delete the measurement memory"},
  {"read-memory",  1025, "ADR",        0, "Read adr 0..999 or -1 for whole memory"},
  {
    "get-stats",    1026, 0,            0, "Get statistic (MAX_PLUS, MIN_PLUS, MAX_MINUS, MIN_MINUS, AVERAGE, VARIANCE) from memory values. "\
    "All values are fixed-point numbers (see --digits) except DEVIATION which uses 3 digits in firmware "\
    "< V5.04.007 and --digits in newer versions."
  },
  {"keypress",     1027, "KEY",        0, "Sim. keypress. Bit 0=S1, 1=S2, 2=S3, 3=long_press. For ex. 12 => long press of S3"},
  {"get-mem-count",1028, 0,            0, "Get number of values in memory"},
  {"get-next-cal-date",1029, 0,        0, "Get the next calibration date as YYMM"},
  {"set-keylock",  1031, "V",          0, "Lock (V=1) or unlock (V=0) keys. Power-off with S1 is still possible. Disconnecting USB automatically unlocks the keys. (firmware >= V4.04.005/V5.04.005)"},
  { 0,0,0,0,0,0 }

};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  libusb_context* ctx;
  libusb_device_handle* h;
  int error;
  int last_key;
};

void termination_handler (int signum)
{
  //fprintf(stderr, "Received signal %i, terminating program\n", signum);
  do_exit = 1;
}

static void print_value (int ret, int value)
{
  if (ret != LIBUSB_SUCCESS)
    fprintf(stderr, "Error: '%s'\n", liballuris_error_name (ret));
  else
    printf ("%i\n", value);
}

static int print_multiple (libusb_device_handle *dev_handle, int num)
{
  // check if measurement is running
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state);
  if (ret == LIBUSB_SUCCESS)
    {
      if (state.measuring)
        {
          int block_size = num;
          if (block_size > 19 || !num)
            block_size = 19;

          int tempx[block_size];

          // enable streaming
          ret = liballuris_cyclic_measurement (dev_handle, 1, block_size);

          int cnt = 0;
          // if num==0, read until sigint or sigterm
          while (!do_exit && !ret && (!num || num > cnt))
            {
              //printf ("polling %i, %i left\n", block_size, num);
              ret = liballuris_poll_measurement (dev_handle, tempx, block_size);
              if (ret == LIBUSB_SUCCESS)
                {
                  int j = (num < block_size)? num : block_size;
                  int k=0;
                  while (k < block_size && (cnt < num || !num))
                    {
                      printf ("%i\n", tempx[k++]);
                      cnt++;
                    }
                  fflush (stdout);
                }
              else
                return ret;
            }

          // disable streaming
          ret = liballuris_cyclic_measurement (dev_handle, 0, block_size);
        }
      else
        {
          fprintf (stderr, "Error: Measurement is not running. Please start it first..\n");
          ret = LIBALLURIS_DEVICE_BUSY;
        }
    }
  return ret;
}

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  //printf ("DEBUG: key=%i, key=0x%x, key=%c\n", key, key, key);

  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
  arguments->last_key = key;

  char *endptr = NULL;
  int r = 0;
  int value, num_samples;
  int stats[6];
  struct liballuris_state device_state;
  char firmware_buf[21];

  if (key == 'l')
    {
      // list accessible devices and exit
      // FIXME: document that a running measurement prohibits reading the serial_number

#define MAX_NUM_DEVICES 4
      struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
      ssize_t cnt = liballuris_get_device_list (arguments->ctx, alluris_devs, MAX_NUM_DEVICES, 1);

      int k;
      printf ("Device list:\n");
      printf ("Num Bus Device Product                   Serial\n");
      for (k=0; k < cnt; k++)
        printf ("%3i %03d    %03d %-25s %s\n", k+1,
                libusb_get_bus_number(alluris_devs[k].dev),
                libusb_get_device_address(alluris_devs[k].dev),
                alluris_devs[k].product,
                alluris_devs[k].serial_number);

      if (!cnt)
        fprintf (stderr, "No accessible device found\n");

      // free device list
      liballuris_free_device_list (alluris_devs, MAX_NUM_DEVICES);

      exit (0);
    }

  if (key == 1009)  // use specific serial number
    {
      if (!arguments->h)
        {
          r = liballuris_open_device (arguments->ctx, arg, &arguments->h);
          if (r)
            {
              fprintf (stderr, "Couldn't open device with serial='%s': %s\n", arg, liballuris_error_name (r));
              exit (EXIT_FAILURE);
            }
          state->next = state->argc;
        }
      return 0;
    }

  if (key == 'b')  // use specific bus and device id
    {
      // split "Bus,Device"
      int bus = strtol (arg, &endptr, 10);
      if (*endptr != ',')
        {
          fprintf (stderr, "Error: Wrong delimiter '%c'\n", *endptr);
          exit (EXIT_FAILURE);
        }
      int device = strtol (++endptr, &endptr, 10);

      if (!arguments->h)
        {
          r = liballuris_open_device_with_id (arguments->ctx, bus, device, &arguments->h);
          if (r)
            {
              fprintf (stderr, "Couldn't open device with bus=%i and device=%i: %s\n", bus, device, liballuris_error_name (r));
              exit (EXIT_FAILURE);
            }
          state->next = state->argc;
        }
      return 0;
    }

  enum liballuris_measurement_mode r_mode;
  enum liballuris_memory_mode r_mem_mode;
  enum liballuris_unit r_unit;
  if (arguments->h)
    switch (key)
      {
      case 'v':
        r = liballuris_get_value (arguments->h, &value);
        print_value (r, value);
        break;
      case 'p':
        r = liballuris_get_pos_peak (arguments->h, &value);
        print_value (r, value);
        break;
      case 'n':
        r = liballuris_get_neg_peak (arguments->h, &value);
        print_value (r, value);
        break;
      case 's':
        num_samples = strtol (arg, &endptr, 10);
        if (!num_samples || num_samples > 1)
          r = print_multiple (arguments->h, num_samples);
        else
          {
            fprintf (stderr, "NUM has to be > 1 or 0 (read until sigint or sigterm)\n");
            r = LIBALLURIS_OUT_OF_RANGE;
          }
        break;
      case 't':
        r = liballuris_tare (arguments->h);
        break;
      case 1000:
        r = liballuris_clear_pos_peak (arguments->h);
        break;
      case 1001:
        r = liballuris_clear_neg_peak (arguments->h);
        break;
      case 1002:
        value = strtol (arg, &endptr, 10);
        r = liballuris_set_upper_limit (arguments->h, value);
        break;
      case 1003:
        value = strtol (arg, &endptr, 10);
        r = liballuris_set_lower_limit (arguments->h, value);
        break;
      case 1004:
        r = liballuris_get_upper_limit (arguments->h, &value);
        print_value (r, value);
        break;
      case 1005:
        r = liballuris_get_lower_limit (arguments->h, &value);
        print_value (r, value);
        break;
      case 1006:
        r = liballuris_start_measurement (arguments->h);
        break;
      case 1007:
        r = liballuris_stop_measurement (arguments->h);
        break;
      case 1008:
        r = liballuris_get_digits (arguments->h, &value);
        print_value (r, value);
        break;
      case 1010:
        r = liballuris_read_state (arguments->h, &device_state);
        if (r == LIBUSB_SUCCESS)
          liballuris_print_state (device_state);
        break;
      case 1011:
        value = strtol (arg, &endptr, 10);
        //printf ("sleep %s %i\n", arg, value);
        usleep (value * 1000);
        break;
      case 1012:
        r_mode = strtol (arg, &endptr, 10);
        r = liballuris_set_mode (arguments->h, r_mode);
        break;
      case 1013:
        r = liballuris_get_mode (arguments->h, &r_mode);
        print_value (r, r_mode);
        break;
      case 1014:
        r_mem_mode = strtol (arg, &endptr, 10);
        r = liballuris_set_mem_mode (arguments->h, r_mem_mode);
        break;
      case 1015:
        r = liballuris_get_mem_mode (arguments->h, &r_mem_mode);
        print_value (r, r_mem_mode);
        break;
      case 1016:  //get-unit
        r = liballuris_get_unit (arguments->h, &r_unit);
        if (r != LIBUSB_SUCCESS)
          fprintf(stderr, "Error: '%s'\n", liballuris_error_name (r));
        else
          printf ("%s\n", liballuris_unit_enum2str (r_unit));
        break;
      case 1017:  //set-unit
        r_unit = liballuris_unit_str2enum (arg);
        r = liballuris_set_unit (arguments->h, r_unit);
        break;
      case 1018:  //get fmax
        r = liballuris_get_F_max (arguments->h, &value);
        print_value (r, value);
        break;
      case 1019:  //set digout
        value = strtol (arg, &endptr, 10);
        r = liballuris_set_digout (arguments->h, value);
        break;
      case 1020:  //get digout
        r = liballuris_get_digout (arguments->h, &value);
        print_value (r, value);
        break;
      case 1021:  //get firmware
        r = liballuris_get_firmware (arguments->h, 0, firmware_buf, 21);
        if (r != LIBUSB_SUCCESS)
          fprintf(stderr, "Error: '%s'\n", liballuris_error_name (r));
        else
          printf ("%s;", firmware_buf);

        r = liballuris_get_firmware (arguments->h, 1, firmware_buf, 21);
        if (r != LIBUSB_SUCCESS)
          fprintf(stderr, "Error: '%s'\n", liballuris_error_name (r));
        else
          printf ("%s\n", firmware_buf);
        break;
      case 1022:  //restore factory defaults
        r = liballuris_restore_factory_defaults (arguments->h);
        break;
      case 1023:  //power off device
        r = liballuris_power_off (arguments->h);
        break;
      case 1024:  //delete-memory
        r = liballuris_delete_memory (arguments->h);
        break;
      case 1025:  //read-memory
        value = strtol (arg, &endptr, 10);
        int star_adr = value;
        int stop_adr = value;
        if (value == -1)
          {
            star_adr = 0;
            stop_adr = 999;
          }
        while (star_adr <= stop_adr)
          {
            r = liballuris_read_memory (arguments->h, star_adr++, &value);
            print_value (r, value);
          }
        break;
      case 1026:  //get-stats
        r = liballuris_get_mem_statistics (arguments->h, stats, 6);
        if (r != LIBUSB_SUCCESS)
          fprintf(stderr, "Error: '%s'\n", liballuris_error_name (r));
        else
          {
            printf ("MAX_PLUS  (raw) = %5i\n", stats[0]);
            printf ("MIN_PLUS  (raw) = %5i\n", stats[1]);
            printf ("MAX_MINUS (raw) = %5i\n", stats[2]);
            printf ("MIN_MINUS (raw) = %5i\n", stats[3]);
            printf ("AVERAGE   (raw) = %5i\n", stats[4]);
            printf ("VARIANCE  (raw) = %5i\n", stats[5]);
          }
        break;
      case 1027:  //simulate keypress
        value = strtol (arg, &endptr, 10);
        r = liballuris_sim_keypress (arguments->h, value);
        break;
      case 1028: //get-mem-count
        r = liballuris_get_mem_count (arguments->h, &value);
        print_value (r, value);
        break;
      case 1029: //next-cal-date
        r = liballuris_get_next_calibration_date (arguments->h, &value);
        print_value (r, value);
        break;

      case 1030: //get_resolution
        r = liballuris_get_resolution (arguments->h, &value);
        print_value (r, value);
        break;

      case 1031: //set-keylock
        value = strtol (arg, &endptr, 10);
        r = liballuris_set_key_lock (arguments->h, value);
        break;

      case 1032: //set-auto-stop
        value = strtol (arg, &endptr, 10);
        r = liballuris_set_autostop (arguments->h, value);
        break;

      case 1033: //get-auto-stop
        r = liballuris_get_autostop (arguments->h, &value);
        print_value (r, value);
        break;

      case 1034: //get-digin
        r = liballuris_get_digin (arguments->h, &value);
        print_value (r, value);
        break;

      default:
        return ARGP_ERR_UNKNOWN;
      }

  if (r || do_exit)
    {
      if (do_exit)
        arguments->error = 0;
      else
        arguments->error = r;
      // stop parsing
      state->next = state->argc;
    }

  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char** argv)
{
  if (argc == 1)
    {
      fprintf (stderr, "ERROR: No commands.\n");
      fprintf (stderr, "Try `gadc --help' or `gadc --usage' for more information.\n");
      return EXIT_FAILURE;
    }

  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);

  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

  if (signal (SIGPIPE, termination_handler) == SIG_IGN)
    signal (SIGPIPE, SIG_IGN);

  struct arguments arguments;

  arguments.ctx          = NULL;
  arguments.h            = NULL;
  arguments.error        = 0;
  arguments.last_key     = 0;

  int r = libusb_init (&arguments.ctx);
  if (r < 0)
    {
      fprintf (stderr, "Couldn't init libusb %s\n", liballuris_error_name (r));
      return EXIT_FAILURE;
    }

  /* First parse to get optional device serial number or list devices*/
  argp_parse (&argp, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &arguments);

  // No devices opened so far so open the first available
  if (! arguments.h)
    {
      r = liballuris_open_device (arguments.ctx, NULL, &arguments.h);
      if (r)
        fprintf (stderr, "Couldn't open device: %s\n", liballuris_error_name (r));
    }

  if (arguments.h)
    {
      r = libusb_claim_interface (arguments.h, 0);
      if (r)
        fprintf (stderr, "Couldn't claim interface: %s\n", liballuris_error_name (r));
      else
        {
          // Second parse, now execute the commands
          argp_parse (&argp, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &arguments);

          if (arguments.error)
            {
              fprintf(stderr, "Error executing key = %i: '%s'\n", arguments.last_key, liballuris_error_name (arguments.error));
              r = arguments.error;

              // cleanup after error
              usleep (500000);
              liballuris_clear_RX (arguments.h, 1000);

              // disable streaming
              liballuris_cyclic_measurement (arguments.h, 0, 19);

              //empty read RX buffer
              fprintf(stderr, "Clearing RX buffer, ");
              liballuris_clear_RX (arguments.h, 1000);
              fprintf(stderr, "closing application...\n");
            }
          //printf ("libusb_release_interface\n");
          libusb_release_interface (arguments.h, 0);
        }
      //printf ("libusb_close\n");
      libusb_close (arguments.h);
    }
  return r;
}
