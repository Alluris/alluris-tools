/*
 * Program discovers FMI-S/B devices on USB
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <liballuris.h>

static char do_exit = 0;

void termination_handler (int signum)
{
  //fprintf (stderr, "Received signal %i, terminating program\n", signum);
  (void) signum;
  do_exit = 1;
}

int all (char* p, int len)
{
  int k;
  for (k=0; k<len; ++k)
    if (!p[k])
      return 0;

  return 1;
}

int
main (int argc, char **argv)
{
  liballuris_debug_level = 0;
  (void) argc;
  (void) argv;

  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);

  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);


  libusb_context* ctx = 0;
  //libusb_device_handle* h = 0;
  //int error = 0;
  //int last_key = 0;

  int r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Error: Couldn't init libusb %s\n", liballuris_error_name (r));
      return EXIT_FAILURE;
    }

  // Check available devices
#define MULTI_MAX_NUM_DEVICES 8
  struct alluris_device_description alluris_devs[MULTI_MAX_NUM_DEVICES];
  ssize_t cnt = liballuris_get_device_list (ctx, alluris_devs, MULTI_MAX_NUM_DEVICES, 1);


  // print header with bus and device address
  char *bus_device_adress_buffer;
  size_t size_bus_device_adress_buffer;
  FILE *stream_bus_device_adress_buffer = open_memstream (&bus_device_adress_buffer, &size_bus_device_adress_buffer);

  char *serial_buffer;
  size_t size_serial_buffer;
  FILE *stream_serial_buffer = open_memstream (&serial_buffer, &size_serial_buffer);

  // open all devices and output header
  libusb_device_handle* handles[cnt];

  int k;
  for (k=0; k < cnt; k++)
    {
      /*
      printf (" %03i; %03d; %03d; %-25s; %s\n", k+1,
              libusb_get_bus_number(alluris_devs[k].dev),
              libusb_get_device_address(alluris_devs[k].dev),
              alluris_devs[k].product,
              alluris_devs[k].serial_number);
      */

      int bus = libusb_get_bus_number(alluris_devs[k].dev);
      int device = libusb_get_device_address(alluris_devs[k].dev);

      fprintf (stream_bus_device_adress_buffer, "%03i,%03i", bus, device);
      fprintf (stream_serial_buffer, "%s", alluris_devs[k].serial_number);

      if (k < (cnt - 1))
        {
          fprintf (stream_bus_device_adress_buffer, " ");
          fprintf (stream_serial_buffer, " ");
        }

      r = liballuris_open_device_with_id (ctx, bus, device, handles + k);
      if (r)
        {
          fprintf (stderr, "Error: Couldn't open device with bus=%i and device=%i: %s\n", bus, device, liballuris_error_name (r));
        }

      r = libusb_claim_interface (handles[k], 0);
      if (r)
        {
          fprintf (stderr, "Error: Couldn't claim interface: %s\n", liballuris_error_name (r));
        }

    }
  fflush (stream_bus_device_adress_buffer);
  fflush (stream_serial_buffer);

  printf ("# %s\n", bus_device_adress_buffer, size_bus_device_adress_buffer);
  printf ("# %s\n", serial_buffer, size_serial_buffer);

  fclose (stream_bus_device_adress_buffer);
  free (bus_device_adress_buffer);
  fclose (stream_serial_buffer);
  free (serial_buffer);

  // check if measurement is running, if not start it
  char do_sleep = 0;
  for (k=0; k < cnt; k++)
    {
      liballuris_clear_RX (handles[k], 100);
      struct liballuris_state state;
      r = liballuris_read_state (handles[k], &state, 3000);
      if (r == LIBUSB_SUCCESS)
        {
          if (! state.measuring)
            {
              do_sleep = 1;
              printf ("# Starting measurements on device %i...\n", k);
              r = liballuris_start_measurement (handles[k]);
              if (r)
                {
                  fprintf (stderr, "Error: Couldn't start device %i : %s\n", k, liballuris_error_name (r));
                  do_exit = 1;
                }
            }
        }
      else
        {
          fprintf (stderr, "Error: Couldn't read state of device %i: %s\n", k, liballuris_error_name (r));
          do_exit = 1;
        }
    }

  //if (do_sleep)
  //  usleep (3e6);

  // recheck that measurment is running, start streaming
  int block_size = 19;
  for (k=0; k < cnt; k++)
    {
      struct liballuris_state state;
      r = liballuris_read_state (handles[k], &state, 3000);

      if (r == LIBUSB_SUCCESS)
        {
          if (state.measuring)
            {
              // enable streaming
              r = liballuris_cyclic_measurement (handles[k], 1, block_size);
              if (r)
                {
                  fprintf (stderr, "Error: Couldn't start streaming on device %i : %s\n", k, liballuris_error_name (r));
                  do_exit = 1;
                }
            }
          else
            {
              fprintf (stderr, "Error: Device %i is still not running...\n", k);
              do_exit = 1;
            }
        }
    }

  //poll all devices
  int num = 2500000;
  int row_cnt = 0;

  int tempx [cnt][block_size];
  char dvalid [cnt];

  do
    {
      memset (dvalid, 0, cnt);

      do
        {
          for (k=0; k < cnt; k++)
            {
              size_t act;
              r = liballuris_poll_measurement_no_wait (handles[k], tempx[k], block_size, &act);
              //printf ("k=%i act=%i\n", k, act);
              if (act == block_size)
                dvalid[k] = 1;

              //printf (" %i", dvalid[k]);
            }
          //printf ("\n");
        }
      while (!all (dvalid, cnt) && !do_exit);

      // display
      if (! do_exit)
        {
          int i;
          for (i = 0; i<block_size; ++i)
            for (k=0; k < cnt; k++)
              printf ("%8i%c", tempx[k][i], (k < (cnt - 1))? ' ':'\n');

          row_cnt += block_size;
        }

    }
  while (row_cnt < num && !do_exit);

  // stop all devices
  for (k=0; k < cnt; k++)
    {
      // disable streaming
      r = liballuris_cyclic_measurement (handles[k], 0, 19);
      if (r)
        fprintf (stderr, "Error: Couldn't disable streaming: %s\n", liballuris_error_name (r));

      liballuris_clear_RX (handles[k], 100);

      //r = liballuris_stop_measurement (handles[k]);
      //if (r)
      //  fprintf (stderr, "Error: Couldn't stop device: %s\n", liballuris_error_name (r));

      //printf ("libusb_release_interface\n");
      libusb_release_interface (handles[k], 0);

      //printf ("libusb_close\n");
      libusb_close (handles[k]);
    }

  // free device list
  liballuris_free_device_list (alluris_devs, MULTI_MAX_NUM_DEVICES);

  libusb_exit (ctx);
  return r;
}

/*
 * 03.11.2016
 * wenn man gadc -b 1,6 --debug 2 --stop --start -s19
 * macht, komt beim read_state nach --start sporadisch ein busy zurück
 * Ist dies der Fall, kann man streaming aktivieren, aber es kommen keine Werte
 */
