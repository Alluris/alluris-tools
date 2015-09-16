#!/bin/bash

valgrind --leak-check=full ../cli/.libs/lt-gadc \
  --stop --state --get-lower-limit --get-upper-limit --get-mode --get-mem-mode \
  --get-unit --digits --fmax --get-digout --get-firmware \
  --neg-peak --pos-peak --clear-neg --clear-pos --tare\
  --set-lower-limit=0 --set-mem-mode=0 --set-mode=0 --set-unit=N --set-upper-limit=0\
  --start --sample 10 --value --set-digout=0 --stop

## we see
#~ ==5091== Conditional jump or move depends on uninitialised value(s)
#~ ==5091==    at 0x4E378B6: liballuris_set_lower_limit (liballuris.c:895)
#~ ==5091==    by 0x4020EC: parse_opt (gadc.c:289)
#~ ==5091==    by 0x53438ED: group_parse (argp-parse.c:257)
#~ ==5091==    by 0x53438ED: parser_parse_opt (argp-parse.c:755)
#~ ==5091==    by 0x53438ED: parser_parse_next (argp-parse.c:867)
#~ ==5091==    by 0x53438ED: argp_parse (argp-parse.c:921)
#~ ==5091==    by 0x40187A: main (gadc.c:441)
## false detections?
