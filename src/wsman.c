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
 * @author Eugene Yarmosh
 * @author Vadim Revyakin
 */
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

#include <wsman-client-api.h>
#include <wsman-client-transport.h>
#include <wsman-debug.h>
// #include "wsman-client-options.h"

#if __linux__
extern char *getpass (const char *__prompt);
#endif

static int server_port = 0;
static char *cainfo = NULL;
static char *cert = NULL;
static char *sslkey = NULL;
static char *endpoint = NULL;
static char *username = NULL;
static char *password = NULL;
static char *server = "localhost";
static char *agent = NULL;
static char *url_path = NULL;
static char *authentication_method = NULL;
static char noverify_peer = 0;
static char noverify_host = 0;

static int  transport_timeout = 0;
static char *proxy = NULL;
static char *proxy_upwd = NULL;


static int debug_level = -1;
static char *test_case = NULL;
static int enum_max_elements = 0;
char enum_optimize = 0;
char enum_estimate = 0;
char dump_request = 0;
char step = 0;
char request_only = 0;
char cim_extensions = 0;
char cim_references = 0;
char cim_associators = 0;
static char *enum_mode = NULL;
static char *binding_enum_mode = NULL;
static char *enum_context = NULL;
static char *event_delivery_mode = NULL;
static char *event_delivery_uri = NULL;
static int event_subscription_expire = 0;
static int event_heartbeat = 0;
static int event_sendbookmark =0;
static char *event_subscription_id = NULL;

static char *cim_namespace = NULL;
static char *fragment = NULL;
static char *wsm_filter = NULL;
static char *wsm_dialect = NULL;
static char *input = NULL;

static unsigned long operation_timeout = 0;
static unsigned long max_envelope_size = 0;

static char *_action = NULL;
static char *config_file = NULL;
static char *output_file = NULL;
static char *resource_uri_opt = NULL;
static char *invoke_method = NULL;
static char **properties = NULL;

struct _WsActions {
	char *action;
	int value;
};
typedef struct _WsActions WsActions;

WsActions action_data[] = {
	{"get", WSMAN_ACTION_TRANSFER_GET},
	{"put", WSMAN_ACTION_TRANSFER_PUT},
	{"create", WSMAN_ACTION_TRANSFER_CREATE},
	{"delete", WSMAN_ACTION_TRANSFER_DELETE},
	{"enumerate", WSMAN_ACTION_ENUMERATION},
	{"pull", WSMAN_ACTION_PULL},
	{"release", WSMAN_ACTION_RELEASE},
	{"invoke", WSMAN_ACTION_CUSTOM},
	{"identify", WSMAN_ACTION_IDENTIFY},
	{"subscribe", WSMAN_ACTION_SUBSCRIBE},
	{"unsubscribe", WSMAN_ACTION_UNSUBSCRIBE},
	{"renew", WSMAN_ACTION_RENEW},
	{"pull", WSMAN_ACTION_EVENT_PULL},
	{"test", WSMAN_ACTION_TEST},
	{NULL, 0},
};

WsActions delivery_mode[] = {
	{"push", WSMAN_DELIVERY_PUSH},
	{"pushwithack", WSMAN_DELIVERY_PUSHWITHACK},
	{"events", WSMAN_DELIVERY_EVENTS},
	{"pull", WSMAN_DELIVERY_PULL},
	{NULL, 0}
};

