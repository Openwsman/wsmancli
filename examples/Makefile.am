AM_CFLAGS = @CFLAGS@

CFLAGS = \
       $(OPENWSMAN_CFLAGS)
       
LDADD = \
       $(OPENWSMAN_LIBS) \
       -lwsman_curl_client_transport

wsmid_identify_SOURCES = \
		wsmid_identify.c

win32_service_SOURCES = \
		win32_service.c
enum_epr_SOURCES = \
		enum_epr.c
		
bin_PROGRAMS = \
	wsmid_identify \
	win32_service \
	enum_epr
   


