#!/bin/bash

## set defaults
gadc --stop
gadc --set-pos-limit 0
gadc --set-neg-limit 0

## check pos limit
gadc --set-pos-limit 321
export value=$(gadc --get-pos-limit)
if (($value != 321)); then
  printf "set-pos-limit FAILED\n"
  exit 1;
fi

## check neg limit
gadc --set-neg-limit 4422
export value=$(gadc --get-neg-limit)
if (($value != 4422)); then
  printf "set-neg-limit FAILED\n"
  exit 1;
fi

## Start measurement, read one value
gadc --start
gadc --value

gadc --pos-peak
gadc --neg-peak

gadc --tare
gadc --clear-neg
gadc --clear-pos

gadc --stop
