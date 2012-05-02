DIST_SUBDIRS=cjflash cjgeldkarte cyberjack cjBingo fxcyberjack fox-dialog mac-ui-helper


if IS_OSX
  osx_subdirs=mac-ui-helper

if WITH_FOX
  fox_subdirs=fxcyberjack
endif

else
if WITH_FOX
  fox_subdirs=fxcyberjack fox-dialog
endif
endif


if ENABLE_NONSERIAL
SUBDIRS=cjflash cjgeldkarte cyberjack $(fox_subdirs) $(osx_subdirs)
else
SUBDIRS=cjgeldkarte $(fox_subdirs) $(osx_subdirs)
endif



