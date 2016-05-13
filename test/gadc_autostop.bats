#!/usr/bin/env bats

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Set auto stop (P14) = 15" {
  run $GADC --set-auto-stop 15
  [ "$status" -eq 0 ]
}

@test "Check auto stop (P14) == 15" {
  run $GADC --get-auto-stop
  [ "$status" -eq 0 ]
  [ "$output" -eq 15 ]
}

@test "Set auto stop (P14) = 30" {
  run $GADC --set-auto-stop 30
  [ "$status" -eq 0 ]
}

@test "Check auto stop (P14) == 30" {
  run $GADC --get-auto-stop
  [ "$status" -eq 0 ]
  [ "$output" -eq 30 ]
}

@test "Set auto stop (P14) = 31, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-auto-stop 31
  [ "$status" -eq 4 ]
}

@test "Set auto stop (P14) = -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-auto-stop -1
  [ "$status" -eq 4 ]
}

@test "Set 10Hz mode and start to measure" {
  run $GADC --set-mode 0 --start
  [ "$status" -eq 0 ]
}

@test "Set auto stop while running, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --set-auto-stop 10
  [ "$status" -eq 2 ]
}

@test "Stop measurement again" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}