static char wsman_parse_options(int argc, char **argv)
{
	char retval = 0;
	u_error_t *error = NULL;

	u_option_entry_t options[] = {
		{"debug", 'd', U_OPTION_ARG_INT, &debug_level,
		 "Set the verbosity of debugging output.", "1-6"},
		{"cacert", 'c', U_OPTION_ARG_STRING, &cainfo,
		 "Certificate file to verify the peer", "<filename>"},
		{"cert", 'A', U_OPTION_ARG_STRING, &cert,
		 "Certificate file. The certificate must be in PEM format.", "<filename>"},
		{"sslkey", 'K', U_OPTION_ARG_STRING, &sslkey,
		 "SSL Key.", "<key>"},
		{"username", 'u', U_OPTION_ARG_STRING, &username,
		 "User name", "<username>"},
		{"path", 'g', U_OPTION_ARG_STRING, &url_path,
		 "Path", "<path>"},
		{"input", 'J', U_OPTION_ARG_STRING, &input,
		 "File with resource for Create and Put operations in XML, can be a SOAP envelope",
		 "<filename>"},
		{"password", 'p', U_OPTION_ARG_STRING, &password,
		 "Password", "<password>"},
		{"hostname", 'h', U_OPTION_ARG_STRING, &server,
		 "Host name", "<hostname>"},
		{"endpoint", 'b', U_OPTION_ARG_STRING, &endpoint,
		 "End point", "<url>"},
		{"port", 'P', U_OPTION_ARG_INT, &server_port,
		 "Server Port", "<port>"},
		{"proxy", 'X', U_OPTION_ARG_STRING, &proxy,
		 "Proxy name", "<proxy>"},
		{"proxyauth", 'Y', U_OPTION_ARG_STRING, &proxy_upwd,
		 "Proxy user:pwd", "<proxyauth>"},
		{"auth", 'y', U_OPTION_ARG_STRING, &authentication_method,
		 "Authentication Method", "<basic|digest|gss>"},
		{"method", 'a', U_OPTION_ARG_STRING, &invoke_method,
		 "Method (Works only with 'invoke')", "<custom method>"},
		{"prop", 'k', U_OPTION_ARG_STRING_ARRAY, &properties,
		 "Properties with key value pairs (For 'put', 'invoke' and 'create')",
		 "<key=val>"},
		{"config-file", 'C', U_OPTION_ARG_STRING, &config_file,
		 "Alternate configuration file", "<file>"},
		{"out-file", 'O', U_OPTION_ARG_STRING, &output_file,
		 "Write output to file", "<file>"},
		{"noverifypeer", 'V', U_OPTION_ARG_NONE, &noverify_peer,
		 "Not to verify peer certificate", NULL},
		{"noverifyhost", 'v', U_OPTION_ARG_NONE, &noverify_host,
		 "Not to verify hostname", NULL},
		{"transport-timeout", 'I', U_OPTION_ARG_INT, &transport_timeout,
		 "Transport timeout in seconds", "<time in sec>"},
		{NULL}
	};



	u_option_entry_t request_options[] = {
		{"filter", 'x', U_OPTION_ARG_STRING, &wsm_filter,
		 "Filter", "<filter>"},
		{"dialect", 'D', U_OPTION_ARG_STRING, &wsm_dialect,
		 "Filter Dialect", "<dialect>"},
		{"operation-timeout", 't', U_OPTION_ARG_INT, &operation_timeout,
		 "Operation timeout in seconds", "<time in sec>"},
		{"max-envelope-size", 'e', U_OPTION_ARG_INT,
		 &max_envelope_size,
		 "maximal envelope size", "<size>"},
		{"fragment", 'F', U_OPTION_ARG_STRING, &fragment,
		 "Fragment (Supported Dialects: XPATH)", "<fragment>"},
		{NULL}
	};

	u_option_entry_t enum_options[] = {

		{"max-elements", 'm', U_OPTION_ARG_INT, &enum_max_elements,
		 "Max Elements Per Pull/Optimized Enumeration",
		 "<max number of elements>"},
		{"optimize", 'o', U_OPTION_ARG_NONE, &enum_optimize,
		 "Optimize enumeration results", NULL},
		{"estimate-count", 'E', U_OPTION_ARG_NONE, &enum_estimate,
		 "Return estimation of total items", NULL},
		{"enum-mode", 'M', U_OPTION_ARG_STRING, &enum_mode,
		 "Enumeration Mode", "epr|objepr"},
		{"enum-context", 'U', U_OPTION_ARG_STRING, &enum_context,
		 "Enumeration Context (For use with Pull and Release)",
		 "<enum context>"},
		{NULL}
	};

	u_option_entry_t event_options[] = {
		{"delivery-mode", 'G', U_OPTION_ARG_STRING, &event_delivery_mode,
		"Four delivery modes available: push/pushwithack/events/pull",
		"<mode>"},
		{"notification-uri", 'Z', U_OPTION_ARG_STRING, &event_delivery_uri,
		"Where notifications are sent",
		"<uri>"},
		{"subscription-expiry-time", 'r', U_OPTION_ARG_INT, &event_subscription_expire,
		"subscription expiry time in seconds",
		"<seconds>"},
		{"heartbeat",'H', U_OPTION_ARG_INT, &event_heartbeat,
		"Send hearbeat in an interval",
		"<seconds>"},
		{"bookmark", 'l', U_OPTION_ARG_NONE, &event_sendbookmark,
		"Send bookmark",NULL},
		{"subscription-identifier", 'i', U_OPTION_ARG_STRING, &event_subscription_id,
		"Used to specify which subscription",
		"<uuid:XXX>"},
		{NULL}
	};

	u_option_entry_t cim_options[] = {

		{"namespace", 'N', U_OPTION_ARG_STRING, &cim_namespace,
		 "CIM Namespace (default is root/cimv2)", "<namespace>"},
		{"binding-enum-mode", 'B', U_OPTION_ARG_STRING,
		 &binding_enum_mode,
		 "CIM binding Enumeration Mode", "none|include|exclude"},
		{"cim-extensions", 'T', U_OPTION_ARG_NONE, &cim_extensions,
		 "Show CIM Extensions", NULL},
		{"references", 'W', U_OPTION_ARG_NONE, &cim_references,
		 "CIM References", NULL},
		{"associators", 'w', U_OPTION_ARG_NONE, &cim_associators,
		 "CIM Associators", NULL},
		{NULL}
	};

	u_option_entry_t test_options[] = {
		{"from-file", 'f', U_OPTION_ARG_STRING, &test_case,
		 "Send request from file", "<file name>"},
		{"print-request", 'R', U_OPTION_ARG_NONE, &dump_request,
		 "print request on stdout", NULL},
		{"request", 'Q', U_OPTION_ARG_NONE, &request_only,
		 "Only output reqest. Not send it.", NULL},
		{"step", 'S', U_OPTION_ARG_NONE, &step,
		 "Do not perform multiple operations (do not pull data when enumerating)",
		 NULL},
		//{ "print-response", 'N', 0, G_OPTION_ARG_NONE, &dump_response, "print all responses to stdout", NULL},
		{NULL}
	};

	u_option_group_t *enum_group;
	u_option_group_t *event_group;
	u_option_group_t *test_group;
	u_option_group_t *cim_group;
	u_option_group_t *req_flag_group;

	u_option_context_t *opt_ctx;
	opt_ctx = u_option_context_new("<action> <Resource Uri>");
	enum_group = u_option_group_new("enumeration", "Enumeration",
					"Enumeration Options");
	test_group = u_option_group_new("tests", "Tests", "Test Cases");
	cim_group = u_option_group_new("cim", "CIM", "CIM Options");
	event_group = u_option_group_new("event", "Event subscription", "Subscription Options");
	req_flag_group =
	    u_option_group_new("flags", "Flags", "Request Flags");

	u_option_group_add_entries(enum_group, enum_options);
	u_option_group_add_entries(test_group, test_options);
	u_option_group_add_entries(cim_group, cim_options);
	u_option_group_add_entries(req_flag_group, request_options);
	u_option_group_add_entries(event_group, event_options);

	u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
	u_option_context_add_main_entries(opt_ctx, options, "wsman");
	u_option_context_add_group(opt_ctx, enum_group);
	u_option_context_add_group(opt_ctx, test_group);
	u_option_context_add_group(opt_ctx, cim_group);
	u_option_context_add_group(opt_ctx, req_flag_group);
	u_option_context_add_group(opt_ctx, event_group);

	retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);
	u_option_context_free(opt_ctx);

	if (error) {
		if (error->message)
			printf("%s\n", error->message);
		u_error_free(error);
		return FALSE;
	}

	if (argc > 2) {
		_action = argv[1];
		resource_uri_opt = argv[2];
	} else {
		if (argv[1] && (strcmp(argv[1], "identify") == 0 ||
				strcmp(argv[1], "test") == 0 ||
                                strcmp(argv[1], "unsubscribe") == 0 ||
                                strcmp(argv[1], "renew") == 0)) {
			_action = argv[1];
		} else {
			fprintf(stderr,
				"Error: operation can not be completed."
				" Action or/and Resource Uri missing.\n");
			return FALSE;
		}
	}
	u_error_free(error);

	// set default options
	if (server_port == 0) {
		server_port = cainfo ? 8888 : 8889;
	}
	if (url_path == NULL) {
		url_path = "/wsman";
	}
	return TRUE;
}




