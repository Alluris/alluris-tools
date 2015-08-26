#!/usr/bin/env bats

## Test "--set-unit" and "--get-unit" in gadc
##
## Devices with F_Max > 10N support N, Kg, lb
## Devices with F_Max <= 10N support N, cN, g, oz


GADC=../cli/gadc

@test "INIT: stopping measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "INIT: --set-unit=N" {
  run $GADC --set-unit=N
  [ "$status" -eq 0 ]
}


####### cN

@test "Check --set-unit=cN, compare with result of --get-unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    skip "F_max > 10N"
  fi

  run $GADC --set-unit=cN
  [ "$status" -eq 0 ]

  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "cN" ]
}

@test "Try to set unit 'cN', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -le 10 ]; then
    skip "F_max <= 10N"
  fi

  run $GADC --set-unit=cN
  [ "$status" -eq 4 ]
}

####### g

@test "Check --set-unit=g, compare with result of --get-unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    skip "F_max > 10N"
  fi

  run $GADC --set-unit=g
  [ "$status" -eq 0 ]

  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "g" ]
}

@test "Try to set unit 'g', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -le 10 ]; then
    skip "F_max <= 10N"
  fi

  run $GADC --set-unit=g
  [ "$status" -eq 4 ]
}
####### oz

@test "Check --set-unit=oz, compare with result of --get-unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    skip "F_max > 10N"
  fi

  run $GADC --set-unit=oz
  [ "$status" -eq 0 ]

  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "oz" ]
}

@test "Try to set unit 'oz', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -le 10 ]; then
    skip "F_max <= 10N"
  fi

  run $GADC --set-unit=oz
  [ "$status" -eq 4 ]
}
####### kg

@test "Check --set-unit=kg, compare with result of --get-unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -le 10 ]; then
    skip "F_max <= 10N"
  fi

  run $GADC --set-unit=kg
  [ "$status" -eq 0 ]

  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "kg" ]
}

@test "Try to set unit 'kg', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    skip "F_max > 10N"
  fi

  run $GADC --set-unit=kg
  [ "$status" -eq 4 ]
}
####### lb

@test "Check --set-unit=lb, compare with result of --get-unit" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -le 10 ]; then
    skip "F_max <= 10N"
  fi

  run $GADC --set-unit=lb
  [ "$status" -eq 0 ]

  run $GADC --get-unit
  [ "$status" -eq 0 ]
  [ "$output" == "lb" ]
}

@test "Try to set unit 'lb', check for LIBALLURIS_OUT_OF_RANGE" {
  run $GADC --fmax
  [ "$status" -eq 0 ]

  if [ "$output" -gt 10 ]; then
    skip "F_max > 10N"
  fi

  run $GADC --set-unit=lb
  [ "$status" -eq 4 ]
}

##### default N

@test "FINALIZE: --set-unit=N" {
  run $GADC --set-unit=N
  [ "$status" -eq 0 ]
}
