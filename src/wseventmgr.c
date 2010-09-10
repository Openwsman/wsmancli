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
 * @author Vadim Revyakin
 * @author Liang Hou
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
static char *url_path = NULL;
static char *authentication_method = NULL;
static char noverify_peer = 0;
static char noverify_host = 0;

static int  transport_timeout = 0;
static char *proxy = NULL;
static char *proxy_upwd = NULL;


static int debug_level = -1;
static char *encoding = NULL;
char dump_request = 0;
char request_only = 0;

static char *wsm_filter = NULL;
static char *wsm_dialect = NULL;
static char *event_delivery_mode = NULL;
static char *event_delivery_sec_mode = NULL;
static char *event_delivery_uri = NULL;
static int event_subscription_expire = 0;
static int event_heartbeat = 0;
static int event_sendbookmark =0;
static char *event_subscription_id = NULL;
static char *event_reference_properties = NULL;
static char *event_username = NULL;
static char *event_password = NULL;
static char *event_thumbprint = NULL;
static char *enum_context = NULL;
static int eventsink_enabled = 0;

static char *cim_namespace = NULL;

static char *_action = NULL;
static char *config_file = NULL;
static char *output_file = NULL;
static char *resource_uri_opt = NULL;

struct _WsActions {
	char *action;
	int value;
};
typedef struct _WsActions WsActions;

