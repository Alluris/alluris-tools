#!/usr/bin/env bats

GADC=../cli/gadc

## Set defaults

@test "INIT: stopping measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "INIT: set positive limit = 0" {
  run $GADC --set-pos-limit 0
  [ "$status" -eq 0 ]
}

@test "INIT: set negative limit = 0" {
  run $GADC --set-neg-limit 0
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

@test "INIT: tare" {
  run $GADC --tare
  [ "$status" -eq 0 ]
}

## Check limits

@test "Set positive limit (P3) = 135" {
  run $GADC --set-pos-limit 135
  [ "$status" -eq 0 ]
}

@test "Check positive limit (P3)" {
  run $GADC --get-pos-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq 135 ]
}

@test "Set negative limit (P4) = -42" {
  run $GADC --set-neg-limit -42
  [ "$status" -eq 0 ]
}

@test "Check negative limit (P4)" {
  run $GADC --get-neg-limit
  [ "$status" -eq 0 ]
  [ "$output" -eq -42 ]
}

## Start and stop

@test "Start measurement" {
  run $GADC --start
  [ "$status" -eq 0 ]
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
