.PHONY:all
.PHONY:fcn_check

all: fcn_check
	-bats gadc_digits.bats
	-bats gadc_digout.bats
	-bats gadc_limits.bats
	-bats gadc_mem_capture.bats
	-bats gadc_mem_mode.bats
	-bats gadc_mode.bats
	-bats gadc_peaks.bats
	-bats gadc_unit.bats
	-bats gadc_state.bats
	-bats gadc_keypress.bats
	-bats gadc_autostop.bats
	# various has to be least because it performs a power down
	-bats gadc_various.bats

fcn_check:
	## check for a problem in bats where duplicate test strings cause problems
	## See https://github.com/sstephenson/bats/issues/125
	grep -H "@test" *.bats | sort | uniq -d
