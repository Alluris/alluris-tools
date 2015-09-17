.PHONY: spellcheck

ASPELL = aspell -p ./aspell.en.pws -l en_US list| sort | uniq

spellcheck: $(TARGETS)
	cat ../README.md | $(ASPELL)
	cat ../liballuris/liballuris.c | $(ASPELL)
	cat ../liballuris/liballuris.h | $(ASPELL)
	cat ../cli/gadc.c | $(ASPELL)
	cat ../examples/fstream.c | $(ASPELL)
	cat ../examples/gadc.m | $(ASPELL)

