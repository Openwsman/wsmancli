#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-client-api.h"
#include "wsman-client-transport.h"
#include "wsman-debug.h"

#define DESC "Description"
static char *file = NULL;
static char *endpoint = NULL;
static int  debug_level = -1;

typedef enum {
      INTEROP_IDENTIFY = 0
} InteropTest;

static set_props(actionOptions *op, char *k, char *v) 
{
    op->properties = hash_create(HASHCOUNT_T_MAX, 0, 0);

    if ( !hash_alloc_insert(op->properties, k, v)) {
        fprintf(stderr, "hash_alloc_insert failed");
    }
    //op->properties = h;
}

static void
wsman_add_selectors_list_from_node( WsXmlNodeH input, actionOptions *options)
{
    hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);

    WsXmlNodeH node = ws_xml_get_child(input, 0, NULL, WSM_SELECTOR_SET);
    if ( node )
    {
        WsXmlNodeH selector;
        int index = 0;
        while( (selector = ws_xml_get_child(node, index++, NULL, WSM_SELECTOR)) )
        {
            char* attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);
            if ( attrVal == NULL )
                attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);

            if ( attrVal )
            {
                if (!hash_alloc_insert(h, attrVal, ws_xml_get_node_text(selector))) {
                    error("hash_alloc_insert failed");
                }
            }
        }
    }
    if (!hash_isempty(h))
        options->selectors = h;
}

static int pull_items(WsManClient *cl, WsXmlDocH doc, void *data)
{
  
  if (!doc) {
    return 0;
  }

        xml_parser_doc_dump(stdout, doc);

  return 1;
}


