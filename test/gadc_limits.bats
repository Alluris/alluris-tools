#!/usr/bin/env bats

## Tests gadc
##      --get-lower-limit      Param P4, lower limit
##      --get-upper-limit      Param P3, upper limit
##      --set-lower-limit=P4   Param P4, lower limit
##      --set-upper-limit=P3   Param P3, upper limit

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Set upper limit (P3) = 135" {
  run $GADC --set-upper-limit 135
  [ "$status" -eq 0 ]
}

@test "Check upper limit (P3)" {
  run $GADC --get-upper-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq 135 ]
}

@test "Set lower limit (P4) = -42" {
  run $GADC --set-lower-limit -42
  [ "$status" -eq 0 ]
}

@test "Check lower limit (P4)" {
  run $GADC --get-lower-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq -42 ]
}

## start measurement

@test "Set 10Hz mode and start to measure" {
  run $GADC --set-mode 0 --start
  [ "$status" -eq 0 ]
}

@test "Set upper limit while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-upper-limit 100
  [ "$status" -eq 2 ]
}

@test "Set lower limit while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-lower-limit -100
  [ "$status" -eq 2 ]
}

@test "Get upper limit while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --get-upper-limit
  [ "$status" -eq 2 ]
}

@test "Get lower limit while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --get-lower-limit
  [ "$status" -eq 2 ]
}

## stop measurement and disable limits

@test "Stop measurement again" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Set upper limit (P3) = 0" {
  run $GADC --set-upper-limit 0
  [ "$status" -eq 0 ]
}

@test "Check upper limit (P3) == 0" {
  run $GADC --get-upper-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Set lower limit (P4) = 0" {
  run $GADC --set-lower-limit 0
  [ "$status" -eq 0 ]
}

@test "Check lower limit (P4) == 0" {
  run $GADC --get-lower-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq -0 ]
}
