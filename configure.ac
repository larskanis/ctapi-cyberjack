#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# y

###-------------------------------------------------------------------------
#
# The following line makes sure that this file is always touched when a new
# revision is checked out.
#
#
# $Revision$
#

# info
CYBERJACK_BUGREPORT_ADDR="<martin@libchipcard.de>"


AC_PREREQ(2.61)
AC_INIT
AC_CANONICAL_TARGET([])
AC_CONFIG_SRCDIR([cjeca32/BaseCommunication.cpp])
AC_CONFIG_HEADERS([config.h])

###-------------------------------------------------------------------------
#
# versions
#
CYBERJACK_VERSION_MAJOR=3
CYBERJACK_VERSION_MINOR=3
CYBERJACK_VERSION_PATCHLEVEL=6
CYBERJACK_VERSION_BUILD=0
dnl "stable", "rcX", "betaX", "cvs"
CYBERJACK_VERSION_TAG="stable"

CYBERJACK_VERSION_FULL_STRING="$CYBERJACK_VERSION_MAJOR.$CYBERJACK_VERSION_MINOR.$CYBERJACK_VERSION_PATCHLEVEL-${CYBERJACK_VERSION_TAG}-r${CYBERJACK_VERSION_BUILD}"
CYBERJACK_VERSION_STRING="$CYBERJACK_VERSION_MAJOR.$CYBERJACK_VERSION_MINOR.$CYBERJACK_VERSION_PATCHLEVEL"


###-------------------------------------------------------------------------
#
# SO version for cyberjack library
#
CYBERJACK_SO_CURRENT="5"
CYBERJACK_SO_AGE="3"
CYBERJACK_SO_REVISION="6"
CYBERJACK_SO_EFFECTIVE="`echo \$(($CYBERJACK_SO_CURRENT-$CYBERJACK_SO_AGE))`"



###-------------------------------------------------------------------------
#
# Create release strings
#
case "$CYBERJACK_VERSION_TAG" in
  cvs|svn|git)
     CYBERJACK_VERSION_RELEASE_STRING="$CYBERJACK_VERSION_MAJOR.$CYBERJACK_VERSION_MINOR.$CYBERJACK_VERSION_PATCHLEVEL.r${CYBERJACK_VERSION_BUILD}"
     ;;
  stable)
     CYBERJACK_VERSION_RELEASE_STRING="$CYBERJACK_VERSION_MAJOR.$CYBERJACK_VERSION_MINOR.$CYBERJACK_VERSION_PATCHLEVEL"
     ;;
  *)
     CYBERJACK_VERSION_RELEASE_STRING="$CYBERJACK_VERSION_MAJOR.$CYBERJACK_VERSION_MINOR.$CYBERJACK_VERSION_PATCHLEVEL${CYBERJACK_VERSION_TAG}"
     ;;
esac


AC_SUBST(CYBERJACK_VERSION_MAJOR)
AC_SUBST(CYBERJACK_VERSION_MINOR)
AC_SUBST(CYBERJACK_VERSION_PATCHLEVEL)
AC_SUBST(CYBERJACK_VERSION_BUILD)
AC_SUBST(CYBERJACK_VERSION_TAG)
AC_SUBST(CYBERJACK_VERSION_FULL_STRING)
AC_SUBST(CYBERJACK_VERSION_STRING)
AC_SUBST(CYBERJACK_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_MAJOR,$CYBERJACK_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_MINOR,$CYBERJACK_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_PATCHLEVEL,$CYBERJACK_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_BUILD,$CYBERJACK_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_STRING,"$CYBERJACK_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_FULL_STRING,"$CYBERJACK_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(CYBERJACK_VERSION_TAG, "$CYBERJACK_VERSION_TAG", [tag])


AC_SUBST(CYBERJACK_SO_CURRENT)
AC_SUBST(CYBERJACK_SO_REVISION)
AC_SUBST(CYBERJACK_SO_AGE)
AC_SUBST(CYBERJACK_SO_EFFECTIVE)

AM_INIT_AUTOMAKE(ctapi-cyberjack, $CYBERJACK_VERSION_RELEASE_STRING)



###-------------------------------------------------------------------------
# check for OS

AQ_CHECK_OS


###-------------------------------------------------------------------------
# prefix handling

AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)



