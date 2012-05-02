
EXTRA_DIST=Info.plist.in PkgInfo part10.c

AM_CFLAGS=$(visibility_cflags)
AM_CPPFLAGS=$(visibility_cflags)


noinst_HEADERS=\
  ctbcs.h \
  pcscdefines.h \
  part10_l.h

if USE_PCSC

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/ctapi -I.. -I../cjeca32 $(pcsc_includes)

bundledir=$(usbdropdir)/libifd-cyberjack.bundle/Contents
bundle_DATA=Info.plist PkgInfo

if IS_LINUX
bundlelinuxdir=$(usbdropdir)/libifd-cyberjack.bundle/Contents/Linux
bundlelinux_LTLIBRARIES=libifd-cyberjack.la
endif

if IS_OSX
bundleosxdir=$(usbdropdir)/libifd-cyberjack.bundle/Contents/MacOS
bundleosx_LTLIBRARIES=libifd-cyberjack.la
endif

libifd_cyberjack_la_SOURCES=ifdhandler.c
libifd_cyberjack_la_LDFLAGS=-version-info @CYBERJACK_SO_CURRENT@:@CYBERJACK_SO_REVISION@:@CYBERJACK_SO_AGE@
libifd_cyberjack_la_LIBADD=$(top_builddir)/ctapi/libctapi-cyberjack.la

else

EXTRA_DIST += $(ifd_cyberjack_la_SOURCES) $(noinst_HEADERS)

endif
