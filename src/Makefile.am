## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(pkgdatadir)/ui
ui_DATA = gtk_telegraph.ui


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GTK_TELEGRAPH_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = gtk_telegraph

gtk_telegraph_SOURCES = \
	main.c \
	gtk_telegraph.h \
	gtk_telegraph.c \
	controller.c \
	controller.h	


gtk_telegraph_LDFLAGS =

gtk_telegraph_LDADD = $(GTK_TELEGRAPH_LIBS)


if PLATFORM_WIN32
gtk_telegraph_LDFLAGS += -Wl,--export-all-symbols
else
gtk_telegraph_LDFLAGS += -Wl,--export-dynamic
endif

if NATIVE_WIN32
gtk_telegraph_LDFLAGS += -mwindows
endif


EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)

