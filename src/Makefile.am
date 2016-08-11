## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = xmahjongg

xmahjongg_SOURCES = \
		alarm.hh alarm.cc \
		board.hh board.cc \
		button.hh button.cc \
		counter.hh counter.cc \
		game.hh game.cc \
		gmjts.hh gmjts.cc \
		hint.hh hint.cc \
		images.c \
		kdets.hh kdets.cc \
		kmjts.hh kmjts.cc \
		main.cc \
		matches.hh matches.cc \
		moment.hh moment.cc \
		panel.hh panel.cc \
		random.cc \
		solution.hh solution.cc \
		solvable.hh solvable.cc \
		swgeneral.hh swgeneral.cc \
		swwidget.hh swwidget.cc \
		tile.hh tile.cc \
		tileset.hh tileset.cc \
		traverse.hh traverse.cc \
		xmj3ts.hh xmj3ts.cc

BUILT_SOURCES = images.c

INCLUDES = $(X_CFLAGS) -I$(top_srcdir)/include

xmahjongg_LDADD = $(LDADD) ../liblcdf/liblcdf.a @X_LIBS@ @X_PRE_LIBS@ -lX11 @X_EXTRA_LIBS@


#
# Dependencies
#

IMAGES = $(top_srcdir)/images/buttons.gif $(top_srcdir)/images/digits.gif
MOSTLYCLEANFILES = images.c
images.c: $(IMAGES) ../liblcdf/giftoc
	../liblcdf/giftoc -extern -makename $(IMAGES) > $@


#
# Maintenance
#

srclinks:
	for i in clp.h clp.c fmalloc.c permstr.hh permstr.cc strerror.c \
	vector.hh vector.cc ; do \
	ln -sf $$HOME/src/edlib/$$i $$i ; done
	for i in gif.h giffunc.c gifread.c giftoc.c gifx.h gifx.c ; do \
	ln -sf $$HOME/src/giflib/$$i $$i ; done

versionize:
	perl -pi -e "s/^\\.ds V.*/.ds V $(VERSION)/;" xmahjongg.6
	perl -pi -e "s/^Version: .*/Version: $(VERSION)/; s/$(PACKAGE)-[\w.]+\.tar\.gz/$(PACKAGE)-$(VERSION).tar.gz/;" rpm.spec

.PHONY: srclinks versionize rpm
