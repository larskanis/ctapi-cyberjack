DIST_SUBDIRS=etc m4 include cjeca32 ecom ppa ctapi ifd libcyberjack checksuite tools debian doc libdialog

SUBDIRS=m4 include libdialog cjeca32

if ENABLE_NONSERIAL
SUBDIRS+=ecom ppa
endif

if IS_LINUX
SUBDIRS+=etc
endif


SUBDIRS+=ctapi ifd libcyberjack checksuite tools debian doc


EXTRA_DIST=\
  ctapi-cyberjack.spec.in ctapi-cyberjack.spec \
  COPYRIGHT.GPL COPYRIGHT.LGPL \
  ChangeLog ChangeLog.old

rpm:    ctapi-cyberjack.spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

#
# If the effective SO version of this library changes then you will need
# to copy 
#   debian/control-tmpl to debian/control 
# and 
#   debian/rules-tmpl to debian/rules
#
deb:
	dpkg-buildpackage -rfakeroot

ChangeLog:
	git log --pretty="format:------------------------------------------------------------------%n%ai %an%n%s%n%b" >ChangeLog


#dist-hook:
#	rm -f ChangeLog 2>/dev/null
#	$(MAKE) ChangeLog

