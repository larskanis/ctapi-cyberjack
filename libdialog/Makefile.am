SUBDIRS=base

INCLUDES=\
 -I../include \
 -I../ctapi \
 -I../cjeca32 \
 -I../cjeca32/ausb \
 -I../ecom \
 -Ibase \
 -I../ppa $(libsysfs_includes) $(pcsc_includes)


AM_CFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"
  
AM_CPPFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"

noinst_LTLIBRARIES=libdialog_client.la
libdialog_client_la_SOURCES=dialog.c
libdialog_client_la_LIBADD=base/libdialog_base.la $(FOUNDATION_FRAMEWORK) $(SYSCONF_FRAMEWORK)

noinst_HEADERS=\
  dialog.h\
  dialog_p.h


noinst_PROGRAMS=libtest
libtest_SOURCES=libtest.cpp
libtest_LDADD=libdialog_client.la

