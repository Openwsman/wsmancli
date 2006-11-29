AM_CFLAGS = @CFLAGS@

wsman_LDADD = \
       $(OPENWSMAN_LIBS)

CFLAGS = \
       $(OPENWSMAN_CFLAGS)
       
LDADD = \
       $(OPENWSMAN_LIBS)

wsman_SOURCES = wsman.c \
		wsman-client-options.c \
		wsman-client-options.h 

wsmid_identify_SOURCES = \
		wsmid_identify.c

wsman_service_SOURCES = \
		win32_service.c
		
bin_PROGRAMS = \
	wsman \
	wsmid_identify \
	win32_service
   


