#!/usr/bin/env bats

## test digout, this also works when measurement is running

GADC=../cli/gadc

@test "INIT: start measurement" {
  run $GADC --start
  [ "$status" -eq 0 ]
}

@test "Set digout = 0, get digout and compare" {
  run $GADC --set-digout 0 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

@test "Set digout = 1, get digout and compare" {
  run $GADC --set-digout 1 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "Set digout = 2, get digout and compare" {
  run $GADC --set-digout 2 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 2 ]
}

@test "Set digout = 3, get digout and compare" {
  run $GADC --set-digout 3 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 3 ]
}

@test "Set digout = 4, get digout and compare" {
  run $GADC --set-digout 4 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 4 ]
}

@test "Set digout = 5, get digout and compare" {
  run $GADC --set-digout 5 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 5 ]
}

@test "Set digout = 6, get digout and compare" {
  run $GADC --set-digout 6 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 6 ]
}

@test "Set digout = 7, get digout and compare" {
  run $GADC --set-digout 7 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 7 ]
}

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Set digout = 0, get digout and compare again" {
  run $GADC --set-digout 0 --get-digout
  [ "$status" -eq 0 ]
  [ "$output" -eq 0 ]
}

### check for errors

@test "Try to set digout -1, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-digout -1
  [ "$status" -eq 4 ]
}

@test "Try to set digout 8, check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --set-digout 8
  [ "$status" -eq 4 ]
}


