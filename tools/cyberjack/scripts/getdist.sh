#!/bin/sh

RPM_COMMAND="`which rpm`"
DIST_NAME=""
DIST_VER=""

# check for DEB
if test -z "${DIST_NAME}"; then
  if test -f "/etc/debian_version"; then
    debtype=["` gcc --version | head -1`"]
    case $debtype in
      *\(Debian\ *)
        DIST_NAME="Debian"
        DIST_VER="`cat /etc/debian_version`"
        ;;
      *\(Ubuntu\ * | *\(KUbuntu\ *)
        DIST_NAME="Ubuntu"
        DIST_VER="`cat /etc/debian_version`"
        ;;
      *)
        DIST_NAME="Debian or derivate"
        DIST_VER="`cat /etc/debian_version`"
        ;;
     esac
  fi
fi


# check for RPM
if test -z "${DIST_NAME}"; then
  if test -n "${RPM_COMMAND}"; then
    if test -x "${RPM_COMMAND}"; then
      if test -e "/etc/mandriva-release"; then
        DIST_NAME="Mandriva"
        #DIST_VER="`rpm -q --queryformat='%{VERSION}' mandriva-release 2>/dev/null`"
        read v1 v2 v3 v4 v5 </etc/mandriva-release
        DIST_VER=$v4
      elif test -e "/etc/SuSE-release"; then
        DIST_NAME="SuSE"
        read v1 v2 v3 v4 </etc/SuSE-release
        case "$v2" in 
          *.*)
            DIST_VER=$v2
            ;;
          *)
            DIST_VER=$v3
            ;;
        esac
      elif test -e "/etc/fedora-release"; then
        DIST_NAME="Fedora Core"
        DIST_VER="`rpm -q --queryformat='%{VERSION}' fedora-release 2>/dev/null`"
      fi
    fi
  fi
fi


if test -z "${DIST_NAME}"; then
  DIST_NAME="(unbekannt)"
fi
if test -z "${DIST_VER}"; then
  DIST_VER="(unbekannt)"
fi

echo "${DIST_NAME}" >distname
echo "${DIST_VER}" >distver

