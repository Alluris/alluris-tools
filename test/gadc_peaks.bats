#!/usr/bin/env bats

## Tests gadc
##  -n, --neg-peak             Negative peak
##  -p, --pos-peak             Positive peak
##      --clear-neg            Clear negative peak
##      --clear-pos            Clear positive peak

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Tare" {
  run $GADC --tare
  [ "$status" -eq 0 ]
}

@test "Clear positive peak" {
  run $GADC --clear-pos
  [ "$status" -eq 0 ]
}

@test "Clear negative peak" {
  run $GADC --clear-neg
  [ "$status" -eq 0 ]
}

@test "Check pos peak == 0" {
  run $GADC --pos-peak
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Check neg peak == 0" {
  run $GADC --neg-peak
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Start measurement" {
  run $GADC --start
  [ "$status" -eq 0 ]
}

@test "Tare while measurement running" {
  run $GADC --tare
  [ "$status" -eq 0 ]
}

@test "Clear positive peak while measurement running" {
  run $GADC --clear-pos
  [ "$status" -eq 0 ]
}

@test "Clear negative peak while measurement running" {
  run $GADC --clear-neg
  [ "$status" -eq 0 ]
}

@test "Check pos peak == 0 while measurement running" {
  run $GADC --pos-peak
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Check neg peak == 0 while measurement running" {
  run $GADC --neg-peak
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}