static int run_interop_test (WsManClient *cl, WsXmlNodeH scenario, InteropTest id)
{
    WsXmlDocH response;
    actionOptions options;
    initialize_action_options(&options);
    wsman_set_action_option(&options,FLAG_DUMP_REQUEST ); 
    if (id == 0) { // 6.1 Identify
        response = wsman_identify(cl, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 2) { // 6.2 Get

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        wsman_add_selectors_list_from_node(input, &options);
        char *resource_uri = ws_xml_get_node_text(r);
        response = ws_transfer_get(cl, resource_uri, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 3) { // 6.3 Get failure - invalid resoure URI

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        wsman_add_selectors_list_from_node(input, &options);
        char *resource_uri = ws_xml_get_node_text(r);
        response = ws_transfer_get(cl, resource_uri, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 4) { // 6.4 Get failure (MaxEnvelop exceeded)

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        wsman_add_selectors_list_from_node(input, &options);
        char *resource_uri = ws_xml_get_node_text(r);
        response = ws_transfer_get(cl, resource_uri, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 5) { // 6.5 Get failure - invalid selectors

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        wsman_add_selectors_list_from_node(input, &options);
        char *resource_uri = ws_xml_get_node_text(r);
        response = ws_transfer_get(cl, resource_uri, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 8) { // 7.1 Enumerate"

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        char *resource_uri = ws_xml_get_node_text(r);
        wsenum_enumerate_and_pull(cl, resource_uri , options, pull_items, NULL );
    }else if (id == 9) { // 7.2 Optimized Enumerate

        wsman_set_action_option(&options, FLAG_ENUMERATION_OPTIMIZATION);
        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        char *resource_uri = ws_xml_get_node_text(r);
        wsenum_enumerate_and_pull(cl, resource_uri , options, pull_items, NULL );
    }else if (id == 10) {  // 7.3 Enumerate failure

        /*
        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        char *resource_uri = ws_xml_get_node_text(r);
        WsXmlDocH e = wsenum_enumerate(cl, resource_uri , options );
        char *enumContext = wsenum_get_enum_context(e);
        if (enumContext) {
            response = wsenum_pull(cl, resource_uri, enumContext, options);
            xml_parser_doc_dump(stdout, response);
            WsXmlDocH response2 = wsenum_pull(cl, resource_uri, "xxxx", options);
            if (response2)
                xml_parser_doc_dump(stdout, response2);
        }
        */

    }else if (id == 11) { // 7.4 Enumerate ObjectAndEPR

        wsman_set_action_option(&options, FLAG_ENUMERATION_ENUM_EPR);
        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        char *resource_uri = ws_xml_get_node_text(r);
        wsenum_enumerate_and_pull(cl, resource_uri , options, pull_items, NULL );
    }else if (id == 16) { // 7.9 Enumerate Polymorphism

        wsman_set_action_option(&options, FLAG_POLYMORPHISM_NONE);
        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        char *resource_uri = ws_xml_get_node_text(r);
        wsenum_enumerate_and_pull(cl, resource_uri , options, pull_items, NULL );
    }else if (id == 17) { // 8.1 Invoke

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        WsXmlNodeH m  = ws_xml_get_child(input, 0, NULL, "MethodName");
        char *resource_uri = ws_xml_get_node_text(r);
        char *method = ws_xml_get_node_text(m);
        wsman_add_selectors_list_from_node(input, &options);
        response = wsman_invoke(cl, resource_uri, method, NULL, options);
        xml_parser_doc_dump(stdout, response);
    }else if (id == 18) { // 9.1 Put

        WsXmlNodeH input  = ws_xml_get_child(scenario, 0, NULL, "Input");
        WsXmlNodeH r  = ws_xml_get_child(input, 0, NULL, "ResourceURI");
        WsXmlNodeH k  = ws_xml_get_child(input, 0, NULL, "PropertyName");
        WsXmlNodeH v  = ws_xml_get_child(input, 0, NULL, "NewValue");
        printf("%s=%s\n", ws_xml_get_node_text(k), ws_xml_get_node_text(v) );
        char *resource_uri = ws_xml_get_node_text(r);
        wsman_add_selectors_list_from_node(input, &options);
        set_props(&options, ws_xml_get_node_text(k), ws_xml_get_node_text(v) );

        response = ws_transfer_get_and_put(cl, resource_uri, options);
        xml_parser_doc_dump(stdout, response);
    }

    return 0;
}





int main(int argc, char** argv)
{
    WsManClient *cl;

    WsXmlDocH doc;
    actionOptions options;
    char retval = 0;
    u_error_t *error = NULL;

    u_option_entry_t opt[] = {
    { "interop-file",	'f',	U_OPTION_ARG_STRING,	&file,
		"Interop file",	"<file>"  },
    { "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
		"Endpoint in form of a URL", "<uri>" },
    { "debug",   'd',    U_OPTION_ARG_INT,    &debug_level,
        "Set the verbosity of debugging output.",   "1-6" },
    { NULL }
    };

    u_option_context_t *opt_ctx;	
    opt_ctx = u_option_context_new("");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, opt, "interop");
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
      u_uri_parse((const char *)endpoint, &uri);
    }
    if (!endpoint || !uri) {
      fprintf(stderr, "endpoint option required\n");
      return 1;
    }
    wsman_debug_set_level(debug_level);

    wsman_client_transport_init(NULL);
    cl = wsman_create_client( uri->host,
        uri->port,
        uri->path,
        uri->scheme,
        uri->user,
        uri->pwd);		
    initialize_action_options(&options);

    if (file == NULL) {
      fprintf(stderr, "Interop file required\n");
      return 1;
    }
    doc = wsman_client_read_file(cl, file, "UTF-8", 0);
    //xml_parser_doc_dump(stdout, doc);


    //WsXmlNodeH node = ws_xml_get_soap_element(doc, "WSManInteropScenarios");

    WsXmlNodeH node;
    if (doc != NULL)
        node = ws_xml_get_doc_root(doc);

    WsXmlNodeH scenario;
    int index = 0;
    int test_id = 0;
    while( (scenario = ws_xml_get_child(node, index++, NULL, "Scenario")) ) {
        WsXmlAttrH desc = ws_xml_get_node_attr(scenario, 0);
        char *attr_val = ws_xml_get_attr_value(desc);
        WsXmlAttrH supported = ws_xml_get_node_attr(scenario, 1);
        if (strcmp(ws_xml_get_attr_value(supported), "true") == 0  && test_id == 18 ) {
            if (desc) {
                if (attr_val)
                    printf("%s (%d)\n\n", attr_val, test_id );
            }
            run_interop_test(cl, scenario, test_id);
        } else {
            if (desc) {
                if (attr_val)
                    printf("%s: Not Supported (%d)\n\n", attr_val, test_id );
            }
        }
        test_id++;
    }

    return 0;

}
