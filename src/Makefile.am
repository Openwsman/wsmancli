AM_CFLAGS = @CFLAGS@

wsman_CFLAGS = \
	       $(OPENWSMAN_CFLAGS)
       
wsman_LDADD = \
       $(OPENWSMAN_LIBS) -lcurl

wsmid_identify_CFLAGS = \
	       $(OPENWSMAN_CFLAGS)
       
wsmid_identify_LDADD = \
       $(OPENWSMAN_LIBS) -lcurl

wsman_SOURCES = wsman.c \
		wsman-client-options.c \
		wsman-client-options.h 

wsmid_identify_SOURCES = \
		wsmid_identify.c
		
bin_PROGRAMS = \
	wsman \
	wsmid_identify
   


