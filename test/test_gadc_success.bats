#!/usr/bin/env bats

## Tests gadc for success

GADC=../cli/gadc

## Set defaults (10Hz default mode, no limits)

@test "INIT: stopping measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "INIT: set upper limit = 0" {
  run $GADC --set-upper-limit 0
  [ "$status" -eq 0 ]
}

@test "INIT: set lower limit = 0" {
  run $GADC --set-lower-limit 0
  [ "$status" -eq 0 ]
}

@test "INIT: clear positive peak" {
  run $GADC --clear-pos
  [ "$status" -eq 0 ]
}

@test "INIT: clear negative peak" {
  run $GADC --clear-neg
  [ "$status" -eq 0 ]
}

@test "INIT: set mode = 0" {
  run $GADC --set-mode 0
  [ "$status" -eq 0 ]
}

@test "INIT: set mem mode = 0" {
  run $GADC --set-mem-mode 0
  [ "$status" -eq 0 ]
}

@test "INIT: tare" {
  run $GADC --tare
  [ "$status" -eq 0 ]
}

## Check limits with standard mode (10 Hz sampling freq.)

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

## Start and stop with standard mode (10 Hz sampling rate)

@test "Start measurement" {
  run $GADC --start
  [ "$status" -eq 0 ]
}

@test "Capture and check value" {
  run $GADC -v
  [ "$status" -eq 0 ]
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Start measurement, read one value, check if it's in the range -10..10, stop measurement" {
  run $GADC --start -v --stop
  [ "$status" -eq 0 ]
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

## Switch to peak mode (900 Hz sampling rate)
@test "Set mode = 1" {
  run $GADC --set-mode 1
  [ "$status" -eq 0 ]
}

@test "Check mode == 1" {
  run $GADC --get-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "Start measurement, read one value, check if it's in the range -10..10, stop measurement" {
  run $GADC --start -v --stop
  [ "$status" -eq 0 ]
  [ "$output" -ge -10 ] && [ "$output" -le 10 ]
}

@test "Start, stop, start, stop" {
  run $GADC --start --stop --start --stop
  [ "$status" -eq 0 ]
}

## check memory modes in standard mode

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
@test "Set mem mode = 0" {
  run $GADC --set-mem-mode 0
  [ "$status" -eq 0 ]
}

@test "Check mem mode == 0" {
  run $GADC --get-mem-mode
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

## check memory modes in peak mode
@test "Set mem mode = 2 and check" {
  run $GADC --set-mode 1 --set-mem-mode 2 --get-mem-mode
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

