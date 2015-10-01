#!/usr/bin/env bats

## Tests gadc

GADC=../cli/gadc

@test "Stop, set mode = 0 (10Hz), start measurement, capture one value" {
  run $GADC --stop --set-mode 0 --start -v
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "List devices" {
  run $GADC --list
  [ "$status" -eq 0 ]
}

@test "Stop, set mode = 1 (900Hz), start measurement, capture one value" {
  run $GADC --stop --set-mode 1 --start -v
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "Start, stop, start, stop" {
  run $GADC --start --stop --start --stop
  [ "$status" -eq 0 ]
}

@test "Set mode=0, start, tare, capture on value, stop" {
  run $GADC --set-mode 1 --start --tare -v --stop
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Get firmware version" {
  run $GADC --get-firmware
  [ "$status" -eq 0 ]
}

@test "Sleep 500ms" {
  run $GADC --sleep 500
  [ "$status" -eq 0 ]
}

@test "Get state" {
  run $GADC --state
  [ "$status" -eq 0 ]
}

@test "Get help" {
  run $GADC --help
  [ "$status" -eq 0 ]
}

@test "Get usage" {
  run $GADC --usage
  [ "$status" -eq 0 ]
}

@test "Get version" {
  run $GADC --version
  [ "$status" -eq 0 ]
}

@test "Try to restore factory defaults while measuring, check for LIBALLURIS_DEVICE_BUSY" {
  run $GADC --start --factory-defaults
  [ "$status" -eq 2 ]
}

@test "Stop device to restore factory defaults" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Restore factory defaults" {
  run $GADC --factory-defaults
  [ "$status" -eq 0 ]
}

@test "Power off device" {
  run $GADC --power-off
  [ "$status" -eq 0 ]
}
