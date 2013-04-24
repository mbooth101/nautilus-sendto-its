plugindir = $(libdir)/nautilus-sendto/plugins

INCLUDES = $(NAUTILUS_SENDTO_CFLAGS) $(SOUP_CFLAGS) $(KEYRING_CFLAGS) $(GCONF_CFLAGS)

plugin_LTLIBRARIES = libnstits.la

libnstits_la_SOURCES=trac-attachment.c
libnstits_la_LDFLAGS = -module -avoid-version
libnstits_la_LIBADD = $(NAUTILUS_SENDTO_LIBS) $(SOUP_LIBS) $(KEYRING_LIBS) $(GCONF_LIBS)
