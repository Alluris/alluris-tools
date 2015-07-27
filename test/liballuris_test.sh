#!/bin/bash

## set defaults
../cli/gadc --stop
../cli/gadc --set-pos-limit 0
../cli/gadc --set-neg-limit 0

## check pos limit
../cli/gadc --set-pos-limit 321
export value=$(../cli/gadc --get-pos-limit)
if (($value != 321)); then
  printf "set-pos-limit FAILED\n"
  exit 1;
fi

## check neg limit
../cli/gadc --set-neg-limit -123
export value=$(../cli/gadc --get-neg-limit)
if (($value != -123)); then
  printf "set-neg-limit FAILED\n"
  exit 1;
fi

## Start measurement, read one value, stop
../cli/gadc --start
../cli/gadc --value
../cli/gadc --stop

## in one command
../cli/gadc --start -v --stop

## get pos and neg peaks
../cli/gadc --start

../cli/gadc --pos-peak --neg-peak

../cli/gadc --tare
../cli/gadc --clear-neg
../cli/gadc --clear-pos

../cli/gadc --stop
