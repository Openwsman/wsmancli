


#include "wsman_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"
#include "wsman-client-transport.h"

#include "common.h"


WsManClient *cl;



int init_test(void) {
  wsman_client_transport_init(NULL);

  ServerData sd[] = {
    {"localhost", 8889, "/wsman", "http", "wsman", "secret"}
  };

  cl = wsman_create_client( 
		      sd[0].server,
		      sd[0].port,
		      sd[0].path,
		      sd[0].scheme,
		      sd[0].username,
		      sd[0].password);
  return 0;
}


int clean_test(void) {
  wsman_release_client(cl);
  wsman_client_transport_fini();
  return 0;
}


void check_response_header(WsXmlDocH doc, char *action) {
    char *xp = NULL;

    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:To");
    CU_ASSERT_PTR_NOT_NULL(xp);
    if (xp != NULL) {
        CU_ASSERT_STRING_EQUAL(xp,
           "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");
    }
    u_free(xp);
    /*
    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:Action");
    CU_ASSERT_PTR_NOT_NULL(xp);
    if (xp != NULL && action != NULL) {
        CU_ASSERT_STRING_EQUAL(xp, action);
    }
    u_free(xp);
    */
    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:MessageID");
    CU_ASSERT_PTR_NOT_NULL(xp);
    u_free(xp);
    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:RelatesTo");
    CU_ASSERT_PTR_NOT_NULL(xp);
    u_free(xp);
}
