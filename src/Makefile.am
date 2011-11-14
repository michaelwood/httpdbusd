EXTRA_DIST =

bin_PROGRAMS = httpdbusd

httpdbusd_SOURCES = \
		httpdbusd.c \
		dbus-client.c dbus-client.h \
		http-server.c http-server.h

httpdbusd_CFLAGS = \
		-I$(top_srcdir) -Wall \
		$(HTTPDBUSD_CFLAGS) \
		$(HTTPDBUSD_DEPS_CFLAGS)

httpdbusd_LDADD = \
		$(HTTPDBUSD_DEPS_LIBS)

#
#%.service: %.service.in $(top_builddir)/config.log
#	$(AM_V_GEN)sed -e "s|\@dir\@|$(bindir)|" $< > $@
#
#servicedir = $(datadir)/dbus-1/services/
#service_in_files = mex-webremote.service.in
#service_DATA = mex-webremote.service

#CLEANFILES = $(service_DATA);

#EXTRA_DIST += $(service_in_files)
