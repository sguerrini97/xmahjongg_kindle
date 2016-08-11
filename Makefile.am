## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = foreign check-news

SUBDIRS = liblcdf src

man_MANS = xmahjongg.6

#
# Installation
#

LAYOUTS	= default bridge wedges ceremonial farandole arena theater \
	deepwell hourglass arrow papillon schoon \
	rat ox tiger hare dragon snake horse ram monkey rooster dog boar
TILESETS = thin.gif small.gif thick.gif gnome.gif gnome2.gif dorothys.gif \
	dorwhite.gif real.gif
BACKGROUNDS = default.gif green.gif

install-share:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/layouts $(DESTDIR)$(pkgdatadir)/tiles $(DESTDIR)$(pkgdatadir)/backgrounds
	for i in $(LAYOUTS); do \
	$(INSTALL_DATA) $(srcdir)/share/layouts/$$i $(DESTDIR)$(pkgdatadir)/layouts; done
	for i in $(TILESETS); do \
	$(INSTALL_DATA) $(srcdir)/share/tiles/$$i $(DESTDIR)$(pkgdatadir)/tiles; done
	for i in $(BACKGROUNDS); do \
	$(INSTALL_DATA) $(srcdir)/share/backgrounds/$$i $(DESTDIR)$(pkgdatadir)/backgrounds; done
install-data-local: install-share

uninstall-local:
	-cd $(DESTDIR)$(pkgdatadir)/layouts; rm -f $(LAYOUTS)
	-cd $(DESTDIR)$(pkgdatadir)/tiles; rm -f $(TILESETS)
	-cd $(DESTDIR)$(pkgdatadir)/backgrounds; rm -f $(BACKGROUNDS)
	-rmdir $(DESTDIR)$(pkgdatadir)/layouts $(DESTDIR)$(pkgdatadir)/tiles $(DESTDIR)$(pkgdatadir)/backgrounds
	-rmdir $(DESTDIR)$(pkgdatadir)


#
# Distribution
#

EXTRA_DIST = \
	xmahjongg.6 \
	xmahjongg.spec \
	include/lcdf/clp.h \
	include/lcdf/inttypes.h \
	include/lcdf/permstr.hh \
	include/lcdf/straccum.hh \
	include/lcdf/string.hh \
	include/lcdf/vector.hh include/lcdf/vector.cc \
	include/lcdfgif/gif.h \
	include/lcdfgif/gifx.h

dist-hook:
	mkdir $(distdir)/share
	mkdir $(distdir)/share/layouts
	mkdir $(distdir)/share/tiles
	mkdir $(distdir)/share/backgrounds
	for l in $(LAYOUTS); do cp -p $(srcdir)/share/layouts/$$l $(distdir)/share/layouts; done
	for t in $(TILESETS); do cp -p $(srcdir)/share/tiles/$$t $(distdir)/share/tiles; done
	for b in $(BACKGROUNDS); do cp -p $(srcdir)/share/backgrounds/$$b $(distdir)/share/backgrounds; done
	mkdir $(distdir)/images
	cp -p $(srcdir)/images/*.gif $(distdir)/images

rpm: dist
	buildarch=`rpm --showrc | awk '/^build arch/ { print $$4; }'` ; \
	mkdir -p /tmp/rpm-xmj/SOURCES /tmp/rpm-xmj/RPMS/$$buildarch \
	/tmp/rpm-xmj/BUILD ; \
	echo 'include: /usr/lib/rpm/rpmrc' > /tmp/rpm-xmj/rc ; \
	echo 'macrofiles: /usr/lib/rpm/macros:/tmp/rpm-xmj/macros' >> /tmp/rpm-xmj/rc ; \
	echo '%_topdir /tmp/rpm-xmj' > /tmp/rpm-xmj/macros ; \
	cp logo.gif $(PACKAGE)-$(VERSION).tar.gz /tmp/rpm-xmj/SOURCES ; \
	rpm --rcfile /tmp/rpm-xmj/rc -bb xmahjongg.spec ; \
	cp /tmp/rpm-xmj/RPMS/$$buildarch/*.rpm .
	rm -rf /tmp/rpm-xmj


#
# Maintenance
#

srclinks:
	test -d $(srcdir)/include || mkdir $(srcdir)/include
	test -d $(srcdir)/include/lcdf || mkdir $(srcdir)/include/lcdf
	test -d $(srcdir)/include/lcdfgif || mkdir $(srcdir)/include/lcdfgif
	cd $(srcdir); for i in clp ; do \
	ln -sf ../../liblcdf/liblcdf/$$i.c liblcdf/$$i.c ; \
	ln -sf ../../../liblcdf/include/lcdf/$$i.h include/lcdf/$$i.h ; done
	cd $(srcdir); for i in fixlibc.c vectorv.cc fmalloc.c ; do \
	ln -sf ../../liblcdf/liblcdf/$$i liblcdf/$$i ; done
	cd $(srcdir); for i in vector ; do \
	ln -sf ../../../liblcdf/include/lcdf/$$i.cc include/lcdf/$$i.cc ; \
	ln -sf ../../../liblcdf/include/lcdf/$$i.hh include/lcdf/$$i.hh ; done
	cd $(srcdir); for i in permstr straccum string ; do \
	ln -sf ../../../liblcdf/include/lcdf/$$i.hh include/lcdf/$$i.hh ; \
	ln -sf ../../liblcdf/liblcdf/$$i.cc liblcdf/$$i.cc ; done
	cd $(srcdir); for i in inttypes.h ; do \
	ln -sf ../../../liblcdf/include/lcdf/$$i include/lcdf/$$i ; done
	cd $(srcdir); for i in gif.h gifx.h ; do \
	ln -sf ../../../liblcdf/include/lcdfgif/$$i include/lcdfgif/$$i ; done
	cd $(srcdir); for i in giffunc.c gifread.c giftoc.c gifx.c ; do \
	ln -sf ../../liblcdf/liblcdfgif/$$i liblcdf/$$i ; done

versionize:
	perl -pi -e "s/^\\.ds V.*/.ds V $(VERSION)/;" $(top_srcdir)/xmahjongg.6
	perl -pi -e "s/^Version: .*/Version: $(VERSION)/; s/$(PACKAGE)-[\w.]+\.tar\.gz/$(PACKAGE)-$(VERSION).tar.gz/;" $(top_srcdir)/xmahjongg.spec

.PHONY: srclinks versionize rpm
