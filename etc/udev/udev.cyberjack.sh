#!/bin/sh

# This indirection is needed for 32-Bit packages installed on a
# 64-Bit SuSE10.0 (maybe on others as well)

if test -n "${DEVICE}"; then
  /bin/chgrp "cyberjack" ${DEVICE}
  /bin/chmod g+rw ${DEVICE}
fi

# This is needed when using the cyberjack kernel module
if test -n "${DEVNAME}"; then
  /bin/chgrp "cyberjack" ${DEVNAME}
  /bin/chmod g+rw ${DEVNAME}
fi
