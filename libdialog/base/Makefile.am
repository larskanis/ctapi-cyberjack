
INCLUDES=\
 -I../../include \
 -I../../ctapi \
 -I../../cjeca32 \
 -I../../cjeca32/ausb \
 -I../../ecom \
 -I../../ppa $(libsysfs_includes) $(pcsc_includes)


AM_CFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"
  
AM_CPPFLAGS=$(visibility_cflags) @cyberjack_platform_def@ \
  -DBUILDING_CYBERJACK \
  -DCYBERJACK_CONFIG_FILE=\"@cyberjack_config_file@\"

noinst_LTLIBRARIES=libdialog_base.la
libdialog_base_la_SOURCES=\
  message.c \
  network.c


noinst_HEADERS=\
  message.h \
  network.h

