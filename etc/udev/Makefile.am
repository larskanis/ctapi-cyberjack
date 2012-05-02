

udevdir=/etc/udev/rules.d

udscriptdir=/sbin
udscript_SCRIPTS=udev.cyberjack.sh

install-data-local:
	test -z "$(udevdir)" || $(mkdir_p) "$(DESTDIR)$(udevdir)"
	echo " Installing udev rules file \"@UDEV_RULES_FILE@\""
	$(install_sh_DATA) @UDEV_RULES_FILE@ $(DESTDIR)$(udevdir)/$(UDEV_SCRIPT_NAME)

uninstall-local:
	echo " Uninstalling udev rules file \"@UDEV_SCRIPT_NAME@\""
	rm -f $(DESTDIR)$(udevdir)/$(UDEV_SCRIPT_NAME)


EXTRA_DIST=\
 rules.fc5 \
 rules.new \
 udev.cyberjack.sh

