# $Id: xml2.m4,v 1.4 2006/01/18 09:44:19 cstim Exp $
# (c) 2009 Martin Preuss<martin@libchipcard.de>
# These functions search for FOX


AC_DEFUN([AQ_CHECK_XML2],[
dnl PREREQUISITES:
dnl   AQ_CHECK_OS must be called before this
dnl IN: 
dnl   All variables which already have a value will not be altered
dnl OUT:
dnl   Variables:
dnl     have_xml2 - set to "yes" if XML2 exists
dnl     libxml2_includes - path to includes
dnl     libxml2_libs - path to libraries
dnl   Defines:
dnl     HAVE_XML2

AC_MSG_CHECKING(if XML2 is allowed)
AC_ARG_ENABLE(xml2,
  [  --enable-xml2         enable xml2 (default=yes)],
  enable_xml2="$enableval",
  enable_xml2="yes")
AC_MSG_RESULT($enable_xml2)

if test "$enable_xml2" = "no"; then
   libxml2_libs=""
   libxml2_includes=""
   have_xml2="no"
else


dnl paths for xml2 includes
AC_MSG_CHECKING(for xml2 includes)
AC_ARG_WITH(xml2-includes, 
  [  --with-xml2-includes=DIR      uses xml2 includes from given dir],
  [local_libxml2_includes="$withval"],
  [local_libxml2_includes="\
        /usr/include/libxml2 \
        /usr/local/include/libxml2 \
        /opt/libxml2/include \
        "
  ]
)

if test -z "$libxml2_includes"; then
	for i in $local_libxml2_includes; do
		if test -z "$libxml2_includes"; then
                  if test -f "$i/libxml/tree.h"; then
                    libxml2_includes="-I$i"
                    break;
                  fi
 		fi
        done
fi
if test -n "$libxml2_includes"; then
	AC_MSG_RESULT($libxml2_includes)
else
	AC_MSG_RESULT(not found)
fi


# Check for x86_64 architecture; potentially set lib-directory suffix
if test "$target_cpu" = "x86_64"; then
  libdirsuffix="64"
else
  libdirsuffix=""
fi

dnl paths for xml2 libs
AC_MSG_CHECKING(for xml2 libraries)
AC_ARG_WITH(xml2-libs, 
  [  --with-xml2-libs=SPEC      uses given xml2 libs ],
  [libxml2_libs="$withval"],
  [libxml2_libs=""]
)



if test -z "$libxml2_libs"; then
     libxml2_libs="-lxml2"
fi

if test -n "$libxml2_libs"; then
	AC_MSG_RESULT($libxml2_libs)
else
	AC_MSG_RESULT(not found)
fi


# check if all necessary xml2 components where found
if test -z "$libxml2_includes" || \
   test -z "$libxml2_libs"; then
	libxml2_libs=""
   	libxml2_includes=""
   	have_xml2="no"
else
   have_xml2="yes"
   AC_DEFINE(HAVE_LIBXML2, 1, [whether XML2 is available])
fi


dnl end of if "$enable_xml2"
fi

AS_SCRUB_INCLUDE(libxml2_includes)
AC_SUBST(libxml2_libs)
AC_SUBST(libxml2_includes)

])
