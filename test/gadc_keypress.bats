#!/usr/bin/env bats

## Test gadc --keypress
##
## Bit 0=S1, 1=S2, 2=S3, 3=long_press.

GADC=../cli/gadc

@test "INIT: stopping measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "INIT: set-unit=N" {
  run $GADC --set-unit=N
  [ "$status" -eq 0 ]
}

@test "S1 starts measurement" {
  # wait until auto-tara has completed
  run $GADC --keypress 1 --sleep 1000 --state
  [ "$status" -eq 0 ]
  [ "${lines[11]}" = "[X] measurement running" ]
}

@test "S1 stops measurement" {
  run $GADC --keypress 1 --state
  [ "$status" -eq 0 ]
  [ "${lines[11]}" = "[ ] measurement running" ]
}

## Devices with F_Max > 10N support N, Kg, lb (so they should switch to kg after N)
## Devices with F_Max <= 10N support N, cN, g, oz (so they should switch to cN after N)

@test "S3 long clears display, then change unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    run $GADC --keypress 12 --keypress 4
    [ "$status" -eq 0 ]

    run $GADC --get-unit
    [ "$status" -eq 0 ]
    [ "$output" == "kg" ]
  else
    run $GADC --keypress 12 --keypress 4
    [ "$status" -eq 0 ]

    run $GADC --get-unit
    [ "$status" -eq 0 ]
    [ "$output" == "cN" ]
  fi
}
