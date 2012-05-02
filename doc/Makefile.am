
DOCFILES=\
  README.ps README.html README.pdf README.txt \
  LIESMICH.ps LIESMICH.html LIESMICH.pdf LIESMICH.txt

#docfiledir=$(datarootdir)/doc/$(PACKAGE)
#docfile_DATA=$(DOCFILES)

manpagedir=$(mandir)
manpage_DATA= \
  cjflash.8 \
  cjgeldkarte.8 \
  cyberjack.8

EXTRA_DIST=\
  README.xml LIESMICH.xml \
  verifypin_ascii.c verifypin_fpin2.c \
  $(DOCFILES) $(manpage_DATA)


%.ps: %.xml
	jw -b ps $<

%.pdf: %.ps
	ps2pdf $<

%.html: %.xml
	jw -b html --nochunks $<
	tidy -i -m -q $@  || /bin/true

%.txt: %.xml
	jw -b txt $<

clean:

#CLEANFILES=$(DOCFILES)

