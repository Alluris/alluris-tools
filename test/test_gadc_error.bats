#!/usr/bin/env bats

## Tests gadc for error

GADC=../cli/gadc

## LIBALLURIS_OUT_OF_RANGE checks

@test "Set mem mode = -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mem-mode -1
  [ "$status" -eq 4 ]
}

@test "Set mem mode = 3, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mem-mode 3
  [ "$status" -eq 4 ]
}

@test "Set mode = -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mode -1
  [ "$status" -eq 4 ]
}

@test "Set mode = 4, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mode 4
  [ "$status" -eq 4 ]
}

## LIBALLURIS_DEVICE_BUSY checks

## start measurement

@test "INIT: Set mode 0 and start to measure" {
  run $GADC --set-mode 0 --stat
  [ "$status" -eq 0 ]
}

@test "Get digits, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --digits
  [ "$status" -eq 2 ]
}

## FIXME: these fail at the moment -> fix in liballuris.c

@test "Set upper limit, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-upper-limit 100
  [ "$status" -eq 2 ]
}

@test "Set lower limit, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-lower-limit -100
  [ "$status" -eq 2 ]
}

@test "Get upper limit, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --get-upper-limit
  [ "$status" -eq 2 ]
}

@test "Get lower limit, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --get-lower-limit
  [ "$status" -eq 2 ]
}
