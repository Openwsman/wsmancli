AM_CFLAGS = @CFLAGS@

CFLAGS = \
       $(OPENWSMAN_CFLAGS) \
       -DPACKAGE_BUILDTS=\"$(PACKAGE_BUILDTS)\"
       
wsman_LDADD = \
       $(OPENWSMAN_LIBS) \
       -lwsman_curl_client_transport
wseventmgr_LDADD = \
	$(wsman_LDADD)

wsman_SOURCES = wsman.c
wseventmgr_SOURCES = wseventmgr.c 

bin_PROGRAMS = \
	wsman \
	wseventmgr

