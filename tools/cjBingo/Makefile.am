INCLUDES=-I../../include -I../../cjeca32

DEFS+=@cyberjack_platform_def@

bin_PROGRAMS=cjBingoTest
cjBingoTest_SOURCES=\
	cjBingoTest.cpp \
	cjBingo.cpp
cjBingoTest_LDADD=../../cjeca32/libeca.la @libusb_libs@ @THREAD_LIBS@


