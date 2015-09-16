#!/usr/bin/env bats

## Tests gadc --state

GADC=../cli/gadc

@test "Stop, set mode = 0 (10Hz), clear peaks" {
  run $GADC --stop --set-mode 0 --set-upper-limit 100 --set-lower-limit -100 --clear-neg --clear-pos
  [ "$status" -eq 0 ]
}

@test "Check state" {
  run $GADC --state
  [ "$status" -eq 0 ]
  [ "${lines[0]}"  = "[ ] upper limit exceeded" ]
  [ "${lines[1]}"  = "[ ] lower limit underrun" ]
  [ "${lines[2]}"  = "[ ] peak  mode active" ]
  [ "${lines[3]}"  = "[ ] peak+ mode active" ]
  [ "${lines[4]}"  = "[ ] peak- mode active" ]
  [ "${lines[5]}"  = "[ ] Store to memory in progress" ]
  [ "${lines[6]}"  = "[ ] overload (abs(F) > 150%)" ]
  [ "${lines[7]}"  = "[X] fracture detected (only W20/W40)" ]
  [ "${lines[8]}"  = "[ ] mem active (P21=1 or P21=2)" ]
  [ "${lines[9]}"  = "[ ] mem-conti (store with displayrate)" ]
  [ "${lines[10]}" = "[ ] grenz_option" ]
  [ "${lines[11]}" = "[ ] measurement running" ]
}

@test "Set mode = 1 (900Hz peak), check state" {
  run $GADC --set-mode 1 --state
  [ "$status" -eq 0 ]
  [ "${lines[2]}"  = "[X] peak  mode active" ]
  [ "${lines[3]}"  = "[ ] peak+ mode active" ]
  [ "${lines[4]}"  = "[ ] peak- mode active" ]
}

@test "Set mode = 2 (900Hz peak+), check state" {
  run $GADC --set-mode 2 --state
  [ "$status" -eq 0 ]
  [ "${lines[2]}"  = "[X] peak  mode active" ]
  [ "${lines[3]}"  = "[X] peak+ mode active" ]
  [ "${lines[4]}"  = "[ ] peak- mode active" ]
}

@test "Set mode = 3 (900Hz peak-), check state, set mode 1" {
  run $GADC --set-mode 3 --state --set-mode 1
  [ "$status" -eq 0 ]
  [ "${lines[2]}"  = "[X] peak  mode active" ]
  [ "${lines[3]}"  = "[ ] peak+ mode active" ]
  [ "${lines[4]}"  = "[X] peak- mode active" ]
}

@test "Start measurement, check state" {
  run $GADC --start --state --stop
  [ "$status" -eq 0 ]
  [ "${lines[11]}" = "[X] measurement running" ]
}

## trigger limits
@test "Set lower limit = 10, start, check state, stop" {
  run $GADC --set-lower-limit 10 --start --state --stop
  [ "$status" -eq 0 ]
  [ "${lines[0]}"  = "[ ] upper limit exceeded" ]
  [ "${lines[1]}"  = "[X] lower limit underrun" ]
  [ "${lines[11]}" = "[X] measurement running" ]
}

@test "Set lower limit = -100, upper limit = -10, start, check state, stop" {
  run $GADC --set-upper-limit -10 --set-lower-limit -100 --clear-neg --clear-pos --start --state --stop
  [ "$status" -eq 0 ]
  [ "${lines[0]}"  = "[X] upper limit exceeded" ]
  [ "${lines[1]}"  = "[ ] lower limit underrun" ]
  [ "${lines[11]}" = "[X] measurement running" ]
}

@test "Restore limits" {
  run $GADC --set-upper-limit 100 --set-lower-limit -100 --clear-neg --clear-pos
  [ "$status" -eq 0 ]
}
