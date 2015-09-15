.PHONY:check

check: fcn_check
	bats gadc_digits.bats
	bats gadc_digout.bats
	bats gadc_limits.bats
	bats gadc_mem_mode.bats
	bats gadc_mode.bats
	bats gadc_peaks.bats
	bats gadc_unit.bats
	bats gadc_various.bats

fcn_check:
	grep -H "@test" *.bats | sort | uniq -d
