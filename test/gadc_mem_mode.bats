#!/usr/bin/env bats

## Tests gadc
##    --get-mem-mode         Memory mode 0=disabled, 1=single, 2=continuous
##    --set-mem-mode=MODE    Memory mode 0=disabled, 1=single, 2=continuous
##    --read-memory
##    --delete-memory
##    --get-stats
##    --get-mem-count

## --set-mem-mode=MODE only works if measurement is stopped
## --get-mem-mode also works while measurement is running

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

## check memory modes in standard mode
## < 0 and > 2 should return LIBALLURIS_OUT_OF_RANGE

@test "Set mem mode = -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mem-mode -1
  [ "$status" -eq 4 ]
}

@test "Set mem mode = 1" {
  run $GADC --set-mem-mode 1
  [ "$status" -eq 0 ]
}

@test "Check mem mode == 1" {
  run $GADC --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "Set mem mode = 2" {
  run $GADC --set-mem-mode 2
  [ "$status" -eq 0 ]
}

@test "Check mem mode == 2" {
  run $GADC --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 2 ]
}

@test "Set mem mode = 3, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mem-mode 3
  [ "$status" -eq 4 ]
}

@test "Set mem mode = 0" {
  run $GADC --set-mem-mode 0
  [ "$status" -eq 0 ]
}

@test "Check mem mode == 0" {
  run $GADC --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

## Switch to peak mode (900 Hz sampling rate)
@test "Set mode = 1" {
  run $GADC --set-mode 1
  [ "$status" -eq 0 ]
}

## check memory modes in peak mode
@test "Set mem mode = 2 and check" {
  run $GADC --set-mem-mode 2 --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 2 ]
}

@test "Set mem mode = 1 and check" {
  run $GADC --set-mem-mode 1 --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "Set mem mode = 0 and check" {
  run $GADC --set-mem-mode 0 --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

## Start measurement
@test "Set 10Hz mode and start to measure" {
  run $GADC --set-mode 0 --start
  [ "$status" -eq 0 ]
}

@test "Set mem mode = 1, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-mem-mode 1
  [ "$status" -eq 2 ]
}

@test "Check mem mode == 0 while measurement running" {
  run $GADC --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}
