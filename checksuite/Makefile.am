
noinst_LTLIBRARIES=libchecksuite.la

DEFS+=-DCD_SCRIPT_DIR=\"$(libdir)/cyberjack\"

AM_CXXFLAGS=@cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK

INCLUDES=\
 -I../include \
 -I../cjeca32 \
 -I../ctapi -I../libcyberjack $(libsysfs_includes) $(hal_includes) $(pcsc_includes)

libchecksuite_la_SOURCES=\
  cm_distri.cpp \
  cm_driver.cpp \
  cm_group.cpp \
  cm_reader.cpp \
  cm_services.cpp \
  cm_uname.cpp \
  checkmodule.cpp \
  checksuite.cpp

noinst_HEADERS=\
  cm_distri.h \
  cm_driver.h \
  cm_group.h \
  cm_reader.h \
  cm_services.h \
  cm_uname.h \
  checkmodule.h \
  checksuite.h

