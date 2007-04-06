SUBDIRS =  cpp

AM_CFLAGS = @CFLAGS@

CFLAGS = \
       $(OPENWSMAN_CFLAGS)
       
LDADD = \
       $(OPENWSMAN_LIBS) \
       -lwsman_curl_client_transport

wsmid_identify_SOURCES = wsmid_identify.c

win32_service_SOURCES = \
		win32_service.c
enum_epr_SOURCES = \
		enum_epr.c
	
serialize_SOURCES = serialize.c
	
create_resource_SOURCES = \
		create_resource.c

session_example_SOURCES = \
		session_example.c

session_transfer_create_SOURCES = \
		session_transfer_create.c

session_invoke_SOURCES = \
		session_invoke.c

session_transfer_get_put_SOURCES = \
		session_transfer_get_put.c
		
noinst_PROGRAMS = \
	wsmid_identify \
	win32_service \
	enum_epr \
	create_resource \
	serialize \
	session_example \
	session_transfer_create \
	session_invoke \
	session_transfer_get_put