static void wsman_output(WsManClient * cl, WsXmlDocH doc)
{
	FILE *f = stdout;
	const char *filename = output_file;
	WS_LASTERR_Code err;

	err = wsmc_get_last_error(cl);
	if (err != WS_LASTERR_OK) {
		return;
	}
	if (!doc) {
		error("doc with NULL content");
		return;
	}
	if (filename) {
		f = fopen(filename, "w+");
		if (f == NULL) {
			error("Could not open file for writing");
			return;
		}
	}
	ws_xml_dump_node_tree(f, ws_xml_get_doc_root(doc));
	if (f != stdout) {
		fclose(f);
	}
	return;
}


static void initialize_logging(void)
{
	debug_add_handler(wsman_debug_message_handler, DEBUG_LEVEL_ALWAYS,
			  NULL);
}



static void
request_usr_pwd( WsManClient *client, wsman_auth_type_t auth,
                char **username,
                char **password)
{
  char *pw;
  char user[21];
  char *p;

  fprintf(stdout,"Authentication failed, please retry\n");
  /*
  fprintf(stdout, "%s authentication is used\n",
          wsmc_transport_get_auth_name( auth));
	  */
  printf("User name: ");
  fflush(stdout);
  if ( (p = fgets(user, 20, stdin) ) != NULL )
  {

    if (strchr(user, '\n'))
      (*(strchr(user, '\n'))) = '\0';
    *username = u_strdup_printf ("%s", user);
  } else {
    *username = NULL;
  }

  pw = (char *)getpass("Password: ");
  *password = u_strdup_printf ("%s", pw);
}



