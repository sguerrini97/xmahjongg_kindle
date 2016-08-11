## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = liblcdf.a
noinst_PROGRAMS = giftoc

liblcdf_a_SOURCES = clp.c \
	fmalloc.c \
	giffunc.c gifread.c gifx.c \
	permstr.cc \
	straccum.cc \
	string.cc \
	vectorv.cc
EXTRA_liblcdf_a_SOURCES = fixlibc.c

giftoc_SOURCES = giftoc.c

## liblcdf_a_DEPENDENCIES = @FIXLIBC_O@
## liblcdf_a_LIBADD = @FIXLIBC_O@

INCLUDES = $(X_CFLAGS) -I$(top_srcdir)/include
