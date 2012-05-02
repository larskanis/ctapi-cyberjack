INCLUDES=-I../../include -I../../libdialog -I../../libdialog/base -I../../cjeca32 -I../fxcyberjack  $(fox_includes) $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

bin_PROGRAMS=cyberjack-gui-agent
cyberjack_gui_agent_LDADD=\
  ../../libdialog/base/libdialog_base.la \
  @THREAD_LIBS@ -lobjc $(COCOA_FRAMEWORK)

cyberjack_gui_agent_SOURCES=\
 Delegate.m \
 PinDialog.m \
 main.m

launchddir=$(pkgdatadir)/Library/LaunchAgents
launchd_DATA=de.reiner-sct.cyberjack.gui.plist

EXTRA_DIST=$(launchd_DATA)


noinst_HEADERS= \
 Delegate.h \
 PinDialog.h