static hash_t *wsman_options_get_properties(void)
{
	int c = 0;
	hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);

	while (properties != NULL && properties[c] != NULL) {
		char *cc[3];
		u_tokenize1(cc, 2, properties[c], '=');
		if (!hash_alloc_insert(h, cc[0], cc[1])) {
			debug("hash_alloc_insert failed");
		}
		c++;
	}
	return h;
}

static int wsman_options_get_action(void)
{
	int op = 0;
	int i;
	for (i = 0; action_data[i].action != NULL; i++) {
		if (strcmp(action_data[i].action, _action) == 0) {
			op = action_data[i].value;
			break;
		}
	}
	return op;
}

static int wsman_options_get_delivery_mode(void)
{
	int mode = 0;
	int i;
	for (i = 0; delivery_mode[i].action != NULL; i++) {
		if (strcmp(delivery_mode[i].action, event_delivery_mode) == 0) {
			mode = delivery_mode[i].value;
			break;
		}
	}
	return mode;
}

static int wsman_read_client_config(dictionary * ini)
{
	if (iniparser_find_entry(ini, "client")) {
		agent = iniparser_getstr(ini, "client:agent");
		server_port = server_port ?
		    server_port : iniparser_getint(ini, "client:port", 80);
		authentication_method = authentication_method ?
		    authentication_method :
		    iniparser_getstr(ini, "client:authentication_method");
	} else {
		return 0;
	}
	return 1;
}


