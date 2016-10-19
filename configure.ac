AC_INIT([liballuris], [0.2.1], [software@alluris.de], [https://github.com/alluris])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([liballuris/liballuris.c])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT
AC_DISABLE_STATIC
AC_PROG_CC

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])

# Checks for header files.
AC_CHECK_HEADERS([stdio.h stdlib.h string.h libusb-1.0/libusb.h])

AC_CHECK_LIB([usb-1.0], [libusb_open],,
  [AC_MSG_ERROR(["Error: Required library usb-1.0 not found. Install the usb-1.0 development package and try again"])])

CFLAGS+=" -Wall -Wextra"

AC_CONFIG_FILES([Makefile
                doc/Makefile
                liballuris/Makefile
                cli/Makefile
                examples/Makefile])
AC_OUTPUT

