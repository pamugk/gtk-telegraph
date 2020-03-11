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
	client.c \
	client.h \
	serverDataModel.h \
	gtk_telegraph.c \
	gtk_telegraph.h	


gtk_telegraph_LDFLAGS = \
	-Wl,--export-dynamic

gtk_telegraph_LDADD = $(GTK_TELEGRAPH_LIBS)




EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)

