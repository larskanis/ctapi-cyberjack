SUBDIRS=scripts

DEFS+=@cyberjack_platform_def@ \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\" $(PTHREAD_CFLAGS)

INCLUDES=\
 -Ilib \
 -I../../ctapi \
 -I../../include \
 -I../../checksuite \
 -I../../cjeca32 $(pcsc_includes)

bin_PROGRAMS=cyberjack

cyberjack_SOURCES=cyberjack.cpp mt-test.cpp
cyberjack_LDADD=\
  ../../checksuite/libchecksuite.la -ldl @libusb_libs@ $(PTHREAD_LIBS) \
  ../../ctapi/libctapi-cyberjack.la \
  ../../libcyberjack/libcyberjack.la -lstdc++

