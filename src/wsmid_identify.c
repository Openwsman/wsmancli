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
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-client.h"
#include "wsman-client-transport.h"


struct __wsmid_identify
{
	char* ProtocolVersion;
	char* ProductVendor;
	char* ProductVersion;
};
typedef struct __wsmid_identify wsmid_identify;

SER_START_ITEMS("IdentifyResponse", wsmid_identify)
SER_STR("ProtocolVersion", 1, 1), 
SER_STR("ProductVendor", 1, 1),
SER_STR("ProductVersion", 1, 1),
SER_END_ITEMS("IdentifyResponse", wsmid_identify);

typedef struct {
	const char *server;
	int port;
	const char *path;
	const char *scheme;
	const char *username;
	const char *password;
} ServerData;

ServerData sd[] = {
	{"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};


int main(int argc, char** argv)
{
	
    WsManClient *cl;
    WsXmlDocH doc;
    actionOptions options;
    wsman_client_transport_init(NULL);
    cl = wsman_create_client( sd[0].server,
        sd[0].port,
        sd[0].path,
        sd[0].scheme,
        sd[0].username,
        sd[0].password);		
    initialize_action_options(&options);


    doc = wsman_identify(cl, options);

    WsXmlNodeH soapBody = ws_xml_get_soap_body(doc);
    if (ws_xml_get_child(soapBody, 0, XML_NS_WSMAN_ID, "IdentifyResponse")) {
         wsmid_identify *id = ws_deserialize(
                cl->wscntx,
                soapBody,
                wsmid_identify_TypeInfo,
                "IdentifyResponse",
                XML_NS_WSMAN_ID,
                XML_NS_WSMAN_ID,
                0,
                0);
        printf(" Vendor: %s\n", id->ProductVendor);
        printf(" Version: %s\n", id->ProductVersion);
        printf(" Protocol Version: %s\n", id->ProtocolVersion);
    }

    if (doc) {			
        ws_xml_destroy_doc(doc);
    }

    destroy_action_options(&options);
    wsman_release_client(cl);

	
	return 0;
}