###-------------------------------------------------------------------------
#
# static/shared handling
#
AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)


###-------------------------------------------------------------------------
#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_CXX
AC_PROG_OBJC
AC_PROG_INSTALL

AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_CHECK_PROG(SED,sed,sed)

ACX_PTHREAD


###-------------------------------------------------------------------------
#
# Search for dlopen
#
LIBS=""
AC_SEARCH_LIBS(dlopen, [c dl], [have_dlopen="yes"], [have_dlopen="no"])
if test "x${have_dlopen}" != "xyes"; then
  AC_MSG_ERROR([
*** You are missing support for dynamic loading of modules.
*** Therefore cjgeldkarte will not work.])
else
  AC_DEFINE(WITH_DLLIB, 1, [whether DL library is available])
  AC_DEFINE(HAS_DLOPEN, 1, [whether function dlopen is available])
  if test "$LIBS" = "-lc"; then
      LIBS=""
  fi
  dl_libs="$LIBS"
  LIBS=""
fi
AC_SUBST(dl_libs)





###-------------------------------------------------------------------------
#
# Checks for header files.
#
AC_HEADER_STDC
#AC_CHECK_HEADERS([regex.h])
AC_CHECK_HEADERS([pthread.h])


###-------------------------------------------------------------------------
#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST


###-------------------------------------------------------------------------
#
# Checks for library functions.
#


###-------------------------------------------------------------------------
#
# OS dependant settings
#

cyberjack_ldflags="-L\${libdir}"
cyberjack_libs="-lcyberjack"
cyberjack_internal_libname="libctapi-cyberjack.la"
cyberjack_headerdir_am="\$(includedir)"
cyberjack_headerdir="\${includedir}"
cyberjack_config_file="\${sysconfdir}/cyberjack.conf"

case "$OS_TYPE" in
    windows) cyberjack_sys_is_windows=1 ;;
          *) cyberjack_sys_is_windows=0 ;;
esac

if test "$OSYSTEM" = "osx"; then
  COCOA_FRAMEWORK="-framework Cocoa"
  FOUNDATION_FRAMEWORK="-framework CoreFoundation"
  SYSCONF_FRAMEWORK="-framework SystemConfiguration"
  IOKIT_FRAMEWORK="-framework IOKit"
  cyberjack_config_file="/etc/cyberjack.conf"
fi

AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])
AM_CONDITIONAL(IS_OSX, [test "$OSYSTEM" = "osx"])
AM_CONDITIONAL(IS_LINUX, [test "$OSYSTEM" = "linux"])

AC_SUBST(cyberjack_sys_is_windows)
AC_SUBST(cyberjack_config_file)
AC_SUBST(cyberjack_platform_def)
AC_SUBST(COCOA_FRAMEWORK)
AC_SUBST(FOUNDATION_FRAMEWORK)
AC_SUBST(SYSCONF_FRAMEWORK)
AC_SUBST(IOKIT_FRAMEWORK)



###-------------------------------------------------------------------------
#
# check for disable-nonserial
#
AC_MSG_CHECKING(whether non-serial access is wanted)
AC_ARG_ENABLE(nonserial,
  [  --enable-nonserial         enable non-serial (default=yes)],
  [ case "${enableval}" in
    yes) enable_nonserial="yes";;
    no)  enable_nonserial="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-nonserial);;
    esac
  ],
  enable_nonserial="yes")

if test "$enable_nonserial" = "yes"; then
 AC_DEFINE(ENABLE_NONSERIAL, 1, [whether non-serial interfaces should be enabled])
