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
	XML_TYPE_BOOL AcceptPause;
	XML_TYPE_BOOL AcceptStop;
	XML_TYPE_STR Caption;
	XML_TYPE_UINT32 CheckPoint;
	XML_TYPE_STR CreationClassName;
	XML_TYPE_STR Description;
	XML_TYPE_BOOL DesktopInteract;
	XML_TYPE_STR  DisplayName;
	XML_TYPE_STR ErrorControl;
	XML_TYPE_UINT32 ExitCode;
	XML_TYPE_STR InstallDate;
	XML_TYPE_STR Name;
	XML_TYPE_STR PathName;
	XML_TYPE_UINT32 ProcessId;
	XML_TYPE_UINT32 ServiceSpecificExitCode;
	XML_TYPE_STR ServiceType;
	XML_TYPE_BOOL Started;
	XML_TYPE_STR StartMode;
	XML_TYPE_STR StartName;
	XML_TYPE_STR State;
	XML_TYPE_STR Status;
	XML_TYPE_STR SystemCreationClassName;
	XML_TYPE_STR SystemName;
	XML_TYPE_UINT32 TagId;
	XML_TYPE_UINT32 WaitHint;
};
typedef struct __CIM_Servie CIM_Servie;

SER_START_ITEMS(CIM_Servie)
SER_NS_BOOL(RESOURCE_URI,"AcceptPause", 1),
SER_NS_BOOL(RESOURCE_URI,"AcceptStop", 1),
SER_NS_STR(RESOURCE_URI,"Caption", 1),
SER_NS_UINT32(RESOURCE_URI,"CheckPoint", 1),
SER_NS_STR(RESOURCE_URI,"CreationClassName", 1),
SER_NS_STR(RESOURCE_URI,"Description", 1),
SER_NS_BOOL(RESOURCE_URI,"DesktopInteract", 1),
SER_NS_STR(RESOURCE_URI,"DisplayName", 1),
SER_NS_STR(RESOURCE_URI,"ErrorControl", 1),
SER_NS_UINT32(RESOURCE_URI,"ExitCode", 1),
SER_NS_STR(RESOURCE_URI,"InstallDate", 1),
SER_NS_STR(RESOURCE_URI,"Name", 1),
SER_NS_STR(RESOURCE_URI,"PathName", 1),
SER_NS_UINT32(RESOURCE_URI,"ProcessId", 1),
SER_NS_UINT32(RESOURCE_URI,"ServiceSpecificExitCode", 1),
SER_NS_STR(RESOURCE_URI,"ServiceType", 1),
SER_NS_BOOL(RESOURCE_URI,"Started", 1),
SER_NS_STR(RESOURCE_URI,"StartMode", 1),
SER_NS_STR(RESOURCE_URI,"StartName", 1),
SER_NS_STR(RESOURCE_URI,"State", 1),
SER_NS_STR(RESOURCE_URI,"Status", 1),
SER_NS_STR(RESOURCE_URI,"SystemCreationClassName", 1),
SER_NS_STR(RESOURCE_URI,"SystemName", 1),
SER_NS_UINT32(RESOURCE_URI,"TagId", 1),
SER_NS_UINT32(RESOURCE_URI,"WaitHint", 1),
SER_END_ITEMS(CIM_Servie);

static char *endpoint = NULL;
char listall = 0;
char stop = 0;
char start = 0;
char desc = 0;
char status = 0;
char dump = 0;




static void print_info(CIM_Servie *service)
{
	if (service->Name) {
		printf("%s\n", service->Name );
	} else {
		printf("Error\n");
		return;
	}
	if (status)
		printf("\tState: %s\n", service->State );
	if (desc)
		printf("\tDescription: %s\n\n", service->Description );
}

static int list_services(WsManClient *cl, WsXmlDocH doc, void *data)
{
	if (!doc) {
		return;
	}
	if (dump)
		ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
	WsXmlNodeH node = ws_xml_get_soap_body(doc);
	node = ws_xml_get_child(node, 0,  XML_NS_ENUMERATION, WSENUM_PULL_RESP);
	node = ws_xml_get_child(node, 0,  XML_NS_ENUMERATION, WSENUM_ITEMS);
	if (ws_xml_get_child(node, 0, RESOURCE_URI , CLASSNAME )) {
		CIM_Servie *service = ws_deserialize(wsman_client_get_context(cl),
				node,
				CIM_Servie_TypeInfo, CLASSNAME,
				RESOURCE_URI, NULL,
				0, 0);
		print_info(service);
	}
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
		{ "dump", 'D',	U_OPTION_ARG_NONE,	&dump,
			"Dump request", NULL },
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
		if (dump) wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
		wsenum_enumerate_and_pull(cl, RESOURCE_URI, options, list_services, NULL );
	} else if (start && argv[1]) {
		if (dump) wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
		wsman_client_add_selector(&options, "Name", argv[1]);
		doc = wsman_invoke(cl, RESOURCE_URI, options,
				"StartService", NULL);
		ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
		ws_xml_destroy_doc(doc);
	} else if (stop && argv[1]) {
		if (dump) wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
		wsman_client_add_selector(&options, "Name", argv[1]);
		doc = wsman_invoke(cl, RESOURCE_URI, options,
				"StopService", NULL);
		ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
		ws_xml_destroy_doc(doc);
	} else if ( argv[1] ) {
		if (dump) wsman_set_action_option(&options,FLAG_DUMP_REQUEST );
		wsman_client_add_selector(&options, "Name", argv[1]);
		doc = ws_transfer_get(cl, RESOURCE_URI,
				options);
		if (doc) {
			WsXmlNodeH node = ws_xml_get_soap_body(doc);
			if (ws_xml_get_child(node, 0, RESOURCE_URI , CLASSNAME )) {
				CIM_Servie *service = ws_deserialize(wsman_client_get_context(cl),
						node,
						CIM_Servie_TypeInfo, CLASSNAME,
						RESOURCE_URI, NULL,
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


