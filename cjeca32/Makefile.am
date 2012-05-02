DIST_SUBDIRS=ausb

if ENABLE_NONSERIAL
SUBDIRS=ausb
endif


INCLUDES=-I../include -Iausb $(pcsc_includes)


AM_CFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"
  
AM_CPPFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"


noinst_HEADERS=\
  BaseCommunication.h \
  BaseReader.h \
  BaseTranslater.h \
  CCIDReader.h \
  EC30Reader.h \
  ECAReader.h \
  ECFReader.h \
  ECPReader.h \
  ECRReader.h \
  ECBReader.h \
  ECMReader.h \
  EFBReader.h \
  RSCTCriticalSection.h \
  USBUnix.h \
  SerialUnix.h \
  SECReader.h \
  ccid.h \
  config_l.h config_p.h \
  ntstatus.h \
  stdafx.h \
  version.h \
  PCSC10.h

noinst_LTLIBRARIES=libeca.la
libeca_la_SOURCES=\
  BaseCommunication.cpp \
  BaseReader.cpp \
  BaseTranslater.cpp \
  CCIDReader.cpp \
  Debug.cpp \
  EC30Reader.cpp \
  ECAReader.cpp \
  ECFReader.cpp \
  ECPReader.cpp \
  ECRReader.cpp \
  ECBReader.cpp \
  ECMReader.cpp \
  EFBReader.cpp \
  RSCTCriticalSection.cpp \
  Reader.cpp \
  USBUnix.cpp \
  SerialUnix.cpp \
  SECReader.cpp \
  cjeca32.cpp \
  Platform_unix.cpp \
  config.cpp


if ENABLE_NONSERIAL
libeca_la_LIBADD=ausb/libausb.la 
endif


