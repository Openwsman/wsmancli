AM_CFLAGS = @CFLAGS@

CFLAGS = \
       $(OPENWSMAN_CFLAGS)
       
wsman_LDADD = \
       $(OPENWSMAN_LIBS) \
       -lwsman_curl_client_transport

wsman_SOURCES = wsman.c 

bin_PROGRAMS = \
	wsman
   


