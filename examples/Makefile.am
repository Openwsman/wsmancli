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

new_api_example_SOURCES = \
		new_api_example.c

new_api_transfer_create_SOURCES = \
		new_api_transfer_create.c

new_api_invoke_SOURCES = \
		new_api_invoke.c

new_api_transfer_get_put_SOURCES = \
		new_api_transfer_get_put.c
		
noinst_PROGRAMS = \
	wsmid_identify \
	win32_service \
	enum_epr \
	create_resource \
	serialize \
	new_api_example \
	new_api_transfer_create \
	new_api_invoke \
	new_api_transfer_get_put