int main(int argc, char **argv)
{
	int retVal = 0;
	int op;
	char *filename;
	dictionary *ini = NULL;
	WsManClient *cl;
	WsXmlDocH doc;
	char *enumContext = NULL;
	WsXmlDocH rqstDoc;
	client_opt_t *options;
	WsXmlDocH enum_response;
	WsXmlDocH resource;
	char *enumeration_mode, *binding_enumeration_mode,
	    *resource_uri_with_selectors;
	char *event_mode, *delivery_uri;
	char *resource_uri = NULL;

	filename = (char *) config_file;
	if (filename) {
		ini = iniparser_new(filename);
		if (ini == NULL) {
			fprintf(stderr, "cannot parse file [%s]",
				filename);
			exit(EXIT_FAILURE);
		} else if (!wsman_read_client_config(ini)) {
			fprintf(stderr, "Configuration file not found\n");
			exit(EXIT_FAILURE);
		}
	}
	if (!wsman_parse_options(argc, argv)) {
		exit(EXIT_FAILURE);
	}
	initialize_logging();
	//      wsmc_transport_init(NULL);
	options = wsmc_options_init();

	debug("Certificate: %s", cainfo);

	if (endpoint) {
		cl = wsmc_create_from_uri(endpoint);
	} else {
		cl = wsmc_create(server,
				server_port,
				url_path,
				cainfo? "https" : "http",
				username,
				password);
	}

	wsmc_transport_set_auth_request_func(cl ,  &request_usr_pwd );


	if (cl == NULL) {
		error("Null Client");
		exit(EXIT_FAILURE);
	}
	// transport options
	wsman_transport_set_auth_method(cl, authentication_method);
	if (proxy) {
		wsman_transport_set_proxy(cl, proxy);
		if (proxy_upwd) {
			wsman_transport_set_proxyauth(cl, proxy_upwd);
		}
	}

	if (cainfo) {
		wsman_transport_set_cainfo(cl, cainfo);
	}
	if (cert) {
		wsman_transport_set_cert(cl, cert);
	}
	if (sslkey) {
		wsman_transport_set_cert(cl, sslkey);
	}
	wsman_transport_set_verify_peer(cl, !noverify_peer);
	wsman_transport_set_verify_host(cl, !noverify_host);
	wsman_transport_set_timeout(cl, transport_timeout);

	// library options
	wsman_debug_set_level(debug_level);
	/*
	 * Setup Resource URI and Selectors
	 */
	resource_uri_with_selectors = resource_uri_opt;
	if (resource_uri_with_selectors &&
			strcmp(resource_uri_with_selectors,CIM_ALL_AVAILABLE_CLASSES) != 0) {
		wsmc_set_options_from_uri(resource_uri_with_selectors,
					   options);
		wsmc_remove_query_string(resource_uri_with_selectors,
					  &resource_uri);
	} else if (resource_uri_with_selectors) {
		wsmc_remove_query_string(resource_uri_with_selectors,
					  &resource_uri);
	}
	op = wsman_options_get_action();

	if (dump_request) {
		wsmc_set_action_option(options, FLAG_DUMP_REQUEST);
	}
	if (max_envelope_size) {
		options->max_envelope_size = max_envelope_size;
	}
	if (operation_timeout) {
		options->timeout = operation_timeout;
	}
	if (fragment) {
		options->fragment = fragment;
	}
	if (wsm_filter) {
		options->filter = wsm_filter;
	}
	if (wsm_dialect) {
		options->dialect = wsm_dialect;
	}
	options->properties = wsman_options_get_properties();
	options->cim_ns = cim_namespace;
	if (cim_extensions) {
		wsmc_set_action_option(options, FLAG_CIM_EXTENSIONS);
	}


	switch (op) {
	case WSMAN_ACTION_TEST:
		rqstDoc = wsmc_read_file(cl, input, "UTF-8", 0);
		wsman_send_request(cl, rqstDoc);
		doc = wsmc_build_envelope_from_response(cl);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_IDENTIFY:
		doc = wsmc_action_identify(cl, options);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_CUSTOM:

		doc = wsmc_action_invoke(cl, resource_uri, options,
				   invoke_method,
				   NULL);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_TRANSFER_DELETE:
		doc = wsmc_action_delete(cl, resource_uri, options);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_TRANSFER_CREATE:
		if (input) {
			resource = wsmc_read_file(cl, input, "UTF-8", 0);
			doc =
			    wsmc_action_create(cl, resource_uri, options,
					       resource);
			ws_xml_destroy_doc(resource);
			wsman_output(cl, doc);
			if (doc) {
				ws_xml_destroy_doc(doc);
			}
		} else {
			fprintf(stderr, "Missing resource data\n");
		}
		break;
	case WSMAN_ACTION_TRANSFER_PUT:
		if (input) {
			printf("input file provided\n");
			resource = wsmc_read_file(cl, input, "UTF-8", 0);
			doc =
			    wsmc_action_put(cl, resource_uri, options,
					    resource);
			ws_xml_destroy_doc(resource);
		} else {
			doc =
			    wsmc_action_get_and_put(cl, resource_uri,
						    options);
		}
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_TRANSFER_GET:
		doc = wsmc_action_get(cl, resource_uri, options);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_PULL:
		doc =
		    wsmc_action_pull(cl, resource_uri, options,
				enum_context);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_RELEASE:
		doc =
		    wsmc_action_release(cl, resource_uri, options,
				   enum_context);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_ENUMERATION:

		enumeration_mode = enum_mode;
		binding_enumeration_mode =
		    binding_enum_mode;

		if (enumeration_mode) {
			if (strcmp(enumeration_mode, "epr") == 0)
				wsmc_set_action_option(options,
							FLAG_ENUMERATION_ENUM_EPR);
			else if (strcmp(enumeration_mode, "objepr") == 0)
				wsmc_set_action_option(options,
							FLAG_ENUMERATION_ENUM_OBJ_AND_EPR);
		}
		if (binding_enumeration_mode) {
			if (strcmp(binding_enumeration_mode, "include") ==
			    0)
				wsmc_set_action_option(options,
							FLAG_IncludeSubClassProperties);
			else if (strcmp
				 (binding_enumeration_mode,
				  "exclude") == 0)
				wsmc_set_action_option(options,
							FLAG_ExcludeSubClassProperties);
			else if (strcmp(binding_enumeration_mode, "none")
				 == 0)
				wsmc_set_action_option(options,
							FLAG_POLYMORPHISM_NONE);
		}
		if (cim_references) {
			wsmc_set_action_option(options,
						FLAG_CIM_REFERENCES);
			options->dialect = WSM_ASSOCIATION_FILTER_DIALECT;
		}
		if (cim_associators) {
			wsmc_set_action_option(options,
						FLAG_CIM_ASSOCIATORS);
			options->dialect = WSM_ASSOCIATION_FILTER_DIALECT;
		}
		if (enum_optimize) {
			wsmc_set_action_option(options,
						FLAG_ENUMERATION_OPTIMIZATION);
		}
		options->max_elements = enum_max_elements;

		if (enum_estimate) {
			wsmc_set_action_option(options,
						FLAG_ENUMERATION_COUNT_ESTIMATION);
		}
		enum_response = wsmc_action_enumerate(cl, resource_uri, options);
		wsman_output(cl, enum_response);
		if (enum_response) {
			if (!(wsmc_get_response_code(cl) == 200 ||
			      wsmc_get_response_code(cl) == 400 ||
			      wsmc_get_response_code(cl) == 500)) {
				break;
			}
			enumContext = wsmc_get_enum_context(enum_response);
			ws_xml_destroy_doc(enum_response);
		} else {
			u_free(enumContext);
			break;
		}

		if (step)
			break;
		while (enumContext != NULL && enumContext[0] != 0) {

			doc = wsmc_action_pull(cl, resource_uri, options,
					enumContext);
			wsman_output(cl, doc);

			if (wsmc_get_response_code(cl) != 200
					&& wsmc_get_response_code(cl) != 400
					&& wsmc_get_response_code(cl) != 500) {
				u_free(enumContext);
				break;
			}
			u_free(enumContext);
			enumContext = wsmc_get_enum_context(doc);
			if (doc) {
				ws_xml_destroy_doc(doc);
			}
		}
		u_free(enumContext);
		break;
	case WSMAN_ACTION_SUBSCRIBE:
		event_mode = event_delivery_mode;
		delivery_uri = event_delivery_uri;
		if(event_sendbookmark)
			wsmc_set_action_option(options, FLAG_EVENT_SENDBOOKMARK);
		if(event_delivery_mode)
			options->delivery_mode = wsman_options_get_delivery_mode();
		if(event_delivery_uri)
			options->delivery_uri = event_delivery_uri;
		if(event_heartbeat)
			options->heartbeat_interval = event_heartbeat;
		if(event_subscription_expire)
			options->expires = event_subscription_expire;
		if(wsm_dialect)
			options->dialect = wsm_dialect;
		rqstDoc = wsmc_action_subscribe(cl, resource_uri, options);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	case WSMAN_ACTION_UNSUBSCRIBE:
		rqstDoc = wsmc_action_unsubscribe(cl, resource_uri, options, event_subscription_id);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	case WSMAN_ACTION_RENEW:
		if(event_subscription_expire)
			options->expires = event_subscription_expire;
		rqstDoc = wsmc_action_renew(cl, resource_uri, options, event_subscription_id);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	case WSMAN_ACTION_EVENT_PULL:
		rqstDoc = wsmc_action_evt_pull(cl, resource_uri, options, enum_context);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	default:
		fprintf(stderr, "Action not supported\n");
		retVal = 1;
	}


	if (wsmc_get_response_code(cl) != 200) {
		fprintf(stderr, "Connection failed. response code = %ld\n",
			wsmc_get_response_code(cl));
		if (wsmc_get_fault_string(cl)) {
			fprintf(stderr, "%s\n",
				wsmc_get_fault_string(cl));
		}
	}
	wsmc_options_destroy(options);
	wsmc_transport_fini(cl);
	wsmc_release(cl);
	if (ini) {
		iniparser_free(ini);
	}
#ifdef DEBUG_VERBOSE
	printf("     ******   Transfer Time = %ull usecs ******\n",
	       get_transfer_time());
#endif
	return retVal;

}
