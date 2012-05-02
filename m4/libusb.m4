# $Id: libusb.m4 31 2005-01-26 00:54:58Z aquamaniac $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for libusb

AC_DEFUN([AC_LIBUSB], [
dnl searches for libusb
dnl Arguments: none
dnl Returns:   libusb_dir
dnl            libusb_libs
dnl            libusb_includes
dnl            have_libusb

AC_MSG_CHECKING(if libusb support desired)
AC_ARG_ENABLE(libusb,
  [  --enable-libusb      enable libusb support (default=yes)],
  enable_libusb="$enableval",
  enable_libusb="yes")
AC_MSG_RESULT($enable_libusb)

have_libusb="no"
libusb_dir=""
libusb_libs=""
libusb_includes=""
if test "$enable_libusb" != "no"; then
  AC_MSG_CHECKING(for libusb)

  AC_ARG_WITH(libusb-dir, [  --with-libusb-dir=DIR
                            uses libusb from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/libusb-config"; then
          libusb_dir="$li";
          break
      fi
  done
  if test -z "$libusb_dir"; then
      AC_MSG_RESULT([not found ])
  else
      AC_MSG_RESULT($libusb_dir)
      AC_MSG_CHECKING(for USB libs)
      if test -n "${LIBUSB_LIBS}"; then
        libusb_libs="${LIBUSB_LIBS}"
      else
        libusb_libs="`$libusb_dir/bin/libusb-config --libs`"
      fi
      AC_MSG_RESULT($libusb_libs)

      AC_MSG_CHECKING(for USB includes)
      if test -n "${LIBUSB_INCLUDES}"; then
        libusb_includes="${LIBUSB_INCLUDES}"
      else
        libusb_includes="`$libusb_dir/bin/libusb-config --cflags`"
      fi
      if test -z "$libusb_includes"; then
        AC_MSG_RESULT([<none needed>])
      else
        AC_MSG_RESULT($libusb_includes)
      fi
      have_libusb="yes"
  fi
dnl end of "if enable-libusb"
fi

AC_SUBST(libusb_dir)
AC_SUBST(libusb_libs)
AC_SUBST(libusb_includes)
])
