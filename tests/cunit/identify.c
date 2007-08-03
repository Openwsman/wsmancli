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

#include "config.h"

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






static TestData tests[] = {
  {
    "Testing Identify Request, check protocol version",
    NULL,
    NULL,
    NULL,
    NULL,
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProtocolVersion",
    XML_NS_WS_MAN,
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProductVersion",
    PACKAGE_VERSION,
    200,
    FLAG_NONE,
    0
  },
  {
    "Testing Identify Request, check product vendor",
    NULL,
    NULL,
    NULL,
    NULL,
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProductVendor",
    "Openwsman Project",
    NULL,
    NULL,
    200,
    FLAG_NONE,
    0
  }
};


static int ntests = sizeof (tests) / sizeof (tests[0]);


extern WsManClient *cl;
client_opt_t *options;



static void identify_test(void) {

    WsXmlDocH response;
    static int i = 0;
    char *xp = NULL;

    wsmc_reinit_conn(cl);
    options = wsmc_options_init();

    response = wsmc_action_identify(cl, options);
    CU_ASSERT_TRUE(wsmc_get_response_code(cl) == tests[i].final_status);

    CU_ASSERT_PTR_NOT_NULL(response);
    if (response == NULL) {
        goto RETURN;
    }

    if (tests[i].value1 != NULL) {
        xp = ws_xml_get_xpath_value(response, tests[i].expr1);
        CU_ASSERT_PTR_NOT_NULL(xp);
        if (xp) {
          CU_ASSERT_STRING_EQUAL(xp, tests[i].value1 );
        }
    }

RETURN:
    if (response) {
      ws_xml_destroy_doc(response);
    }
    u_free(xp);
    wsmc_options_destroy(options);
    i++;
}



int add_identify_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;

         /* add the tests to the suite */
    for (i =0; i < ntests; i++) {
        found_test += (NULL != CU_add_test(ps,
                        tests[i].explanation, identify_test));
    }

  return (found_test > 0);
}


