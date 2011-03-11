noinst_PROGRAMS = simple-client

simple_client_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS)

simple_client_LDADD = \
	$(top_builddir)/eekboard/libeekboard.la \
	$(top_builddir)/eek/libeek.la \
	$(GIO2_LIBS)

simple_client_SOURCES = main.c
