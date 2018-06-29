#!/usr/bin/env bats

## Tests gadc --digits
## only works if measurement is stopped

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Get digits" {
  run $GADC --digits
  [ "$status" -eq 0 ]
  [ "$output" -ge 0 ] && [ "$output" -le 3 ]
}

@test "Start measurement" {
  run $GADC --start
  [ "$status" -eq 0 ]
}

@test "Get digits while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --digits
  [ "$status" -eq 2 ]
}

@test "Stop measurement again" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Get digits again " {
  run $GADC --digits
  [ "$status" -eq 0 ]
  [ "$output" -ge 0 ] && [ "$output" -le 3 ]
}
