if HAVE_DOXYGEN
directory = $(top_srcdir)/doc/man/man3/

dist_man_MANS = $(directory)/liballuris.c.3 $(directory)/liballuris.h.3
$(directory)/liballuris.c.3: doxyfile.stamp
$(directory)/liballuris.h.3: doxyfile.stamp

doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_srcdir)/doc/man
endif 