fi
AM_CONDITIONAL(ENABLE_NONSERIAL, [test "$enable_nonserial" = "yes"])
AC_MSG_RESULT($enable_nonserial)
AC_SUBST(enable_nonserial)



if test "$enable_nonserial" = "yes"; then

  ###-------------------------------------------------------------------------
  #
  # libUSB-1.0
  #
  
  pkg_modules="libusb-1.0 >= 1.0.0"
  PKG_CHECK_MODULES(libusb, [$pkg_modules], [have_libusb="yes"], [have_libusb="no"])
  if test "$have_libusb" = "yes"; then
    AC_DEFINE(USE_USB1, 1, [if libUSB-1.0 is available])
    libusb_includes="$libusb_CFLAGS"
    libusb_libs="$libusb_LIBS"
  else

    ###-------------------------------------------------------------------------
    #
    # libusb-0.x
    #

    pkg_modules="libusb < 1.0.0"
    PKG_CHECK_MODULES(libusb, [$pkg_modules], [have_libusb="yes"], [have_libusb="no"])
    if test "$have_libusb" = "yes"; then
      libusb_includes="$libusb_CFLAGS"
      libusb_libs="$libusb_LIBS"
    else
      AC_MSG_ERROR([
  *** LibUSB not found, please install the packages libusb and
  *** libusb-devel (or libusb-dev on Debian).])
    fi
  fi
  
  AC_SUBST(libusb_includes)
  AC_SUBST(libusb_libs)
  

  ###-------------------------------------------------------------------------
  #
  # HAL
  #
  
  AC_MSG_CHECKING(if HAL support desired)
  AC_ARG_ENABLE(hal, [  --enable-hal      enable HAL support (default=yes)],
                enable_hal="$enableval",
                enable_hal="yes")
  AC_MSG_RESULT($enable_hal)

  if test "$enable_hal" = "yes"; then
    pkg_modules="hal >= 0.5.0"
    PKG_CHECK_MODULES(hal, [$pkg_modules], [have_hal="yes"], [have_hal="no"])
    if test "$have_hal" = "yes"; then
      AC_DEFINE(HAVE_HAL, 1, [if HAL is available])
      hal_includes="$hal_CFLAGS"
      hal_libs="$hal_LIBS"
    fi
  fi
fi

AC_SUBST(hal_libs)
AC_SUBST(hal_includes)



# ---------------------------------------------------------------------------
# check for PC/SC
#

AQ_CHECK_PCSC
AM_CONDITIONAL(USE_PCSC, [test "$have_pcsc" = "yes"])
if test "$have_pcsc" != "yes"; then
  pcsc_includes="-I\$(top_srcdir)/include -I\$(top_srcdir)/include/PCSC"
else
  AC_DEFINE(HAVE_PCSC, 1, [if PC/SC is available])
fi


case "$OSYSTEM" in
  osx)
  	cyberjack_ifd_filename="libifd-cyberjack.dylib"
        cyberjack_ifd_dropdir="/usr/libexec/SmartCardServices/drivers"
        # use our own PC/SC headers for MacOSX
        pcsc_includes="-I\$(top_srcdir)/include/macosx -I\$(top_srcdir)/include/macosx/PCSC"
        ;;
  linux)
  	cyberjack_ifd_filename="libifd-cyberjack.so"
        cyberjack_ifd_dropdir="\$(libdir)/pcsc/drivers"
        ;;
esac

AC_ARG_WITH(usbdropdir,
  [  --with-usbdropdir       pcsc bundle location],
  [ usbdropdir="${withval}" ],
  [ usbdropdir="${cyberjack_ifd_dropdir}" ]
)

AC_SUBST(cyberjack_ifd_filename)
AC_SUBST(usbdropdir)



# ---------------------------------------------------------------------------
# check for LibXML2
#

