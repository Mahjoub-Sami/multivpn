bin_PROGRAMS = multivpn
#man_MANS = multivpn.1
#EXTRA_DIST = $(man_MANS)
multivpn_SOURCES = main.c debug.c globals.c tcp_plugin.c parser.c plugin.c tuntap.c generictunel.c sip_plugin.c base64.c
multivpn_CFLAGS = $(LIBPJPROJECT_CFLAGS)
multivpn_LDADD = $(LIBPJPROJECT_LIBS)
