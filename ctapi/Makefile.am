
INCLUDES=\
 -I../include \
 -I../libdialog -I../libdialog/base \
 -I../cjeca32 \
 -I../cjeca32/ausb \
 -I../ecom \
 -I../ppa $(libsysfs_includes) $(pcsc_includes)

EXTRA_DIST=ctapi_special.cpp

AM_CFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"
  
AM_CPPFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"

lib_LTLIBRARIES=libctapi-cyberjack.la
libctapi_cyberjack_la_SOURCES=\
  ctapi.cpp \
  ctapi_beep.cpp


if ENABLE_NONSERIAL
libctapi_cyberjack_la_LIBADD=\
 ../cjeca32/libeca.la  \
 ../ecom/libecom.la \
 ../ppa/libppa.la \
 ../libdialog/libdialog_client.la \
 -lstdc++ -ldl $(libusb_LIBS) @THREAD_LIBS@ $(hal_libs) $(IOKIT_FRAMEWORK) $(FOUNDATION_FRAMEWORK)
else
libctapi_cyberjack_la_LIBADD=\
 ../cjeca32/libeca.la  \
 -lstdc++ -ldl @THREAD_LIBS@  $(IOKIT_FRAMEWORK) $(FOUNDATION_FRAMEWORK)
endif

libctapi_cyberjack_la_LDFLAGS=\
 -version-info @CYBERJACK_SO_CURRENT@:@CYBERJACK_SO_REVISION@:@CYBERJACK_SO_AGE@
 

noinst_HEADERS=\
  ctapi_beep.h

noinst_PROGRAMS=libtest

libtest_SOURCES=libtest.c
libtest_LDADD=libctapi-cyberjack.la -ldl -lstdc++
#libtest_LDFLAGS=-all-static
