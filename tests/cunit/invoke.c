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
#include "wsman_config.h"

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


TestData invoke_tests[] = {
  {
    "Custom Method without any selectors.",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService",
    "ServiceStatus",
    NULL,
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",	    
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",
    500, 
    0,
    0
  },

  {
    "Custom Method with non existent Resource URI.",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdServicex",
    "ServiceStatus",
    NULL, 
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",  
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    500, 
    0,
    0
  },

  {
    "Custom Method without parameters. (StopService)",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService",
    "StopService",
    "SystemCreationClassName=OMC_UnitaryComputerSystem&SystemName=%s&Name=postfix",
    NULL,
    "/s:Envelope/s:Body/n1:StopService_OUTPUT/ReturnValue",
    "0",
    NULL,
    NULL,
    200,
    0,
    0
  },
  {
    "Custom Method without parameters. (ServiceStatus)",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService",
    "ServiceStatus",
    "SystemCreationClassName=OMC_UnitaryComputerSystem&SystemName=%s&Name=postfix",
    NULL,
    "/s:Envelope/s:Body/n1:ServiceStatus_OUTPUT/ReturnValue",
    "3",
    NULL,
    NULL,
    200,
    0,
    0
  },
  {
    "Custom Method without parameters. (StartService)",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService",
    "StartService",
    "SystemCreationClassName=OMC_UnitaryComputerSystem&SystemName=%s&Name=postfix",
    NULL,
    "/s:Envelope/s:Body/n1:StartService_OUTPUT/ReturnValue",
    "0",
    NULL,
    NULL,
    200,
    0,
    0
  } /*,

  {
    "Custom Method with wrong selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Namex=%s",
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/UnexpectedSelectors",
    500,
    0,
    0
  },

  {
    "Custom Method with all selectors but with wrong values.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL,
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=%sx",
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    500,
    0,
    0
  },
  {
    "Custom Method with correct selectors and parameters check for new value",
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData",
    NULL,
    "InstanceID=omc:timezone",
    "TimeZone=US/Pacific",
    "/s:Envelope/s:Body/p:OMC_TimeZoneSettingData/p:TimeZone",
    "US/Pacific",
    NULL,
    NULL,
    200,
    FLAG_DUMP_REQUEST,
  },
  {
    "Custom Method with correct selectors and parameters reset value",
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
  }, */
};

static int ntests = sizeof (invoke_tests) / sizeof (invoke_tests[0]);



extern WsManClient *cl;
actionOptions options;

static void invoke_test() {
    WsXmlDocH doc;
    char *xpf = NULL;
    char *xpd = NULL;
    static int i = 0; // executed test number.
    char *old_selectors = invoke_tests[i].selectors;


    if (invoke_tests[i].selectors) {
        invoke_tests[i].selectors =
              u_strdup_printf(invoke_tests[i].selectors, host, host, host);
    }

    reinit_client_connection(cl);
    initialize_action_options(&options);

    if (invoke_tests[i].selectors != NULL) {
       wsman_add_selectors_from_query_string (&options, invoke_tests[i].selectors);
    }
    if (invoke_tests[i].properties != NULL) {
       wsman_add_properties_from_query_string (&options, invoke_tests[i].properties);
    }
    options.flags = invoke_tests[i].flags;

    doc = wsman_invoke (cl, (char *)invoke_tests[i].resource_uri, (char *)invoke_tests[i].method, options);
    //ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
    CU_ASSERT_TRUE(wsman_get_client_response_code(cl) == invoke_tests[i].final_status);

    CU_ASSERT_PTR_NOT_NULL(doc);
    if (!doc) {
        goto RETURN;
    }
    if (invoke_tests[i].expr1 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(invoke_tests[i].value1);
    if (invoke_tests[i].value1 == NULL) {
        goto RETURN;
    }
    xpf = ws_xml_get_xpath_value(doc, invoke_tests[i].expr1);
    CU_ASSERT_PTR_NOT_NULL(xpf);
    if (!xpf) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpf, invoke_tests[i].value1);

    if (strcmp(xpf, invoke_tests[i].value1)) {
        //printf("Expected %s;   returned %s\n",
        //           invoke_tests[i].value1, xpf);
         goto RETURN;
    }
    if (invoke_tests[i].expr2 == NULL) {
        goto RETURN;
    }
    xpd = ws_xml_get_xpath_value(doc, invoke_tests[i].expr2);
    CU_ASSERT_PTR_NOT_NULL(xpd);
    if (!xpd) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(invoke_tests[i].value2);
    if (invoke_tests[i].value2 == NULL) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpd, invoke_tests[i].value2 );
    if (strcmp(xpd, invoke_tests[i].value2)) {
         goto RETURN;
    }
RETURN:
    u_free(xpf);
    u_free(xpd);
    if (doc) {
        ws_xml_destroy_doc(doc);
    }
    u_free((char *)invoke_tests[i].selectors);
    invoke_tests[i].selectors = old_selectors;
    destroy_action_options(&options);
    i++; // increase executed test number
}




int add_invoke_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;
        /* add the tests to the suite */
    for (i =0; i < ntests; i++) {
            found_test += (NULL != CU_add_test(ps, invoke_tests[i].explanation,
                                            (CU_TestFunc)invoke_test));
    }
    return (found_test > 0);
}

