INCLUDES=-I../ctapi -I../include -I../cjeca32 -I../cjeca32/ausb $(pcsc_includes)

DEFS+=@cyberjack_platform_def@

AM_CFLAGS=$(visibility_cflags)

noinst_LTLIBRARIES=libecom.la
libecom_la_SOURCES=\
 cjctapi.c \
 cjt1.c \
 cjio.c \
 cjio_user.c

noinst_HEADERS=\
 cj.h \
 cjctapi.h \
 cjio.h \
 cjio_user.h \
 cjt1.h \
 ctapi-ecom.h
 
