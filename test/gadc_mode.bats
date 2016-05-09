#!/usr/bin/env bats

## Tests gadc
##   --get-mode             Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-
##   --set-mode=MODE        Measurement mode 0=std, 1=peak, 2=peak+, 3=peak-

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Set mode = 2 (Peak+) and check" {
  run $GADC --set-mode 2 --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 2 ]
}

@test "Set mode = 3 (Peak-) and check" {
  run $GADC --set-mode 3 --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 3 ]
}

@test "Set mode = 0 (10Hz standard)" {
  run $GADC --set-mode 0 --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Start measurement, read 10 samples" {
  run $GADC --start -s 10
  [ "$status" -eq 0 ]
}

@test "Set mode = 1, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-mode 1
  [ "$status" -eq 2 ]
}

@test "Check mode == 0 while running)" {
  run $GADC --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Stop 10Hz measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Check mode == 0)" {
  run $GADC --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Set mode = 1 (900Hz peak)" {
  run $GADC --set-mode 1
  [ "$status" -eq 0 ]
}

@test "Start measurement, read 900 samples" {
  run $GADC --start -s 900
  [ "$status" -eq 0 ]
}

@test "Set mode = 0, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-mode 0
  [ "$status" -eq 2 ]
}

@test "Check mode == 1 while running)" {
  run $GADC --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "Stop 900Hz measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Check mode == 1)" {
  run $GADC --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

## out of range checks

@test "Set mode = -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mode -1
  [ "$status" -eq 4 ]
}

@test "Set mode = 4, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-mode 4
  [ "$status" -eq 4 ]
}
