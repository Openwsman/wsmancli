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
 * @author Nathan Rakoff
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
#include "wsman-debug.h"
#include "common.h"



static int _debug = 0;


static char *filters1[] = {
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
     "wsa:DestinationUnreachable",
     "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
     "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
     NULL, NULL
};


static char *filters2[] = {
   "/s:Envelope/s:Body/wsen:EnumerateResponse/wsman:Items[1]/wsa:EndpointReference/wsa:Address",
    NULL,
    NULL, NULL,
};

static char *filters3[] = {
    "/s:Envelope/s:Header/wsman:TotalItemsCountEstimate",
    NULL,
    NULL, NULL,
};


static TestData tests[] = {
  {
    "Enumeration with non existent Resource URI.", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx", 
    NULL, 
    NULL, 
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
/* 
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
*/
    500,
    FLAG_NONE,
    0,
    filters1,
  },
  {
    "Enumeration  (Optimized)", 
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService", 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    200,
    FLAG_ENUMERATION_OPTIMIZATION,
    200
  },
  {
    "Enumeration  (Optimized/EPR/Count)", 
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService", 
    NULL, 
    NULL, 
    NULL, 
    NULL,
    NULL, 
    NULL,
    NULL, 
    200,
    FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_ENUM_EPR |
                                              FLAG_ENUMERATION_COUNT_ESTIMATION ,
    200,
    filters2,
  },
  {
    "Enumeration (Optimized/EPR)", 
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService", 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    200,
    FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_ENUM_EPR,
    200
  },
  {
    "Enumeration (Optimized/ObjEPR)", 
    "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_InitdService", 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    200,
    FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_ENUM_OBJ_AND_EPR,
    200
  },
  {
    "Enumeration with valid Resource URI and Items Count Estimation.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL, 
    NULL, 
    NULL, 
    "/s:Envelope/s:Header/wsman:TotalItemsCountEstimate",
    NULL,
    NULL,
    NULL,
    200,
    FLAG_ENUMERATION_COUNT_ESTIMATION,
    0,
    filters3,
  }, /*
     {
     "Enumeration with valid Resource URI.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_NONE,
     0
     },
     {
     "Enumeration with valid Resource URI and additional invalid selectors.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_NONE,
     1
     },	
     {
     "Enumeration with valid Resource URI/Count Estimation.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_COUNT_ESTIMATION,
     0
     },
     {
     "Enumeration with valid Resource URI/Optimization.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION,
     0
     },	
     {
     "Enumeration with Count Estimation/Optimzation and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION,
     10

     },
     {
     "Enumeration with Count Estimation/Optimzation/Epr and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION | FLAG_ENUMERATION_ENUM_EPR,
     10

     },	
     {
     "Enumeration with Count Estimation/Optimzation/ObjAndEpr and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION | FLAG_ENUMERATION_ENUM_OBJ_AND_EPR,
     10

     }	*/
};


static int ntests = sizeof (tests) / sizeof (tests[0]);


extern WsManClient *cl;

actionOptions options;

static void enumeration_test() {
    char *enumContext = NULL;
    static int i = 0;
    char *xp = NULL;
    int num;
    int j;

    reinit_client_connection(cl);
    initialize_action_options(&options);

    options.flags = tests[i].flags;

    if (tests[i].selectors != NULL)
         wsman_add_selectors_from_query_string(&options, tests[i].selectors);

    options.max_elements = tests[i].max_elements;
    WsXmlDocH enum_response = wsenum_enumerate(cl,
                                (char *)tests[i].resource_uri, options);

    CU_ASSERT_TRUE(wsman_get_client_response_code(cl) == tests[i].final_status );
    CU_ASSERT_PTR_NOT_NULL(enum_response);
    if (enum_response) {
        enumContext = wsenum_get_enum_context(enum_response);
    } else {
        goto RETURN;
    }
    check_response_header(enum_response, wsman_get_client_response_code(cl),
       ENUM_ACTION_ENUMERATERESPONSE);

    if (_debug) wsman_output(enum_response);

    if (tests[i].filters == NULL) {
        goto RETURN;
    }

    for (j = 0; tests[i].filters[j] != NULL; j += 2) {
        u_free(xp);
        xp = ws_xml_get_xpath_value(enum_response, tests[i].filters[j]);
        CU_ASSERT_PTR_NOT_NULL(xp);
        if (xp == NULL) {
            if (verbose) {
                printf("\n No Xpath: %s      ", tests[i].filters[j]);
            }
            continue;
        }
        if (tests[i].filters[j + 1]) {
             CU_ASSERT_STRING_EQUAL(xp, tests[i].filters[j + 1]);
             if (verbose && strcmp(xp, tests[i].filters[j + 1])) {
                printf("\nExpected:  %s\nReturned:  %s       ",
                        tests[i].filters[j + 1], xp);
             }
        }
    }

RETURN:
    if (enumContext) {
        wsenum_release(cl,
                       (char *)tests[i].resource_uri, 
                       enumContext,
                       options);
    }
    if (enum_response) {
        ws_xml_destroy_doc(enum_response);
    }
    u_free(xp);
    destroy_action_options(&options);
    i++; // decrease executed test number
}


int add_enumeration_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;

        /* add the tests to the suite */
    for (i = 0; i < ntests; i++) {
        found_test += (NULL != CU_add_test(ps,
                            tests[i].explanation, enumeration_test));
    }
    return (found_test > 0);
}