AQ_CHECK_XML2
AM_CONDITIONAL(USE_XML2, [test "$have_libxml2" = "yes"])



###-------------------------------------------------------------------------
# Check for fox toolkit
#

AQ_CHECK_FOX
AM_CONDITIONAL(WITH_FOX, [test "$have_fox" = "yes"])


###-------------------------------------------------------------------------
#
# check for release
#
AC_MSG_CHECKING(whether this is an official release)
AC_ARG_ENABLE(release,
  [  --enable-release         make this an official release (default=no)],
  [ case "${enableval}" in
    yes) enable_release="yes";;
    no)  enable_release="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-release);;
    esac
  ],
  enable_release="no")

if test "$enable_release" = "yes"; then
  STRIPALL="-s"
else
  STRIPALL=""
fi
AC_SUBST(STRIPALL)
AC_MSG_RESULT($enable_release)



###-------------------------------------------------------------------------
#
# check for pthread-based approach
#
AC_MSG_CHECKING(whether the threaded approach is wanted)
AC_ARG_ENABLE(threads,
  [  --enable-threads         enable threads (default=no)],
  [ case "${enableval}" in
    yes) enable_threads="yes";;
    no)  enable_threads="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-threads);;
    esac
  ],
  enable_threads="no")

if test "$enable_threads" = "yes"; then
  AC_DEFINE_UNQUOTED(USE_THREADS, 1, [whether pthreads are to be used])
  THREAD_LIBS="-lpthread"
else
  THREAD_LIBS=""
fi
AC_SUBST(USE_THREADS)
AC_SUBST(THREAD_LIBS)
AC_MSG_RESULT($enable_threads)



###-------------------------------------------------------------------------
#
# check whether udev support is wanted
#

AC_ARG_ENABLE(udev,
  [  --enable-udev         create udev files],
  [ ENABLE_UDEV="${enableval}" ],
  [ ENABLE_UDEV="yes" ]
)



###-------------------------------------------------------------------------
#
# GCC version (check for usability)
#
AC_MSG_CHECKING(if symbol visibility is supported)
AC_ARG_ENABLE(visibility,
  [  --enable-visibility             enable visibility (default=no)],
  enable_visibility="$enableval",
  enable_visibility="yes")
AC_MSG_RESULT($enable_visibility)

if test "$enable_visibility" = "yes"; then
  AC_MSG_CHECKING(if symbol visibility is supported)
  visibility_cflags=""
  case `$CC --version|head -n1|cut -f1 -d " "` in 
    gcc | gcc-* | *-gcc | *-gcc-*)
       dnl Determine version number (watch out: the outside square
       dnl parenteses are vitally important here!)
       gccversion=["` ${CC} -dumpversion `"]
       case $gccversion in
         4.*)
           # Activate -fvisibility=hidden only for explicit 4.x gcc
           # versions. We never now what future gcc's might do with
           # that argument, and we also can't be sure that other
           # gcc's happen to show very weird version
           # numbers. Therefore leave the default at non-activated.
           visibility_supported="yes, gcc-${gccversion}"
           AC_DEFINE(GCC_WITH_VISIBILITY_ATTRIBUTE, 1, [visibility])
           visibility_cflags="-fvisibility=hidden"
           ;;
         *)
           # Default: -fvisibility=hidden not activated.
           visibility_supported="no (needs gcc >=4.0, you have gcc-${gccversion})"
           ;;
        esac
        ;;
    *)
      visibility_supported="no (needs gcc >=4.0, you use $CC)"
      ;;
  esac
  
  # -fvisibility is at least not available on MinGW/gcc-3.4.4 (will
  # give an "unrecognized cmdline option"). Also, unfortunately I
  # don't know an easy way to ask the compiler here. Maybe
  # http://autoconf-archive.cryp.to/ax_cflags_gcc_option.html
  case "$OS_TYPE" in
    windows)
          visibility_supported="no (not yet on MinGW/Windows)"
          visibility_cflags=""
          ;;
    *)
          ;;
  esac
