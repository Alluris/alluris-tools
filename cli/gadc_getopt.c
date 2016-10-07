#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

const char *program_name = "gadc";

const char *program_version = "0.2.3";

const char *program_bug_address = "<software@alluris.de>";
  
void usage ()
{
  printf ("Usage: %s [OPTION]...\n", program_name);
  fputs ("\
Generic Alluris device control\n\
\n\
 Device discovery and connection:\n\
  -b Bus,Device              Connect to specific alluris device using bus and\n\
                             device id\n\
  -l, --list                 List accessible (stopped and not claimed) devices\n\
      --serial=SERIAL        Connect to specific alluris device using serial\n\
                             number. This only works if the device is stopped.\n\
\n\
 Measurement:\n\
  -n, --neg-peak             Negative peak\n\
  -p, --pos-peak             Positive peak\n\
      --start                Start\n\
      --stop                 Stop\n\
  -s, --sample=NUM           Capture NUM values (Inf if NUM==0)\n\
  -v, --value                Value\n\
\n\
 Tare:\n\
      --clear-neg            Clear negative peak\n\
      --clear-pos            Clear positive peak\n\
  -t, --tare                 Tare measurement\n\
\n\
 Settings:\n\
      --factory-defaults     Restore factory defaults\n\
      --get-auto-stop        Get auto-stop\n\
      --get-lower-limit      Param P4, lower limit\n\
      --get-mem-mode         Memory mode 0=disabled, 1=single, 2=continuous\n\
      --get-mode             Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-\n\
      --get-unit             Unit\n\
      --get-upper-limit      Param P3, upper limit\n\
      --set-auto-stop=S      Set auto-stop 0..30\n\
      --set-lower-limit=P4   Param P4, lower limit\n\
      --set-mem-mode=MODE    Memory mode 0=disabled, 1=single, 2=continuous\n\
      --set-mode=MODE        Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-\n\
      --set-unit=U           Unit 'N', 'cN', 'kg', 'g', 'lb', 'oz'\n\
      --set-upper-limit=P3   Param P3, upper limit\n\
\n\
 Get fixed attributes:\n\
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

int
main (int argc, char **argv)
{
  usage ();
  
  
  
  return EXIT_SUCCESS;
}
