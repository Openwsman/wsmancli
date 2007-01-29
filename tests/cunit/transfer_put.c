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


TestData put_tests[] = {
  {
    "Transfer Put without any selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    NULL,
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
    "Transfer Put with non existent Resource URI.",
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
    "Transfer Put with unsufficient selectors.",
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
    "Transfer Put with wrong selectors.",
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
    "Transfer Put with all selectors but with wrong values.",
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
    "Transfer Put with correct selectors and parameters check for new value",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData",
    NULL,
    "InstanceID=omc:timezone",
    "TimeZone=US/Pacific",
    "/s:Envelope/s:Body/p:OMC_TimeZoneSettingData/p:TimeZone",
    "US/Pacific",
    NULL,
    NULL,
    200,
    0
  },
  {
    "Transfer Put with correct selectors and parameters reset value",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData",
    NULL,
    "InstanceID=omc:timezone",
    "TimeZone=US/Eastern",
    "/s:Envelope/s:Body/p:OMC_TimeZoneSettingData/p:TimeZone",
    "US/Eastern",
    NULL,
    NULL,
    200,
    0,
  },
  {
    "Transfer Put with correct selectors and no parameters",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData",
    NULL,
    "InstanceID=omc:timezone",
    NULL,
    "/s:Envelope/s:Body/p:OMC_TimeZoneSettingData/p:TimeZone",
    "US/Eastern",
    NULL,
    NULL,
    200,
    0,
  },
};

static int ntests = sizeof (put_tests) / sizeof (put_tests[0]);



extern WsManClient *cl;
actionOptions options;

static void transfer_put_test() {
    WsXmlDocH doc;
    char *xpf = NULL;
    char *xpd = NULL;
    static int i = 0; // executed test number.
    char *selectors = NULL;


    if (put_tests[i].selectors) {
        selectors =
              u_strdup_printf(put_tests[i].selectors, host, host, host);
    }

    initialize_action_options(&options);

    if (put_tests[i].selectors != NULL) {
       wsman_add_selectors_from_query_string (&options, selectors);
    }
    if (put_tests[i].properties != NULL) {
       wsman_add_properties_from_query_string (&options,
                                               put_tests[i].properties);
    }
    options.flags = put_tests[i].flags;


    doc = ws_transfer_get_and_put(cl, (char *)put_tests[i].resource_uri, options);
    //ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
    CU_ASSERT_TRUE(wsman_client_get_response_code(cl) ==
                                               put_tests[i].final_status);
    if (wsman_client_get_response_code(cl) !=
                            put_tests[i].final_status) {
        if (verbose) {
            printf("\nExpected = %ld, Returned = %ld        ",
                           put_tests[i].final_status,
                           wsman_client_get_response_code(cl));
         //   ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(doc);
    if (!doc) {
        goto RETURN;
    }
    if (put_tests[i].expr1 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(put_tests[i].value1);
    if (put_tests[i].value1 == NULL) {
        goto RETURN;
    }
    xpf = ws_xml_get_xpath_value(doc, put_tests[i].expr1);
    CU_ASSERT_PTR_NOT_NULL(xpf);
    if (!xpf) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpf, put_tests[i].value1);

    if (strcmp(xpf, put_tests[i].value1)) {
        if (verbose) {
            printf("Expected %s\nReturned %s       ",
                     put_tests[i].value1, xpf);
        }
         goto RETURN;
    }
    if (put_tests[i].expr2 == NULL) {
        goto RETURN;
    }
    xpd = ws_xml_get_xpath_value(doc, put_tests[i].expr2);
    CU_ASSERT_PTR_NOT_NULL(xpd);
    if (!xpd) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(put_tests[i].value2);
    if (put_tests[i].value2 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpd, put_tests[i].value2 );
    if (strcmp(xpd, put_tests[i].value2)) {
        if (verbose) {
            printf("\nExpected %sReturned %s       ",
                     put_tests[i].value2, xpd);
        }
        goto RETURN;
    }
RETURN:
    u_free(xpf);
    u_free(xpd);
    if (doc) {
        ws_xml_destroy_doc(doc);
    }
    u_free(selectors);
    destroy_action_options(&options);
    i++; // increase executed test number
}




int add_transfer_put_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;
        /* add the tests to the suite */
    for (i =0; i < ntests; i++) {
            found_test += (NULL != CU_add_test(ps, put_tests[i].explanation,
                                            (CU_TestFunc)transfer_put_test));
    }
    return (found_test > 0);
}

