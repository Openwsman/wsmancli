/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-client-api.h"
#include "wsman-client-transport.h"
#include "common.h"


TestData get_tests[] = {
  {
    "Transfer Get without any selectors.",						// explanation
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 		// resource_uri
    NULL,										// method
    NULL,										// selectors (as URI key=value&key=value ...)
    NULL,										// properties (as URI key=value&key=value ...)
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",				// expr1
    "wsman:InvalidSelectors",	    							// value1
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",				// expr2
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",	// value2
    400, 										// final_status
    0,											// flags
    0											// max_elements
											//   char **filters;  char **common_filters;
  },

  {
    "Transfer Get with non existent Resource URI.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
    NULL,
    NULL, 
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",  
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    400, 
    0,
    0
  },

  {
    "Transfer Get with unsufficient selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "Name=%s",
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",
    400,
    0,
    0
  },

  {
    "Transfer Get with wrong selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Namex=%s",
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/UnexpectedSelectors",
    400,
    0,
    0
  },

  {
    "Transfer Get with all selectors but with wrong values.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=%sx",
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    400,
    0,
    0
  },
  {
    "Transfer Get with all selectors (CIM_OperatingSystem).",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_OperatingSystem",
    NULL,
    "CSCreationClassName=OMC_UnitaryComputerSystem&CSName=%s"
                     "&CreationClassName=OMC_OperatingSystem&Name=Linux",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    200,
    0,
    0
  },
  {
    "Transfer Get with correct selectors. Check response code",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=%s",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    200,
    0,
  },
};

static int ntests = sizeof (get_tests) / sizeof (get_tests[0]);



extern WsManClient *cl;
client_opt_t *options;

static void transfer_get_test() {
    WsXmlDocH doc;
    char *xpf = NULL;
    char *xpd = NULL;
    static int i = 0; // executed test number.
    char *old_selectors = get_tests[i].selectors;


    if (get_tests[i].selectors) {
        get_tests[i].selectors =
              u_strdup_printf(get_tests[i].selectors, host, host, host);
    }

    wsman_client_reinit_conn(cl);
    options = wsman_client_options_init();

    if (get_tests[i].selectors != NULL) {
       wsman_add_selectors_from_query_string (options, get_tests[i].selectors);
    }


    doc = wsman_client_action_get(cl, (char *)get_tests[i].resource_uri, options);
    CU_ASSERT_TRUE(wsman_client_get_response_code(cl) == get_tests[i].final_status);

    CU_ASSERT_PTR_NOT_NULL(doc);
    if (!doc) {
        goto RETURN;
    }

    if (get_tests[i].expr1 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(get_tests[i].value1);
    if (get_tests[i].value1 == NULL) {
        goto RETURN;
    }
    xpf = ws_xml_get_xpath_value(doc, get_tests[i].expr1);
    CU_ASSERT_PTR_NOT_NULL(xpf);
    if (!xpf) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpf, get_tests[i].value1);

    if (strcmp(xpf, get_tests[i].value1)) {
         goto RETURN;
    }
    if (get_tests[i].expr2 == NULL) {
        goto RETURN;
    }
    xpd = ws_xml_get_xpath_value(doc, get_tests[i].expr2);
    CU_ASSERT_PTR_NOT_NULL(xpd);
    if (!xpd) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(get_tests[i].value2);
    if (get_tests[i].value2 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpd, get_tests[i].value2 );
    if (strcmp(xpd, get_tests[i].value2)) {
         goto RETURN;
    }

RETURN:
    u_free(xpf);
    u_free(xpd);
    if (doc) {
        ws_xml_destroy_doc(doc);
    }
    u_free((char *)get_tests[i].selectors);
    get_tests[i].selectors = old_selectors;
    wsman_client_options_destroy(options);
    i++; // increase executed test number
}




int add_transfer_get_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;
        /* add the tests to the suite */
    for (i =0; i < ntests; i++) {
            found_test += (NULL != CU_add_test(ps, get_tests[i].explanation,
                                            (CU_TestFunc)transfer_get_test));
    }
    return (found_test > 0);
}