else
  visibility_supported="no (not wanted)"
  visibility_cflags=""
fi


AC_MSG_RESULT(${visibility_supported})
AC_SUBST(visibility_cflags)



###-------------------------------------------------------------------------
#
# Name of udev script
#
AC_MSG_CHECKING(for name of udev script for Debian derivate)
UDEV_SCRIPT_NAME=99-cyberjack.rules
UDEV_RULES_FILE="rules.new"
case `basename $CC` in 
  gcc | gcc-* | *-gcc | *-gcc-*)
     debtype=["` ${CC} --version | head -1`"]
     case $debtype in
       *\(Debian\ *)
         UDEV_SCRIPT_NAME=z80_cyberjack.rules
         ;;
       *\(Ubuntu\ * | *\(KUbuntu\ *)
         UDEV_SCRIPT_NAME=99-cyberjack.rules
         ;;
       *)
         ;;
      esac
      ;;
  *)
    ;;
esac
echo $UDEV_SCRIPT_NAME > udev_script_name
AC_MSG_RESULT($UDEV_SCRIPT_NAME)
AC_SUBST(UDEV_SCRIPT_NAME)


###-------------------------------------------------------------------------
#
# RPM distribution specific stuff

AC_MSG_CHECKING(whether this is an RPM distribution)
RPM_COMMAND="`which rpm`"
RPM_DIST_NAME=""
RPM_DIST_TAG=""
RPM_DIST_VER=""
if test "$ENABLE_UDEV" = "yes"; then
  ETC_SUBDIRS="udev"
else
  ETC_SUBDIRS=""
fi
USE_HOTPLUG=0
USE_UDEV=0
UDEV_RULES_FILE="rules.new"
RPM_PCSC_DROPDIR="%{_libdir}/readers"

if test -n "${RPM_COMMAND}"; then
  if test -x "${RPM_COMMAND}"; then
    if test -e "/etc/mandriva-release"; then
      RPM_DIST_NAME="mandriva"
      RPM_DIST_TAG="mdk"
#      RPM_DIST_VER="`rpm -q --queryformat='%{VERSION}' mandriva-release 2>/dev/null`"
      read v1 v2 v3 v4 v5 </etc/mandriva-release
      RPM_DIST_VER=$v4
    elif test -e "/etc/SuSE-release"; then
      RPM_DIST_NAME="suse"
      RPM_DIST_TAG="suse"
      read v1 v2 v3 v4 </etc/SuSE-release
      case "$v2" in 
      	*.*)
          RPM_DIST_VER=$v2
          ;;
        *)
          RPM_DIST_VER=$v3
          ;;
      esac
    elif test -e "/etc/fedora-release"; then
      RPM_DIST_NAME="fedora"
      RPM_DIST_TAG="fc"
      RPM_DIST_VER="`rpm -q --queryformat='%{VERSION}' fedora-release 2>/dev/null`"
    fi
    if test -n "${RPM_DIST_TAG}"; then
      case "${RPM_DIST_TAG}${RPM_DIST_VER}" in
        suse9.*)
          ETC_SUBDIRS="resmgr"
          RPM_PCSC_DROPDIR="%{_libdir}/readers"
          ;;
        suse10.0)
          ETC_SUBDIRS="udev"
          USE_UDEV=1
          RPM_PCSC_DROPDIR="%{_libdir}/readers"
          ;;
        suse10.1)
          ETC_SUBDIRS="hal"
          RPM_PCSC_DROPDIR="%{_libdir}/readers"
          ;;
        suse11.*)
          ETC_SUBDIRS="udev"
          USE_UDEV=1
          RPM_PCSC_DROPDIR="%{_libdir}/readers"
          ;;
        fc4)
          ETC_SUBDIRS="hotplug"
          USE_HOTPLUG=1
          RPM_PCSC_DROPDIR="%{_libdir}/pcsc/drivers"
          ;;
        fc5)
 	  UDEV_RULES_FILE="rules.fc5"
          ETC_SUBDIRS="udev"
          USE_UDEV=1
          RPM_PCSC_DROPDIR="%{_libdir}/pcsc/drivers"
          ;;
        fc6)
          ETC_SUBDIRS="udev"
          USE_UDEV=1
          RPM_PCSC_DROPDIR="%{_libdir}/pcsc/drivers"
          ;;
        *)
          ETC_SUBDIRS="udev"
          USE_UDEV=1
          RPM_PCSC_DROPDIR="%{_libdir}/readers"
          ;;
      esac
      AC_MSG_RESULT([Yes, ${RPM_DIST_NAME} ${RPM_DIST_VER}])
    else
      AC_MSG_RESULT([no (RPM found, but unknown distribution)])
    fi
  else
    AC_MSG_RESULT([no (RPM not found)])
  fi
