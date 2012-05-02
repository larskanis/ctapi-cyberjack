INCLUDES=-I../ctapi -I../include -I../cjeca32 -I../cjeca32/ausb $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

AM_CFLAGS=$(visibility_cflags)

noinst_LTLIBRARIES=libppa.la
libppa_la_SOURCES=\
  cjccid.c \
  cjdata.c \
  cjppUpdate.c \
  cjppPlatformLinux.c \
  cjppDebugOutLinux.c \
  SyncInterpreter.c \
  easy_dat.c

noinst_HEADERS=\
 ccid.h \
 cjpp.h \
 cjppa.h \
 cjppa_linux.h
 
