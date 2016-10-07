AM_CPPFLAGS = -I$(top_srcdir)/liballuris
AM_LDFLAGS  = -L$(top_srcdir)/liballuris

bin_PROGRAMS = gadc gadc_getopt

gadc_SOURCES = gadc.c
gadc_LDADD = ../liballuris/liballuris.la

gadc_getopt_SOURCES = gadc_getopt.c
gadc_getopt_LDADD = ../liballuris/liballuris.la