WsActions action_data[] = {
	{"pull", WSMAN_ACTION_PULL},
	{"subscribe", WSMAN_ACTION_SUBSCRIBE},
	{"unsubscribe", WSMAN_ACTION_UNSUBSCRIBE},
	{"renew", WSMAN_ACTION_RENEW},
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

WsActions delivery_sec_mode[] = {
	{"httpbasic", WSMAN_DELIVERY_SEC_HTTP_BASIC},
	{"httpdigest", WSMAN_DELIVERY_SEC_HTTP_DIGEST},
	{"httpsbasic", WSMAN_DELIVERY_SEC_HTTPS_BASIC},
	{"httpsdigest", WSMAN_DELIVERY_SEC_HTTPS_DIGEST},
	{"httpsmutual", WSMAN_DELIVERY_SEC_HTTPS_MUTUAL},
	{"httpsmutualbasic", WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_BASIC},
	{"httpsmutualdigest", WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_DIGEST},
	{NULL, 0}
};

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

static char wsman_parse_options(int argc, char **argv)
{
	char retval = 0;
	u_error_t *error = NULL;

	u_option_entry_t options[] = {
		{"debug", 'd', U_OPTION_ARG_INT, &debug_level,
		 "Set the verbosity of debugging output.", "1-6"},
		 {"print-request", 'R', U_OPTION_ARG_NONE, &dump_request,
                 "print request on stdout", NULL},
		{"encoding", 'j', U_OPTION_ARG_STRING, &encoding,
		"Set request message encoding"},
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


	u_option_entry_t event_options[] = {
		{"filter", 'x', U_OPTION_ARG_STRING, &wsm_filter,
		 "Filter", "<filter>"},
		{"dialect", 'D', U_OPTION_ARG_STRING, &wsm_dialect,
		 "Filter Dialect", "<dialect>"},
		{"delivery-mode", 'G', U_OPTION_ARG_STRING, &event_delivery_mode,
		"Four delivery modes available: push/pushwithack/events/pull",
		"<mode>"},
		{"delivery-sec-mode", 's', U_OPTION_ARG_STRING, &event_delivery_sec_mode,
                "Four delivery modes available: httpbasic/httpdigest/httpsbasic/httpsdigest/httpsmutual/httpsmutualbasic/httpsmutualdigest",
                "<mode>"},
		{"delivery-username", 'U', U_OPTION_ARG_STRING, &event_username,
		"username for the eventing receiver",
		"<username>"},
		{"delivery-password", 'P', U_OPTION_ARG_STRING, &event_password,
                "password for the eventing receiver",
                "<password>"},
		{"delivery-thumbprint", 'T', U_OPTION_ARG_STRING, &event_thumbprint,
                "ceritificate thumbprint of the eventing receiver",
                "<thumbprint>"},
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
		{"event-reference-properties", 'r', U_OPTION_ARG_STRING, &event_reference_properties,
		"Event Reference Properties, correlation of Events with Subscription",
		"<xml string>"},
		{"enum-context", 'C', U_OPTION_ARG_STRING, &enum_context,
                 "Events enumeration Context (For use with Pull)",
                 "<enum context>"},
		{"enable-eventsink", 'E', U_OPTION_ARG_NONE, &eventsink_enabled,
		"Enable this tool as event sink",
		NULL},
		{NULL}
	};

	u_option_group_t *event_group;
	u_option_group_t *req_flag_group;

	u_option_context_t *opt_ctx;
	opt_ctx = u_option_context_new("<action> <Resource Uri>");
	event_group = u_option_group_new("event", "Event subscription", "Subscription Options");
	req_flag_group =
	    u_option_group_new("flags", "Flags", "Request Flags");

	u_option_group_add_entries(event_group, event_options);

	u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
	u_option_context_add_main_entries(opt_ctx, options, "wsman");
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
				strcmp(argv[1], "anonid") == 0 ||
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
		server_port = cainfo ? 5985 : 5986;
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

static int wsman_options_get_delivery_sec_mode(void)
{
        int mode = 0;
        int i;
        for (i = 0; delivery_sec_mode[i].action != NULL; i++) {
                if (strcmp(delivery_sec_mode[i].action, event_delivery_sec_mode) == 0) {
                        mode = delivery_sec_mode[i].value;
                        break;
                }
        }
        return mode;
}


int main(int argc, char **argv)
{
	int retVal = 0;
	int op;
	WsManClient *cl;
	WsXmlDocH doc;
	WsXmlDocH rqstDoc;
	client_opt_t *options;
	char *resource_uri_with_selectors;
	char *event_mode, *delivery_uri;
	char *resource_uri = NULL;
	char subscontext[512];
	filter_t *filter = NULL;
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
	if (encoding) {
		wsmc_set_encoding(cl, encoding);
	}
	if (dump_request) {
		wsmc_set_action_option(options, FLAG_DUMP_REQUEST);
	}
	if (wsm_filter) {
		filter = filter_create_simple(wsm_dialect, wsm_filter );
	}
	options->cim_ns = cim_namespace;

	switch (op) {
	case WSMAN_ACTION_PULL:
		doc = wsmc_action_pull(cl, resource_uri, options, filter,
				enum_context);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_SUBSCRIBE:
		event_mode = event_delivery_mode;
		delivery_uri = event_delivery_uri;
		if(event_sendbookmark)
			wsmc_set_action_option(options, FLAG_EVENT_SENDBOOKMARK);
		if(event_delivery_mode)
			options->delivery_mode = wsman_options_get_delivery_mode();
		if(event_delivery_sec_mode)
                        options->delivery_sec_mode = wsman_options_get_delivery_sec_mode();
		if(event_username)
			options->delivery_username = event_username;
		if(event_password)
			options->delivery_password = event_password;
		if(event_thumbprint)
			options->delivery_certificatethumbprint = event_thumbprint;
		if(event_delivery_uri)
			options->delivery_uri = event_delivery_uri;
		if(event_heartbeat)
			options->heartbeat_interval = event_heartbeat;
		if(event_subscription_expire)
			options->expires = event_subscription_expire;
		/*
		if(wsm_dialect)
			options->dialect = wsm_dialect;
			*/
		if(event_reference_properties)
			options->reference = event_reference_properties;
		rqstDoc = wsmc_action_subscribe(cl, resource_uri, options, filter);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	case WSMAN_ACTION_UNSUBSCRIBE:
		snprintf(subscontext, 512 , "<wsa:ReferenceParameters xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" \
			xmlns:wse=\"http://schemas.xmlsoap.org/ws/2004/08/eventing\"><wse:Identifier>%s</wse:Identifier> \
			</wsa:ReferenceParameters>", event_subscription_id);
		rqstDoc = wsmc_action_unsubscribe(cl, resource_uri, options, subscontext);
		wsman_output(cl, rqstDoc);
		if (rqstDoc) {
			ws_xml_destroy_doc(rqstDoc);
		}
		break;
	case WSMAN_ACTION_RENEW:
		if(event_subscription_expire)
			options->expires = event_subscription_expire;
		snprintf(subscontext, 512 , "<wsa:ReferenceParameters xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" \
                        xmlns:wse=\"http://schemas.xmlsoap.org/ws/2004/08/eventing\"><wse:Identifier>%s</wse:Identifier> \
                        </wsa:ReferenceParameters>", event_subscription_id);
		rqstDoc = wsmc_action_renew(cl, resource_uri, options, subscontext);
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
	if(op == WSMAN_ACTION_PULL && strcmp(event_delivery_mode, "pull") \
		&& eventsink_enabled) {
// to do here:  start related event sink service according to options
//		start_eventsink(options);
	}
#ifdef DEBUG_VERBOSE
	printf("     ******   Transfer Time = %ull usecs ******\n",
	       get_transfer_time());
#endif
	return retVal;

}
