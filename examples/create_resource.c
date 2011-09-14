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

#include "wsman-xml-serializer.h"
#include "wsman-client-api.h"
#include "wsman-client-transport.h"
#include "u/libu.h"

#define RESOURCE_URI "http://example.com/wbem/wscim/1/schema/1/EXL_ExamplePolicy"
#define CLASSNAME "EXL_ExamplePolicy"

/*
struct __XmlSerializerInfo Handles_TypeInfo[] =
{
      SER_UINT32("Handles", 1, 1)
};
*/
SER_TYPEINFO_UINT32;

struct __EXL_ExamplePolicy
{
    XML_TYPE_STR ElementName;
    XML_TYPE_STR Description;
    XML_TYPE_STR Caption;
    XML_TYPE_STR InstanceID;
    XML_TYPE_STR PolicyName;
    XML_TYPE_UINT32   PolicyPrecedence;
    XML_TYPE_DYN_ARRAY Handles;
    XML_TYPE_BOOL   DefaultTest;

};
typedef struct __EXL_ExamplePolicy EXL_ExamplePolicy;

SER_START_ITEMS(EXL_ExamplePolicy)
SER_NS_STR(RESOURCE_URI, "ElementName", 1),
SER_NS_STR(RESOURCE_URI, "Description", 1),
SER_NS_STR(RESOURCE_URI, "Caption", 1),
SER_NS_STR(RESOURCE_URI, "InstanceID", 1),
SER_NS_STR(RESOURCE_URI, "PolicyName", 1),
SER_NS_UINT32(RESOURCE_URI, "PolicyPrecedence", 1 ),
SER_NS_DYN_ARRAY(RESOURCE_URI, "Handles", 1, 10, uint32),
SER_NS_BOOL(RESOURCE_URI, "DefaultTest", 1),
SER_END_ITEMS(EXL_ExamplePolicy);


static char *endpoint = NULL;
static char dump;

int main(int argc, char** argv)
{
    WsManClient *cl;
    WsXmlDocH doc;
    client_opt_t *options;
    char retval = 0;
    u_error_t *error = NULL;


    u_option_entry_t opt[] = {
    { "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
        "Endpoint in form of a URL", "<uri>" },
    { "dump", 'd',	U_OPTION_ARG_NONE,	&dump,
        "Dump request", NULL },
    { NULL }
    };

    u_option_context_t *opt_ctx;	
    opt_ctx = u_option_context_new("");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, opt, "Create Resource");
    retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);

    u_option_context_free(opt_ctx);

    if (error) {
      if (error->message)
        printf ("%s\n", error->message);
      u_error_free(error);
      return 1;
    }
    u_error_free(error);


    u_uri_t *uri;
    if (endpoint) {
      if (u_uri_parse((const char *)endpoint, &uri)!=0 )
      	return 1;
    }
    if (!endpoint || !uri) {
      fprintf(stderr, "endpoint option required\n");
      return 1;
    }


    cl = wsmc_create( uri->host,
        uri->port,
        uri->path,
        uri->scheme,
        uri->user,
        uri->pwd);		
    wsmc_transport_init(cl, NULL);
    options = wsmc_options_init();

    if (dump) wsmc_set_action_option(options,FLAG_DUMP_REQUEST );
    options->max_envelope_size = 51200;
    options->timeout = 60000;

    EXL_ExamplePolicy *d = u_malloc(sizeof(EXL_ExamplePolicy));
    d->ElementName = u_strdup("name");
    d->DefaultTest = 1;

    int *array = NULL;
    int count = 4;
    array = (int *) malloc (sizeof (int) * count);
    array[0] = 1;
    array[1] = 0;
    array[2] = 3;
    array[3] = 5;
    d->Handles.count = count;
    d->Handles.data = array;

    doc = wsmc_action_create_serialized(cl, RESOURCE_URI, options,  d,
                              EXL_ExamplePolicy_TypeInfo);
    if (doc)
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));

    if (uri) {
      u_uri_free(uri);
    }

    wsmc_options_destroy(options);
    wsmc_release(cl);
    return 0;
}


