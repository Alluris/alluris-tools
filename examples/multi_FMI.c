/*
 *
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
  libusb_device_handle* h = 0;
  //int error = 0;
  //int last_key = 0;

  int r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Error: Couldn't init libusb %s\n", liballuris_error_name (r));
      return EXIT_FAILURE;
    }

  // Check available devices
#define MULTI_MAX_NUM_DEVICES 4
  struct alluris_device_description alluris_devs[MULTI_MAX_NUM_DEVICES];
  ssize_t cnt = liballuris_get_device_list (ctx, alluris_devs, MULTI_MAX_NUM_DEVICES, 1);

  int k;
  //~ printf ("#Num; Bus; Dev; Product;                   Serial\n");
  //~ for (k=0; k < cnt; k++)
    //~ {
      //~ printf (" %03i; %03d; %03d; %-25s; %s\n", k+1,
              //~ libusb_get_bus_number(alluris_devs[k].dev),
              //~ libusb_get_device_address(alluris_devs[k].dev),
              //~ alluris_devs[k].product,
              //~ alluris_devs[k].serial_number);
//~ 
      //~ //stop devices which are running
      //~ if (! strcmp (alluris_devs[k].serial_number, "*BUSY*"))
        //~ {
          //~ int bus = libusb_get_bus_number(alluris_devs[k].dev);
          //~ int device = libusb_get_device_address(alluris_devs[k].dev);
          //~ fprintf (stderr, "stopping device with bus_id = %i and device_id = %i ...\n", bus, device);
          //~ r = liballuris_open_device_with_id (ctx, bus, device, &h);
          //~ if (! r)
            //~ {
              //~ r = libusb_claim_interface (h, 0);
              //~ if (r)
                //~ {
                  //~ fprintf (stderr, "Error: Couldn't claim interface: %s\n", liballuris_error_name (r));
                //~ }
            //~ }
          //~ else
            //~ fprintf (stderr, "Error: Couldn't open device with bus=%i and device=%i: %s\n", bus, device, liballuris_error_name (r));
//~ 
          //~ r = liballuris_stop_measurement (h);
          //~ if (r)
            //~ fprintf (stderr, "Error: Couldn't stop device: %s\n", liballuris_error_name (r));
        //~ }
    //~ }

  // free device list
  liballuris_free_device_list (alluris_devs, MULTI_MAX_NUM_DEVICES);

  // scan again
  cnt = liballuris_get_device_list (ctx, alluris_devs, MULTI_MAX_NUM_DEVICES, 1);

  // open all devices
  libusb_device_handle* handles[cnt];

  for (k=0; k < cnt; k++)
    {
      printf (" %03i; %03d; %03d; %-25s; %s\n", k+1,
              libusb_get_bus_number(alluris_devs[k].dev),
              libusb_get_device_address(alluris_devs[k].dev),
              alluris_devs[k].product,
              alluris_devs[k].serial_number);

      int bus = libusb_get_bus_number(alluris_devs[k].dev);
      int device = libusb_get_device_address(alluris_devs[k].dev);
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

  //~ // start all devices
  //~ for (k=0; k < cnt; k++)
    //~ {
      //~ liballuris_clear_RX (handles[k], 100);
//~ 
      //~ r = liballuris_set_mode (handles[k], 1);
      //~ if (r)
        //~ fprintf (stderr, "Error: Couldn't set mode=1: %s\n", liballuris_error_name (r));
      //~ r = liballuris_start_measurement (handles[k]);
      //~ if (r)
        //~ fprintf (stderr, "Error: Couldn't start device: %s\n", liballuris_error_name (r));
//~ 
    //~ }

  // check that measurment is running, start streaming
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
            }
          else
            fprintf (stderr, "Error: Measurement is not running...\n");
        }
    }

  //sleep (1);

  //poll all device
  int num = 25;
  for (k=0; k < cnt; k++)
    {
      int tempx[block_size];
      int cnt_values = 0;
      // if num==0, read until sigint or sigterm
      r = 0;
      printf ("k = %i\n", k);
      while (!do_exit && !r && (!num || num > cnt_values))
        {
          //printf ("polling %i, %i left\n", block_size, num);
          r = liballuris_poll_measurement (handles[k], tempx, block_size);
          printf ("poll...\n");
          fflush (stdout);
          if (r == LIBUSB_SUCCESS)
            {
              int i = 0;
              while (i < block_size && (cnt_values < num || !num))
                {
                  //printf ("%i\n", tempx[i++]);
                  i++;
                  printf ("device %i cnt=%i\n", k, cnt_values);
                  cnt_values++;
                }
              fflush (stdout);
            }
          else
            fprintf (stderr, "Error: liballuris_poll_measurement failed: %s\n", liballuris_error_name (r));
        }
    }

  // stop all devices
  for (k=0; k < cnt; k++)
    {
      // disable streaming
      r = liballuris_cyclic_measurement (handles[k], 0, 19);
      if (r)
        fprintf (stderr, "Error: Couldn't disable streaming: %s\n", liballuris_error_name (r));

      liballuris_clear_RX (handles[k], 100);

      //~ r = liballuris_stop_measurement (handles[k]);
      //~ if (r)
        //~ fprintf (stderr, "Error: Couldn't stop device: %s\n", liballuris_error_name (r));

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
