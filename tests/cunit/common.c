


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
  wsman_client_transport_init(cl, NULL);
  return 0;
}


int clean_test(void) {
  wsman_release_client(cl);
  wsman_client_transport_fini();
  return 0;
}


void check_response_header(WsXmlDocH doc, long resp_code, char *action) {
    char *xp = NULL;

    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:To");
    CU_ASSERT_PTR_NOT_NULL(xp);
    if (xp != NULL) {
        CU_ASSERT_STRING_EQUAL(xp,
           "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");
    }
    u_free(xp);

    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:Action");
    CU_ASSERT_PTR_NOT_NULL(xp);
    if (xp != NULL) {
        if (resp_code == 200 && action != NULL) {
            CU_ASSERT_STRING_EQUAL(xp, action);
        } else if (resp_code == 500) {
            CU_ASSERT_STRING_EQUAL(xp,
              "http://schemas.xmlsoap.org/ws/2004/08/addressing/fault");
        }
    }
    u_free(xp);
    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:MessageID");
    CU_ASSERT_PTR_NOT_NULL(xp);
    u_free(xp);
    xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Header/wsa:RelatesTo");
    CU_ASSERT_PTR_NOT_NULL(xp);
    u_free(xp);
}



void handle_filters(WsXmlDocH doc, char *f[])
{
    int j;
    char *xp = NULL;

    if (f == NULL) {
        return;
    }
    for (j = 0; f[j] != NULL && f[j + 1] != NULL; j += 2) {
        if (f[j] == NULL) {
            continue;
        }
        char *val;
        u_free(xp);
        xp = ws_xml_get_xpath_value(doc, f[j]);
        CU_ASSERT_PTR_NOT_NULL(xp);
        if (xp == NULL) {
            if (verbose) {
                printf("\n No Xpath: %s      ", f[j]);
            }
            continue;
        }
        if (f[j + 1]) {
            val = u_strdup_printf(f[j + 1], host);
            CU_ASSERT_STRING_EQUAL(xp, val);
            if (verbose && strcmp(xp, val)) {
               printf("\nExpected:  %s\nReturned:  %s       ", val, xp);
            }
            u_free(val);
        }
    }
}