else
  AC_MSG_RESULT([no (no RPM installed)])
fi

case "${RPM_DIST_TAG}${RPM_DIST_VER}" in 
  suse10.*)
    AC_DEFINE_UNQUOTED(USE_SUSE_HACK, "1", [needed for SuSE])
    ;;
esac


AC_SUBST(RPM_DIST_NAME)
AC_SUBST(RPM_DIST_TAG)
AC_SUBST(RPM_DIST_VER)
AC_SUBST(RPM_PCSC_DROPDIR)

AC_SUBST(ETC_SUBDIRS)
AC_SUBST(USE_HOTPLUG)
AC_SUBST(USE_UDEV)
AC_SUBST(UDEV_RULES_FILE)



###-------------------------------------------------------------------------
#
# Debug arguments for compilation
#

ACX_COMPILE_WARN()


###-------------------------------------------------------------------------
#
# compile program flagss
#

# Clean up include variables
AS_SCRUB_INCLUDE(cyberjack_includes)
AS_SCRUB_INCLUDE(all_includes)


###-------------------------------------------------------------------------
#
# substitute vars
#
AC_SUBST(cyberjack_ldflags)
AC_SUBST(cyberjack_libs)
AC_SUBST(cyberjack_internal_libname)
AC_SUBST(cyberjack_pkgincludedir)
AC_SUBST(cyberjack_plugindir)
AC_SUBST(cyberjack_headerdir_am)
AC_SUBST(cyberjack_headerdir)


###-------------------------------------------------------------------------
#
# list of files to create
#
AC_CONFIG_FILES([Makefile
                 cjeca32/Makefile
                 cjeca32/ausb/Makefile
                 ecom/Makefile
                 ppa/Makefile
                 ctapi/Makefile
                 ifd/Makefile
                 ifd/Info.plist
                 libcyberjack/Makefile
                 libdialog/Makefile
                 libdialog/base/Makefile
                 checksuite/Makefile
                 tools/Makefile
                 tools/cjflash/Makefile
                 tools/cjgeldkarte/Makefile
                 tools/cyberjack/Makefile
                 tools/cyberjack/scripts/Makefile
                 tools/cjBingo/Makefile
                 tools/fxcyberjack/Makefile
                 tools/fxcyberjack/icons/Makefile
                 tools/fxcyberjack/diag/Makefile
                 tools/fox-dialog/Makefile
                 tools/mac-ui-helper/Makefile
                 include/Makefile
                 include/macosx/Makefile
                 include/macosx/PCSC/Makefile
                 m4/Makefile
                 etc/Makefile
                 etc/hal/Makefile
                 etc/hotplug/Makefile
                 etc/resmgr/Makefile
                 etc/udev/Makefile
                 ctapi-cyberjack.spec
                 debian/Makefile
                 debian/control-tmpl
                 debian/rules-tmpl
                 doc/Makefile
                 ])
AC_OUTPUT

