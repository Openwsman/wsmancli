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


#include "wsman-api.h"
#include "u/libu.h"

int facility = LOG_DAEMON;

struct __wsmid_identify
{
	XML_TYPE_STR ProtocolVersion;
	XML_TYPE_STR ProductVendor;
	XML_TYPE_STR ProductVersion;
};
typedef struct __wsmid_identify wsmid_identify;

SER_START_ITEMS(wsmid_identify)
SER_NS_STR(XML_NS_WSMAN_ID, "ProtocolVersion", 1),
	SER_NS_STR(XML_NS_WSMAN_ID, "ProductVendor", 1),
	SER_NS_STR(XML_NS_WSMAN_ID, "ProductVersion", 1),
	SER_END_ITEMS(wsmid_identify);


static char  vendor = 0;
static char version = 0;
static char protocol = 0;
static char *endpoint = NULL;
static int debug_level = -1;

static void initialize_logging(void)
{
	debug_add_handler(wsman_debug_message_handler, DEBUG_LEVEL_ALWAYS,
						              NULL);
}


int main(int argc, char** argv)
{

	WsManClient *cl;
	WsXmlDocH doc;
	client_opt_t *options;
	char retval = 0;
	u_error_t *error = NULL;
	WsSerializerContextH cntx = NULL;

	initialize_logging();

	u_option_entry_t opt[] = {
		{ "product",	'p',	U_OPTION_ARG_NONE,	&vendor,
			"Print Product Vendor",	NULL },
		{ "version",	'v',	U_OPTION_ARG_NONE,	&version,
			"Print Product Version",	NULL  },
		{ "protocol",	'P',	U_OPTION_ARG_NONE,	&protocol,
			"Print Protocol Version",	NULL  },
		{ "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
			"Endpoint in form of a URL", "<uri>" },
		{ NULL }
	};

	u_option_context_t *opt_ctx;
	opt_ctx = u_option_context_new("");
	u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
	u_option_context_add_main_entries(opt_ctx, opt, "wsmid_identify");
	retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);
	if (retval < 0) {
		fprintf(stderr, "Failed to parse arguments\n");
		return 1;
	}

	u_option_context_free(opt_ctx);

	if (error) {
		if (error->message)
			printf ("%s\n", error->message);
		u_error_free(error);
		return 1;
	}
	u_error_free(error);

	wsman_debug_set_level(debug_level);

	u_uri_t *uri;
	if (endpoint) {
		u_uri_parse((const char *)endpoint, &uri);
	}
	if (!endpoint || !uri) {
		fprintf(stderr, "endpoint option required\n");
		return 1;
	}

	/*
	   fprintf( stderr, "wsman_create_client( host %s, port %d, path %s, scheme %s, user %s, passwd %s\n", uri->host,
	   uri->port,
	   uri->path,
	   uri->scheme,
	   uri->user,
	   uri->pwd);
	   */

	cl = wsmc_create( uri->host,
			uri->port,
			uri->path,
			uri->scheme,
			uri->user,
			uri->pwd);
	options = wsmc_options_init();


	doc = wsmc_action_identify(cl, options);
	if (doc) {
		WsXmlNodeH soapBody = ws_xml_get_soap_body(doc);
		if (ws_xml_get_child(soapBody, 0, XML_NS_WSMAN_ID, "IdentifyResponse")) {

			cntx = ws_serializer_init();
			debug("cntx: %p", cntx);

			wsmid_identify *id = ws_deserialize(cntx,
					soapBody,
					wsmid_identify_TypeInfo, "IdentifyResponse",
					XML_NS_WSMAN_ID, NULL,
					0, 0);
			if (!id) {
					fprintf(stderr, "Serialization failed\n");
					return 1;
			}

			if (vendor)
				printf("%s\n", id->ProductVendor);
			if (version)
				printf("%s\n", id->ProductVersion);
			if (protocol)
				printf("%s\n", id->ProtocolVersion);

			if (!protocol && !vendor && !version && id) {
				printf("\n");
				printf("%s %s supporting protocol %s\n", id->ProductVendor, id->ProductVersion,id->ProtocolVersion);
			}

		}
		if (uri) {
			u_uri_free(uri);
		}

		ws_xml_destroy_doc(doc);
	} else {
		if (wsmc_get_response_code(cl) != 200) {
			fprintf(stderr, "Connection failed. response code = %ld\n",
					wsmc_get_response_code(cl));
			if (wsmc_get_fault_string(cl)) {
				fprintf(stderr, "%s\n",
						wsmc_get_fault_string(cl));
			}
		}
	}

	wsmc_options_destroy(options);
	wsmc_release(cl);


	return 0;
}


