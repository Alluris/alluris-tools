#!/usr/bin/env bats

## Tests gadc
## FIMXE: please also check
##   --read-memory
##   --delete-memory
##   --get-stats
##   --get-mem-count
## For example enable 2=continuous, apply some force, read memory, calculate
## and compare statistics, delete memory

GADC=../cli/gadc

@test "Stop measurement" {
  run $GADC --stop
  [ "$status" -eq 0 ]
}

@test "Stop, set mem mode = 1 (single), delete memory and start measurement" {
  run $GADC --stop --set-mem-mode 1 --delete-memory --start --sleep 1000
  [ "$status" -eq 0 ]
}

@test "Capture to memory with S2" {
  ## FIXME: this should also work without --sleep
  run $GADC --keypress 2 --sleep 1000 --keypress 2 --sleep 1000
  run $GADC --get-mem-count
  [ "$status" -eq 0 ]
  [ "$output" -eq 2 ]
}


