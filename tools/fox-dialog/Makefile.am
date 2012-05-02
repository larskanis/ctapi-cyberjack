INCLUDES=-I../../include -I../../libdialog -I../../libdialog/base -I../../cjeca32 -I../fxcyberjack  $(fox_includes) $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

bin_PROGRAMS=cyberjack-gui
cyberjack_gui_LDADD=\
  ../../libdialog/base/libdialog_base.la \
  @THREAD_LIBS@ $(fox_libs) -lstdc++
  
cyberjack_gui_SOURCES=\
 dlg.cpp \
 main.cpp

noinst_HEADERS= \
 dlg.hpp


