SUBDIRS=icons diag

INCLUDES=-I../../include -I../../ctapi -I../../libcyberjack $(fox_includes) -I../../cjeca32 $(libxml2_includes) $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

bin_PROGRAMS=fxcyberjack
fxcyberjack_LDADD=\
  ../../libcyberjack/libcyberjack.la \
  diag/libfxdiag.la \
  ../../checksuite/libchecksuite.la @THREAD_LIBS@ $(fox_libs) \
  ../../cjeca32/libeca.la $(libxml2_libs)  -lstdc++
  
fxcyberjack_SOURCES=\
 fxcy_readerlist.cpp \
 fxcy_readerview.cpp \
 fxcy_mainwindow.cpp \
 fxcy_app.cpp \
 fxcy_readerdetails.cpp \
 fxcy_readerflash.cpp \
 fxcy_readerflash2.cpp \
 fxcy_readerkeys.cpp \
 main.cpp

noinst_HEADERS=\
 fxcy_readerlist.hpp \
 fxcy_readerview.hpp \
 fxcy_mainwindow.hpp \
 fxcy_readerdetails.hpp \
 fxcy_readerflash.hpp \
 fxcy_readerflash2.hpp \
 fxcy_readerkeys.hpp \
 fxcy_app.hpp

