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
#include "wsman-xml-serializer.h"

#define RESOURCE_URI "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"
#define CLASSNAME "Win32_Service"

/*
  boolean AcceptPause;
  boolean AcceptStop;
  string Caption;
  uint32 CheckPoint;
  string CreationClassName;
  string Description;
  boolean DesktopInteract;
  string DisplayName;
  string ErrorControl;
  uint32 ExitCode;
  datetime InstallDate;
  string Name;
  string PathName;
  uint32 ProcessId;
  uint32 ServiceSpecificExitCode;
  string ServiceType;
  boolean Started;
  string StartMode;
  string StartName;
  string State;
  string Status;
  string SystemCreationClassName;
  string SystemName;
  uint32 TagId;
  uint32 WaitHint;
 
   */


struct __CIM_Servie
{
  int AcceptPause;
  int AcceptStop;
  char* Caption;
  unsigned long CheckPoint;
  char* CreationClassName;
  char* Description;
  int DesktopInteract;
  char*  DisplayName;
  char* ErrorControl;
  unsigned long ExitCode;
  char* InstallDate;
  char* Name;
  char* PathName;
  unsigned long ProcessId;
  unsigned long ServiceSpecificExitCode;
  char* ServiceType;
  int Started;
  char* StartMode;
  char* StartName;
  char* State;
  char* Status;
  char* SystemCreationClassName;
  char* SystemName;
  unsigned long TagId;
  unsigned long WaitHint;
};
typedef struct __CIM_Servie CIM_Servie;

SER_START_ITEMS("CIM_Servie", CIM_Servie)
SER_BOOL("AcceptPause",0,1),
SER_BOOL("AcceptStop",0,1),
SER_STR("Caption",0,1),
SER_UINT32("CheckPoint", 1 ,1 ),
SER_STR("CreationClassName",0,1),
SER_STR("Description",0,1),
SER_BOOL("DesktopInteract",0,1),
SER_STR("DisplayName",0,1),
SER_STR("ErrorControl",0,1),
SER_UINT32("ExitCode",0,1),
SER_STR("InstallDate",0,1),
SER_STR("Name",0,1),
SER_STR("PathName",0,1),
SER_UINT32("ProcessId",0,1),
SER_UINT32("ServiceSpecificExitCode",0,1),
SER_STR("ServiceType",0,1),
SER_BOOL("Started",0,1),
SER_STR("StartMode",0,1),
SER_STR("StartName",0,1),
SER_STR("State",0,1),
SER_STR("Status",0,1),
SER_STR("SystemCreationClassName",0,1),
SER_STR("SystemName",0,1),
SER_UINT32("TagId",0,1),
SER_UINT32("WaitHint",0,1),
SER_END_ITEMS("CIM_Servie", CIM_Servie);

static char *endpoint = NULL;
char listall = 0;
char stop = 0;
char start = 0;
char desc = 0;
char status = 0;




static void print_info(CIM_Servie *service) {
    printf("%s\n", service->Name );
    if (status)
        printf("\tState: %s\n", service->State );
    if (desc)
        printf("\tDescription: %s\n\n", service->Description );
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
    { "list-all", 'l',	U_OPTION_ARG_NONE,	&listall,
		"List all services", NULL },
    { "desc", 'd',	U_OPTION_ARG_NONE,	&desc,
		"Show service description", NULL },
    { "stop", 's',	U_OPTION_ARG_NONE,	&stop,
		"Stop service", NULL },
    { "start", 'S',	U_OPTION_ARG_NONE,	&start,
		"Start service", NULL },
    { "status", 'X',	U_OPTION_ARG_NONE,	&status,
		"Get service status", NULL },
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

    if (listall) {
        char *enumContext;
        WsXmlDocH enum_response;

        enum_response = wsenum_enumerate(cl, RESOURCE_URI, options);
        if (enum_response) {
            if (!wsman_get_client_response_code(cl) == 200 ||
                    !wsman_get_client_response_code(cl) == 500) {
                return (1);
            }
            enumContext = wsenum_get_enum_context(enum_response);
            ws_xml_destroy_doc(enum_response);
        } else {
            return(1);
        }

        while (enumContext !=NULL) {
            doc = wsenum_pull(cl, RESOURCE_URI, enumContext, options);

            if (wsman_get_client_response_code(cl) != 200 &&
                    wsman_get_client_response_code(cl) != 500) {
                return (1);
            }
            enumContext = wsenum_get_enum_context(doc);
            if (doc) {
                WsXmlNodeH node = ws_xml_get_soap_body(doc);
                node = ws_xml_get_child(node, 0,  XML_NS_ENUMERATION, WSENUM_PULL_RESP);
                node = ws_xml_get_child(node, 0,  XML_NS_ENUMERATION, WSENUM_ITEMS);
                if (ws_xml_get_child(node, 0, RESOURCE_URI , CLASSNAME )) {
                    CIM_Servie *service = ws_deserialize(wsman_client_get_context(cl),
                            node,
                            CIM_Servie_TypeInfo, CLASSNAME,
                            RESOURCE_URI, RESOURCE_URI,
                            0, 0);
                    print_info(service);
                }
                ws_xml_destroy_doc(doc);
            }
        }
    } else if (start && argv[1]) {
        //wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
        wsman_add_selectors_from_query_string(&options, u_strdup_printf("Name=%s", argv[1]));
        doc = wsman_invoke(cl, RESOURCE_URI,
                                "StartService", options);
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        ws_xml_destroy_doc(doc);
    } else if (stop && argv[1]) {
        //wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
        wsman_add_selectors_from_query_string(&options, u_strdup_printf("Name=%s", argv[1]));
        doc = wsman_invoke(cl, RESOURCE_URI,
                                "StopService", options);
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        ws_xml_destroy_doc(doc);
    } else if ( argv[1] ) {
        //wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
        wsman_add_selectors_from_query_string(&options, u_strdup_printf("Name=%s", argv[1]));
        doc = ws_transfer_get(cl, RESOURCE_URI,
                                 options);
        if (doc) {
            WsXmlNodeH node = ws_xml_get_soap_body(doc);
            if (ws_xml_get_child(node, 0, RESOURCE_URI , CLASSNAME )) {
                CIM_Servie *service = ws_deserialize(wsman_client_get_context(cl),
                        node,
                        CIM_Servie_TypeInfo, CLASSNAME,
                        RESOURCE_URI, RESOURCE_URI,
                        0, 0);
                desc = 1;
                status = 1;
                print_info(service);
            }
        }
        ws_xml_destroy_doc(doc);
    }

    if (uri) {
      u_uri_free(uri);
    }
    
    destroy_action_options(&options);
    wsman_release_client(cl);
    return 0;
}


