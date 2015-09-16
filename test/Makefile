.PHONY:check
.PHONY:all
.PHONY:single
.PHONY:fcn_check

all: fcn_check
	bats *.bats

single: fcn_check
	bats gadc_digits.bats
	bats gadc_digout.bats
	bats gadc_limits.bats
	bats gadc_mem_mode.bats
	bats gadc_mode.bats
	bats gadc_peaks.bats
	bats gadc_unit.bats
	bats gadc_various.bats
	bats gadc_state.bats

fcn_check:
	grep -H "@test" *.bats | sort | uniq -d
