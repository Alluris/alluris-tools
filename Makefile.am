ACLOCAL_AMFLAGS = -I m4
SUBDIRS = liballuris cli doc examples

style:
	find . \( -name "*.c" -or -name "*.h" \) -exec sed -i 's/[[:space:]]*$$//' {} \;
	find . \( -name "*.c" -or -name "*.h" \) -exec astyle --style=gnu -s2 -n {} \;

realclean:
	$(MAKE) maintainer-clean
	rm -rf m4 autom4te.cache
	rm -rf doc/html doc/doxygen_sqlite3.db
	rm -f aclocal.m4 ar-lib compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing
	find . -name "Makefile.in" -exec rm {} \;
	find . -name "octave-workspace" -exec rm {} \;
