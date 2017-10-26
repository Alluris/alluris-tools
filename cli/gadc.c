/*

Copyright (C) 2016 Alluris GmbH & Co. KG <weber@alluris.de>

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
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <liballuris.h>

const char *program_name = "gadc";

const char *program_version = "0.2.4";

const char *program_bug_address = "<software@alluris.de>";

static char do_exit = 0;
static int verbose_flag;

void usage ()
{
  printf ("Usage: %s [OPTION]...\n", program_name);
  fputs ("\
Generic Alluris device control\n\
\n\
 Device discovery and connection:\n\
  -l, --list                 List accessible (stopped and not claimed) devices\n\
  -b Bus,Device              Connect to specific alluris device using bus and\n\
                             device id\n\
  -S, --serial=SERIAL        Connect to specific alluris device using serial\n\
                             number. This only works if the device is stopped.\n\
\n\
 Measurement:\n\
  -n, --neg-peak             Negative peak\n\
  -p, --pos-peak             Positive peak\n\
      --start                Start\n\
      --stop                 Stop\n\
  -s, --sample=NUM           Capture NUM values (Inf if NUM==0)\n\
  -v, --value                Get single value without starting streaming\n\
\n\
 Tare:\n\
      --clear-neg            Clear negative peak\n\
      --clear-pos            Clear positive peak\n\
  -t, --tare                 Tare measurement\n\
\n\
 Settings:\n\
      --factory-defaults     Restore factory defaults\n\
      --get-auto-stop        Get auto-stop\n\
      --set-auto-stop=S      Set auto-stop 0..30\n\
      --get-lower-limit      Param P4, lower limit\n\
      --set-lower-limit=P4   Param P4, lower limit\n\
      --get-upper-limit      Param P3, upper limit\n\
      --set-upper-limit=P3   Param P3, upper limit\n\
      --get-mode             Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-\n\
      --set-mode=MODE        Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-\n\
      --get-mem-mode         Memory mode 0=disabled, 1=single, 2=continuous\n\
      --set-mem-mode=MODE    Memory mode 0=disabled, 1=single, 2=continuous\n\
      --get-unit             Unit\n\
      --set-unit=U           Unit 'N', 'cN', 'kg', 'g', 'lb', 'oz'\n\
      --get-peak-level       Get CTT/TTT peak-threshold\n\
      --set-peak-level       Set CTT/TTT peak-threshold 1..99%\n\
\n\
 Get read only settings:\n\
      --digits               Digits of used fixed-point numbers\n\
      --fmax                 Fmax\n\
      --resolution           Resolution (1, 2 or 5)\n\
\n\
 Misc:\n\
      --verbose              Be chatty\n\
      --brief                Only output values  and errors (default)\n\
      --delete-memory        Delete the measurement memory\n\
      --debug=LVL            Set debug level 0..2\n\
      --get-digin            Get state of the digital input (firmware >=\n\
                             V4.04.007/V5.04.007)\n\
      --get-digout           Get state of the 3 digital outputs (firmware >=\n\
                             V4.03.008/V5.03.008)\n\
      --get-firmware         Get firmware of communication and measurement\n\
                             processor\n\
      --get-mem-count        Get number of values in memory\n\
      --get-next-cal-date    Get the next calibration date as YYMM\n\
      --get-stats            Get statistic (MAX_PLUS, MIN_PLUS, MAX_MINUS,\n\
                             MIN_MINUS, AVERAGE, VARIANCE) from memory values.\n\
                             All values are fixed-point numbers (see --digits)\n\
                             except DEVIATION which uses 3 digits in firmware <\n\
                             V5.04.007 and --digits in newer versions.\n\
      --keypress=KEY         Sim. keypress. Bit 0=S1, 1=S2, 2=S3, 3=long_press.\n\
                             For ex. 12 => long press of S3\n\
      --power-off            Power off the device\n\
      --read-memory=ADR      Read adr 0..999 or -1 for whole memory\n\
      --set-digout=MASK      Set state of the 3 digital outputs = MASK\n\
                             (firmware >= V4.03.008/V5.03.008)\n\
      --set-keylock=V        Lock (V=1) or unlock (V=0) keys. Power-off with S1\n\
                             is still possible. Disconnecting USB automatically\n\
                             unlocks the keys. (firmware >=\n\
                             V4.04.005/V5.04.005)\n\
      --sleep=T              Sleep T milliseconds\n\
      --state                Read RAM state\n\
\n\
      --help                 Give this help list\n\
  -V, --version              Print program version\n\
", stdout);
}

void termination_handler (int signum)
{
  //fprintf (stderr, "Received signal %i, terminating program\n", signum);
  (void) signum;
  do_exit = 1;
}

static int print_multiple (libusb_device_handle *dev_handle, int num)
{
  // check if measurement is running
  struct liballuris_state state;
  int ret = liballuris_read_state (dev_handle, &state, 3000);
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
                  int k = 0;
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

void cleanup (libusb_device_handle* h)
{
  // cleanup after error
  liballuris_clear_RX (h, 1000);

  // disable streaming
  fprintf (stderr, "INFO: Disable streaming, ");
  liballuris_cyclic_measurement (h, 0, 19);

  //empty read RX buffer
  fprintf (stderr, "clearing RX buffer...\n");
  liballuris_clear_RX (h, 1000);
}

int get_base10_int (const char *p, int* value)
{
  char *endptr;
  *value = strtol (p, &endptr, 10);
  if (errno == ERANGE)
    return LIBALLURIS_OUT_OF_RANGE;
  if (endptr == p) // no digits
    return LIBALLURIS_PARSE_ERROR;
  return 0;
}

static struct option const long_options[] =
{
  {"verbose", no_argument, &verbose_flag, 1},
  {"brief", no_argument, &verbose_flag, 0},
  {"debug", required_argument, NULL, 'd'},
  {"list", no_argument, NULL, 'l'},
  {"serial", required_argument, NULL, 'S'},

  {"neg-peak", no_argument, NULL, 'n'},
  {"pos-peak", no_argument, NULL, 'p'},
  {"start", no_argument, NULL, 1000},
  {"stop", no_argument, NULL, 1001},
  {"sample", required_argument, NULL, 's'},
  {"value", no_argument, NULL, 'v'},

  {"clear-neg", no_argument, NULL, 1010},
  {"clear-pos", no_argument, NULL, 1011},
  {"tare", no_argument, NULL, 't'},

  {"factory-defaults", no_argument, NULL, 1020},
  {"get-auto-stop", no_argument, NULL, 1021},
  {"set-auto-stop", required_argument, NULL, 1022},
  {"get-lower-limit", no_argument, NULL, 1023},
  {"set-lower-limit", required_argument, NULL, 1024},
  {"get-upper-limit", no_argument, NULL, 1025},
  {"set-upper-limit", required_argument, NULL, 1026},
  {"get-mode", no_argument, NULL, 1027},
  {"set-mode", required_argument, NULL, 1028},
  {"get-mem-mode", no_argument, NULL, 1029},
  {"set-mem-mode", required_argument, NULL, 1030},
  {"get-unit", no_argument, NULL, 1031},
  {"set-unit", required_argument, NULL, 1032},
  {"get-peak-level", no_argument, NULL, 1033},
  {"set-peak-level", required_argument, NULL, 1034},

  {"digits", no_argument, NULL, 1040},
  {"fmax", no_argument, NULL, 1041},
  {"resolution", no_argument, NULL, 1042},

  {"delete-memory", no_argument, NULL, 1060},
  {"get-digin", no_argument, NULL, 1061},
  {"get-digout", no_argument, NULL, 1062},
  {"get-firmware", no_argument, NULL, 1063},
  {"get-mem-count", no_argument, NULL, 1064},
  {"get-next-cal-date", no_argument, NULL, 1065},
  {"get-stats", no_argument, NULL, 1066},
  {"keypress", required_argument, NULL, 1067},
  {"power-off", no_argument, NULL, 1068},
  {"read-memory", required_argument, NULL, 1069},
  {"set-digout", required_argument, NULL, 1070},
  {"set-keylock", required_argument, NULL, 1071},
  {"sleep", required_argument, NULL, 1072},
  {"state", no_argument, NULL, 1073},
  {"help", no_argument, NULL, 1074},
  {"version", no_argument, NULL, 'V'},

  {NULL, 0, NULL, 0}
};

int
main (int argc, char **argv)
{
  if (argc == 1)
    {
      fprintf (stderr, "Error: No commands.\n");
      fprintf (stderr, "Try `gadc --help' for more information.\n");
      return EXIT_FAILURE;
    }

  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);

  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

#ifndef _WIN32
  if (signal (SIGPIPE, termination_handler) == SIG_IGN)
    signal (SIGPIPE, SIG_IGN);
#endif

  libusb_context* ctx = 0;
  libusb_device_handle* h = 0;
  //int error = 0;
  //int last_key = 0;

  int r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Error: Couldn't init libusb %s\n", liballuris_error_name (r));
      return EXIT_FAILURE;
    }

  int c = 0;

  //usage ();

  /* getopt_long stores the option index here. */
  int option_index;

  while (! do_exit && ! r)
    {
      option_index = -1;
      c = getopt_long (argc, argv, "b:lS:nps:vtVd:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      if (c == 'd') // debug
        {
          int tmp_debug;
          r = get_base10_int (optarg, &tmp_debug);
          if (! r)
            liballuris_debug_level = tmp_debug;
          continue;
        }

      //be chatty
      if (verbose_flag && c != '?' && c > 0)
        {
          if (option_index >= 0)
            printf ("Processing long option '%s'", long_options[option_index].name);
          else if (c > 0)
            printf ("Processing short option '%c'", c);

          if (optarg)
            printf (" with optarg = '%s'", optarg);
          printf ("\n");
        }

      if (c!='b' && c!='S' && c!='l' && c!='V' && c!='?' && c && c != 1074)
        {
          r = liballuris_open_if_not_opened (ctx, NULL, &h);
          if (r)
            break;
        }

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          break;

        case 'l': // list
          liballuris_print_device_list (stdout, ctx);
          break;

        case 'b': // bus,device id
        case 'S': // serial
          //printf ("option -%c with value `%s'\n", c, optarg);

          if (h)
            {
              //printf ("libusb_release_interface\n");
              libusb_release_interface (h, 0);

              //printf ("libusb_close\n");
              libusb_close (h);

              h = 0;
            }
          r = liballuris_open_if_not_opened (ctx, optarg, &h);
          break;

        case 'n': // neg-peak
        {
          int value;
          r = liballuris_get_neg_peak (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 'p': // pos-peak
        {
          int value;
          r = liballuris_get_pos_peak (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1000: // start
          r = liballuris_start_measurement (h);
          break;

        case 1001: // stop
          r = liballuris_stop_measurement (h);
          break;

        case 's': // read multiple samples
        {
          int num_samples;
          r = get_base10_int (optarg, &num_samples);
          if (r == LIBUSB_SUCCESS)
            {
              if (num_samples >= 0)
                {
                  //printf ("num_samples=%i\n", num_samples);
                  r = print_multiple (h, num_samples);
                  //printf ("print_multiple returned %i\n", r);
                }
              else
                {
                  fprintf (stderr, "Error: NUM(=%i) has to be > 1 or 0 (read until sigint or sigterm)\n", num_samples);
                  r = LIBALLURIS_OUT_OF_RANGE;
                }
            }
          break;
        }
        case 'v': // read one sample
        {
          int value;
          r = liballuris_get_value (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1010: // clear-neg
          r = liballuris_clear_neg_peak (h);
          break;
        case 1011: // clear-pos
          r = liballuris_clear_pos_peak (h);
          break;
        case 't': // tare
          r = liballuris_tare (h);
          // tare needs some time to calculate the mean (ca. 200ms)
          // usleep (200000);
          break;
        case 1020: // factory-defaults
          r = liballuris_restore_factory_defaults (h);
          break;

        case 1021: // get-auto-stop
        {
          int value;
          r = liballuris_get_autostop (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1022: // set-auto-stop
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_autostop (h, value);
          break;
        }

        case 1023: // get-lower-limit
        {
          int value;
          r = liballuris_get_lower_limit (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1024: // set-lower-limit
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_lower_limit (h, value);
          break;
        }

        case 1025: // get-upper-limit
        {
          int value;
          r = liballuris_get_upper_limit (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1026: // set-upper-limit
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_upper_limit (h, value);
          break;
        }

        case 1027: // get-mode
        {
          enum liballuris_measurement_mode r_mode;
          r = liballuris_get_mode (h, &r_mode);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", r_mode);
          break;
        }

        case 1028: // set-mode
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            {
              enum liballuris_measurement_mode r_mode = value;
              r = liballuris_set_mode (h, r_mode);
            }
          break;
        }

        case 1029: // get-mem-mode
        {
          enum liballuris_memory_mode r_mem_mode;
          r = liballuris_get_mem_mode (h, &r_mem_mode);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", r_mem_mode);
          break;
        }

        case 1030: // set-mem-mode
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            {
              enum liballuris_memory_mode r_mem_mode = value;
              r = liballuris_set_mem_mode (h, r_mem_mode);
            }
          break;
        }

        case 1031: // get-unit
        {
          enum liballuris_unit r_unit;
          r = liballuris_get_unit (h, &r_unit);
          if (r == LIBUSB_SUCCESS)
            printf ("%s\n", liballuris_unit_enum2str (r_unit));
          break;
        }

        case 1032: // set-unit
        {
          enum liballuris_unit r_unit = liballuris_unit_str2enum (optarg);
          r = liballuris_set_unit (h, r_unit);
          break;
        }

        case 1033: // get-peak-level
        {
          int value;
          r = liballuris_get_peak_level (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1034: // set-peak-level
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_peak_level (h, value);
          break;
        }

        case 1040: // digits
        {
          int value;
          r = liballuris_get_digits (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1041: // fmax
        {
          int value;
          r = liballuris_get_F_max (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1042: // resolution
        {
          int value;
          r = liballuris_get_resolution (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1060: // delete-memory
          r = liballuris_delete_memory (h);
          break;

        case 1061: // get-digin
        {
          int value;
          r = liballuris_get_digin (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1062: // get-digout
        {
          int value;
          r = liballuris_get_digout (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1063: // get-firmware
        {
          char firmware_buf[21];
          r = liballuris_get_firmware (h, 0, firmware_buf, 21);
          if (r == LIBUSB_SUCCESS)
            {
              printf ("%s;", firmware_buf);
              r = liballuris_get_firmware (h, 1, firmware_buf, 21);
              if (r == LIBUSB_SUCCESS)
                printf ("%s\n", firmware_buf);
            }
          break;
        }

        case 1064: // get-mem-count
        {
          int value;
          r = liballuris_get_mem_count (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1065: // get-next-cal-date
        {
          int value;
          r = liballuris_get_next_calibration_date (h, &value);
          if (r == LIBUSB_SUCCESS)
            printf ("%i\n", value);
          break;
        }

        case 1066: // get-stats
        {
          int stats[6];
          r = liballuris_get_mem_statistics (h, stats, 6);
          if (r == LIBUSB_SUCCESS)
            {
              printf ("MAX_PLUS  (raw) = %5i\n", stats[0]);
              printf ("MIN_PLUS  (raw) = %5i\n", stats[1]);
              printf ("MAX_MINUS (raw) = %5i\n", stats[2]);
              printf ("MIN_MINUS (raw) = %5i\n", stats[3]);
              printf ("AVERAGE   (raw) = %5i\n", stats[4]);
              printf ("VARIANCE  (raw) = %5i\n", stats[5]);
            }
          break;
        }

        case 1067: // keypress
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_sim_keypress (h, value);
          break;
        }

        case 1068: // power-off
          r = liballuris_power_off (h);
          break;

        case 1069: // read-memory
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            {
              int start_adr = value;
              int stop_adr = value;
              if (value == -1)
                {
                  start_adr = 0;
                  stop_adr = 999;
                }
              while (start_adr <= stop_adr)
                {
                  r = liballuris_read_memory (h, start_adr++, &value);
                  if (r == LIBUSB_SUCCESS)
                    printf ("%i\n", value);
                }
            }
          break;
        }

        case 1070: // set-digout
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_digout (h, value);
          break;
        }

        case 1071: // set-keylock
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            r = liballuris_set_key_lock (h, value);
          break;
        }

        case 1072: // sleep [ms]
        {
          int value;
          r = get_base10_int (optarg, &value);
          if (r == LIBUSB_SUCCESS)
            usleep (value * 1000);
          break;
        }

        case 1073: // state
        {
          struct liballuris_state device_state;
          r = liballuris_read_state (h, &device_state, 3000);
          if (r == LIBUSB_SUCCESS)
            liballuris_print_state (device_state);
          break;
        }

        case 1074: // help
          usage ();
          break;

        case 'V': // version
          printf ("%s version %s, Copyright (c) 2015-2016 Alluris GmbH & Co. KG\n",
                  program_name,
                  program_version);
          printf (" built on %s %s. ", __DATE__, __TIME__);
          printf ("Please report problems to %s\n", program_bug_address);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          printf ("Use '--help' to see possible options\n");
          break;

        default:
          fprintf (stderr, "Error: This should never happen...\n");
          abort ();
        }
    }

  // check if the loop was exited due to an error
  // add option info
  if (r)
    {
      fprintf (stderr, "Error: '%s' ", liballuris_error_name (r));

      if (option_index >= 0)
        fprintf (stderr, "while processing long option '%s'",
                 long_options[option_index].name);
      else if (c > 0)
        fprintf (stderr, "while processing short option '%c'", c);

      if (optarg)
        fprintf (stderr, " with optarg = '%s'", optarg);
      fprintf (stderr, "\n");
    }

  // check if we have any remaining command line arguments (not options)
  if (c == -1 && optind < argc)
    {
      fprintf (stderr, "Error: non-option arguments aren't allowed (yet)\n");
      r = LIBALLURIS_OUT_OF_RANGE;
    }

  if (   r == LIBALLURIS_MALFORMED_REPLY
         || r == LIBUSB_ERROR_OVERFLOW
         || r == LIBUSB_ERROR_TIMEOUT)
    cleanup (h);

  if (h)
    {
      //printf ("libusb_release_interface\n");
      libusb_release_interface (h, 0);

      //printf ("libusb_close\n");
      libusb_close (h);
    }

  libusb_exit (ctx);
  return r;
}
