INCLUDES=-I. -I.. -I$(top_srcdir)/include -I$(top_srcdir)/ctapi $(libusb_includes) $(hal_includes)

AM_CFLAGS=@cyberjack_platform_def@ -DBUILDING_CYBERJACK $(visibility_cflags)
AM_CPPFLAGS=@cyberjack_platform_def@ -DBUILDING_CYBERJACK $(visibility_cflags)

EXTRA_DIST=usbdev_hal.c usbdev_libusb1.c


noinst_LTLIBRARIES=libausb.la
libausb_la_SOURCES=\
 ausb_libusb0.c \
 ausb_libusb1.c \
 ausb.c \
 ausb1.c \
 ausb11.c \
 ausb3.c \
 ausb31.c \
 usbdev.c

noinst_HEADERS=\
 ausb_libusb0_l.h \
 ausb_libusb1_l.h \
 ausb_l.h \
 usbdev_l.h

libausb_la_LIBADD=$(IOKIT_FRAMEWORK) $(FOUNDATION_FRAMEWORK) $(libusb_LIBS) $(hal_libs)
