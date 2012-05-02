INCLUDES=-I../../include -I../../cjeca32 $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

bin_PROGRAMS=cjflash
cjflash_SOURCES=cjflash.cpp
cjflash_LDADD=../../cjeca32/libeca.la @libusb_libs@ $(hal_libs) @THREAD_LIBS@ -lstdc++


