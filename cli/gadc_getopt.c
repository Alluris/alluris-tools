#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <liballuris.h>

const char *program_name = "gadc";

const char *program_version = "0.2.3";

const char *program_bug_address = "<software@alluris.de>";

char do_exit = 0;

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
  --serial=SERIAL            Connect to specific alluris device using serial\n\
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
\n\
 Get read only settings:\n\
      --digits               Digits of used fixed-point numbers\n\
      --fmax                 Fmax\n\
      --resolution           Resolution (1, 2 or 5)\n\
\n\
 Misc:\n\
      --delete-memory        Delete the measurement memory\n\
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
  -?, --help                 Give this help list\n\
  -V, --version              Print program version\n\
", stdout);
}

void termination_handler (int signum)
{
  //fprintf(stderr, "Received signal %i, terminating program\n", signum);
  (void) signum;
  do_exit = 1;
}

static void print_value (int ret, int value)
{
  if (ret != LIBUSB_SUCCESS)
    fprintf(stderr, "Error: '%s'\n", liballuris_error_name (ret));
  else
    printf ("%i\n", value);
}

void list_devices (libusb_context* ctx)
{
  // list accessible devices and exit
  // FIXME: document that a running measurement prohibits reading the serial_number
      
  struct alluris_device_description alluris_devs[MAX_NUM_DEVICES];
  ssize_t cnt = liballuris_get_device_list (ctx, alluris_devs, MAX_NUM_DEVICES, 1);

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
}
      
static struct option const long_options[] =
{
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
      fprintf (stderr, "ERROR: No commands.\n");
      fprintf (stderr, "Try `gadc --help' for more information.\n");
      return EXIT_FAILURE;
    }

  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);

  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

  if (signal (SIGPIPE, termination_handler) == SIG_IGN)
    signal (SIGPIPE, SIG_IGN);


  libusb_context* ctx = 0;
  libusb_device_handle* h = 0;
  int error = 0;
  int last_key = 0;

  int r = libusb_init (&ctx);
  if (r < 0)
    {
      fprintf (stderr, "Couldn't init libusb %s\n", liballuris_error_name (r));
      return EXIT_FAILURE;
    }

  int c;

  /* First parse to get optional device serial number, bud and device id or list devices */
  while (1)
    {
      int option_index = 0;
      c = getopt_long (argc, argv, "b:lSnpaos:vt",
                       long_options, &option_index);

      if (c == -1)
        break;

    switch (c)
      {
        case 'l': // list
          puts ("option -l\n");
          break;

        case 'b': // bus,device id
          printf ("option -b with value `%s'\n", optarg);
        break;

        case 'S': // serial
          printf ("option -s with value `%s'\n", optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

      }
    }
  
  /* Reset optind to 1 for a second run*/
  optind = 1;

  //usage ();
  while (1)
    {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "b:lSnpaos:vt",
                     long_options, &option_index);

    
    //printf ("option_index=%i\n", option_index);
    //printf ("optind=%i\n", optind);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
      {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf ("option %s", long_options[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'l': // list
      case 'b': // bus,device id
      case 'S': // serial
        // already handled
        break;

      case 'n': // neg-peak
        puts ("option -n\n");
        break;

      case 'p': // pos-peak
        puts ("option -p\n");
        break;

      case '?':
        /* getopt_long already printed an error message. */
        break;

      default:
        abort ();
      }
  }

  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }
 
  return EXIT_SUCCESS;
}
