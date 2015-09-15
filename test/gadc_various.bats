#!/usr/bin/env bats

## Tests gadc

GADC=../cli/gadc

@test "Stop, set mode = 0 (10Hz), start measurement, capture one value" {
  run $GADC --stop --set-mode 0 --start -v
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "Stop, set mode = 1 (900Hz), start measurement, capture one value" {
  run $GADC --stop --set-mode 1 --start -v
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "Start, stop, start, stop" {
  run $GADC --start --stop --start --stop
  [ "$status" -eq 0 ]
}



