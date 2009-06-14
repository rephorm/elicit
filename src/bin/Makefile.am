MAINTAINERCLEANFILES = Makefile.in

INCLUDES = @ELICIT_CFLAGS@ -Wall

bin_PROGRAMS = elicit

elicit_SOURCES = \
main.c \
elicit.c elicit.h \
config.c config.h \
color.c color.h \
palette.c palette.h \
grab.c grab.h \
zoom.c zoom.h \
band.c band.h \
types.h \
$INCLUDES

elicit_LDADD = @ELICIT_LIBS@

