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

static char *endpoint = NULL;

struct __WsmanEpr {
  hash_t              *selectors;
  char* resource_uri;

};
typedef struct __WsmanEpr WsmanEpr;

static int collect_epr(WsManClient *cl, WsXmlDocH doc, void *data)
{
  
  if (!doc) {
    return 0;
  }

  list_t *list = (list_t *)data;
   WsXmlNodeH resource_uri_node, node1, node2, node3, node4;
 

  WsmanEpr *epr = (WsmanEpr *)u_zalloc(sizeof(WsmanEpr));

  WsXmlNodeH body = ws_xml_get_soap_body(doc);

 
  if (body != NULL)
    node1 = ws_xml_get_child(body, 0,  XML_NS_ENUMERATION, WSENUM_PULL_RESP);
  else  
    return 0;
  
  if (node1 != NULL )
    node2 = ws_xml_get_child(node1, 0,  XML_NS_ENUMERATION, WSENUM_ITEMS);
  else
    return 0;
  if (node2 != NULL) 
    node3 =  ws_xml_get_child(node2, 0,  XML_NS_ADDRESSING, WSA_EPR);
  else
    return 0;

  if (node3 != NULL) 
    node4 =  ws_xml_get_child(node3, 0,  XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
  else
    return 0;
  
  if (node4 != NULL) 
    resource_uri_node = ws_xml_get_child(node4, 0,  XML_NS_WS_MAN, WSM_RESOURCE_URI);
  else
    return 0;
  
  if (resource_uri_node != NULL) {
    epr->resource_uri =  u_strdup(ws_xml_get_node_text(resource_uri_node));
  } else {
    return 0;
  }

  epr->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0);
  WsXmlNodeH selectors = ws_xml_get_child(node4, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
  if ( selectors )
  {
    WsXmlNodeH selector;
    int index = 0;
    while( (selector = ws_xml_get_child(selectors, index++, XML_NS_WS_MAN, WSM_SELECTOR)) ) {
      char* attrVal = ws_xml_find_attr_value(selector, XML_NS_WS_MAN, WSM_NAME);
      if ( attrVal == NULL )
        attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);
        
      if ( attrVal ) {
        if (!hash_alloc_insert(epr->selectors, u_strdup(attrVal), u_strdup(ws_xml_get_node_text(selector)))) {
          error("hash_alloc_insert failed");
        }
      }
    }
    
  }

  lnode_t *n = lnode_create(epr);
  list_append(list, n);

  return 1;
}



int main(int argc, char** argv)
{
	
    WsManClient *cl;
    WsXmlDocH doc;
    actionOptions options;
    char retval = 0;
    u_error_t *error = NULL;


    u_option_entry_t opt[] = {
    { "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
		"Endpoint in form of a URL", "<uri>" },
    { NULL }
    };

    u_option_context_t *opt_ctx;	
    opt_ctx = u_option_context_new("");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, opt, "Win32 Service");
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


    wsman_client_transport_init(NULL);
    cl = wsman_create_client( uri->host,
        uri->port,
        uri->path,
        uri->scheme,
        uri->user,
        uri->pwd);		
    initialize_action_options(&options);

    wsman_set_action_option(&options, FLAG_ENUMERATION_ENUM_EPR);
    

    list_t *l = list_create(LISTCOUNT_T_MAX);
    wsenum_enumerate_and_pull(cl, argv[1] , options, collect_epr, l );
    
    
    printf("returned items: %d\n", list_count(l));
    lnode_t *node = list_first(l);
    while(node) {
      WsmanEpr* epr = (WsmanEpr*) node->list_data;
      hscan_t hs;
      hnode_t *hn;

      printf("%s\n", epr->resource_uri);
      hash_scan_begin(&hs, epr->selectors);
      while ((hn = hash_scan_next(&hs))) {
        printf("\t%s  =   %s\n", (char*) hnode_getkey(hn),
               (char*) hnode_get(hn) );
      }
     
      node = list_next (l, node);
    }		

    if (uri) {
      u_uri_free(uri);
    }
    
    destroy_action_options(&options);
    wsman_release_client(cl);
    return 0;
}


