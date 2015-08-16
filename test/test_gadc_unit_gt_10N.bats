#!/usr/bin/env bats

## test unit on devices with fmax > 10N

GADC=../cli/gadc

@test "INIT: stopping measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Check F_max > 10N" {
  run $GADC --fmax
  [ "$status" -eq 0 ]
  [ "$output" -gt 10 ]
}

@test "Set unit N" {
  run $GADC --set-unit N
  [ "$status" -eq 0 ]
}

@test "Set unit Kg" {
  run $GADC --set-unit kg
  [ "$status" -eq 0 ]
}

@test "Get unit, check == kg" {
  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "kg" ]
}

@test "Set unit lb" {
  run $GADC --set-unit lb
  [ "$status" -eq 0 ]
}

@test "Get unit, check == lb" {
  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "lb" ]
}

@test "Set unit N" {
  run $GADC --set-unit N
  [ "$status" -eq 0 ]
}

@test "Get unit, check == N" {
  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "N" ]
}

### check for errors

@test "Try to set unit 'g', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-unit g
  [ "$status" -eq 4 ]
}

@test "Try to set unit 'oz', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-unit oz
  [ "$status" -eq 4 ]
}

@test "Try to set unit 'N' while measuring, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --start --set-unit N
  [ "$status" -eq 2 ]
}

@test "Try to get unit while measuring, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --start --get-unit
  [ "$status" -eq 2 ]
}

@test "FINALIZE: Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}
