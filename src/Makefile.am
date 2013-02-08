plugindir = $(CLAWS_MAIL_PLUGINDIR)

plugin_LTLIBRARIES = rootmail.la

rootmail_la_SOURCES = \
	rootmail.c

rootmail_la_LDFLAGS = \
	-avoid-version -module

AM_CPPFLAGS = \
        $(CLAWS_MAIL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS)
