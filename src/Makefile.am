plugindir = $(libdir)/nautilus-sendto/plugins

INCLUDES = $(NAUTILUS_SENDTO_CFLAGS) $(SOUP_CFLAGS) $(KEYRING_CFLAGS)

plugin_LTLIBRARIES = libnsttrac.la

libnsttrac_la_SOURCES=trac-attachment.c
libnsttrac_la_LDFLAGS = -module -avoid-version
libnsttrac_la_LIBADD = $(NAUTILUS_SENDTO_LIBS) $(SOUP_LIBS) $(KEYRING_LIBS)
