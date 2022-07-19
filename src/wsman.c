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

#include <wsman-client.h>
#include <wsman-client-transport.h>
#include <wsman-debug.h>
#include <u/libu.h>

#if __linux__
extern char *getpass (const char *__prompt);
#endif

static long int server_port = 0;
static char *cainfo = NULL;
static char *cert = NULL;
static char *sslkey = NULL;
static char *endpoint = NULL;
static char *username = NULL;
static char *username_given = NULL; /* copy of either the username from env or cmdline*/
static char *username_prev = NULL; /* input username to request_usr_pwd() when called last time */
static char *password = NULL;
static char *password_given = NULL; /* copy of either the password from env or cmdline */
static char *password_prev = NULL; /* input password to request_usr_pwd() when called last time */
static char *server = "localhost";
static char *agent = NULL;
static char *url_path = NULL;
static char *authentication_method = NULL;
static char noverify_peer = 0;
static char noverify_host = 0;

static long int  transport_timeout = 0;
static char *proxy = NULL;
static char *proxy_upwd = NULL;


static long int non_interactive = 0;
static long int debug_level = -1;
static char *encoding = NULL;
static char *test_case = NULL;
static long int enum_max_elements = 0;
char enum_optimize = 0;
char enum_estimate = 0;
char dump_request = 0;
char step = 0;
char cim_extensions = 0;
static char *enum_mode = NULL;
static char *binding_enum_mode = NULL;
static char *enum_context = NULL;
static char *event_delivery_mode = NULL;
static char *event_delivery_sec_mode = NULL;
static char *event_delivery_uri = NULL;
static long int event_subscription_expire = 0;
static long int event_heartbeat = 0;
static int event_sendbookmark =0;
static char *event_subscription_id = NULL;
static char *event_reference_properties = NULL;
static char *event_username = NULL;
static char *event_password = NULL;
static char *event_thumbprint = NULL;

static char *cim_namespace = NULL;
static char *fragment = NULL;
static char *locale = NULL;
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
static char **option_set_values = NULL;

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
	{"anonid", WSMAN_ACTION_ANON_IDENTIFY},
	{"subscribe", WSMAN_ACTION_SUBSCRIBE},
	{"unsubscribe", WSMAN_ACTION_UNSUBSCRIBE},
	{"renew", WSMAN_ACTION_RENEW},
	{"associators", WSMAN_ACTION_ASSOCIATORS},
	{"references", WSMAN_ACTION_REFERENCES},
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
	char my_version = 0;

	u_option_entry_t options[] = {
		{"non-interactive", 0, U_OPTION_ARG_NONE, &non_interactive,
			"Non interactive mode, don't ask for credentials", NULL},
		{"version", 'q', U_OPTION_ARG_NONE, &my_version,
			"Display application version", NULL},
		{"debug", 'd', U_OPTION_ARG_INT, &debug_level,
			"Set the verbosity of debugging output.", "1-6"},
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
			"Service Path (default: 'wsman')", "<path>"},
		{"input", 'J', U_OPTION_ARG_STRING, &input,
			"File with resource for Create and Put operations in XML, can be a SOAP envelope",
			"<filename>"},
		{"password", 'p', U_OPTION_ARG_STRING, &password,
			"User Password", "<password>"},
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
		{"option", (char)0, U_OPTION_ARG_STRING_ARRAY, &option_set_values,
			"Option with key value pair for OptionSet (For 'put', 'invoke' and 'create')",
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
			"Operation timeout in milliseconds", "<time in msec>"},
		{"max-envelope-size", 'e', U_OPTION_ARG_INT,
			&max_envelope_size,
			"maximal envelope size", "<size>"},
		{"fragment", 'F', U_OPTION_ARG_STRING, &fragment,
			"Fragment (Supported Dialects: XPATH)", "<fragment>"},
		{"locale", 'L', U_OPTION_ARG_STRING, &locale,
			"Locale for this request", "<RFC 5646 language code>"},
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
		{"delivery-sec-mode", 's', U_OPTION_ARG_STRING, &event_delivery_sec_mode,
			"Four delivery modes available: httpbasic/httpdigest/httpsbasic/httpsdigest/httpsmutual/httpsmutualbasic/httpsmutualdigest",
			"<mode>"},
		{"delivery-username", 'n', U_OPTION_ARG_STRING, &event_username,
			"username for the eventing receiver",
			"<username>"},
		{"delivery-password", 'z', U_OPTION_ARG_STRING, &event_password,
			"password for the eventing receiver",
			"<password>"},
		{"delivery-thumbprint", 'Y', U_OPTION_ARG_STRING, &event_thumbprint,
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
		{"event-reference-properties", 'L', U_OPTION_ARG_STRING, &event_reference_properties,
			"Event Reference Properties, correlation of Events with Subscription",
			"<xml string>"},
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
		{NULL}
	};

	u_option_entry_t test_options[] = {
		{"from-file", 'f', U_OPTION_ARG_STRING, &test_case,
			"Send request from file", "<file name>"},
		{"print-request", 'R', U_OPTION_ARG_NONE, &dump_request,
			"print request on stdout", NULL},
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

	if (retval == 0) {
          if (error) {
		if (error->message)
			fprintf(stderr, "%s\n", error->message);
		u_error_free(error);
          }
          else {
		fprintf(stderr, "Can't parse context information\n");
          }
          return FALSE;
	}
        else if (retval == 2) { /* help */
          exit(0);
        }

	if (my_version) {
		fprintf(stdout, PACKAGE_STRING " (" PACKAGE_BUILDTS ")\n\n");
		exit(0);
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
		server_port = cainfo ? 5986 : 5985;
	}
	if (url_path == NULL) {
		if (strcmp(argv[1], "anonid") == 0)
			url_path = "/wsman-anon/identify";
		else
			url_path = "/wsman";
	}
	return TRUE;
}


static void wsman_output(WsManClient * cl, WsXmlDocH doc)
{
	FILE *f = stdout;
	const char *filename = output_file;
	const char *badxml = NULL;
	WS_LASTERR_Code err;

	err = wsmc_get_last_error(cl);
	if (err != WS_LASTERR_OK) {
		return;
	}
	if (!doc) {
		error("doc with NULL content");
		badxml = (char*)u_buf_ptr(cl->connection->response);
		if ((-1 == debug_level) || (NULL == badxml)) {
			goto error1;
		}
	}
	if (filename) {
		f = fopen(filename, "w");
		if (f == NULL) {
			error("Could not open file for writing");
			goto error1;
		}
	}
	if (NULL != badxml) {
		fprintf(f, "%s", badxml); // on debug mode, output the bad xml
	} else {
		ws_xml_dump_node_tree(f, ws_xml_get_doc_root(doc));
	}
	if (f != stdout) {
		fclose(f);
	}

error1:
	return;
}

/*
 * output pull results to separate files (appending "-<index>" to the name)
 * 
 */
static void wsman_output_pull(WsManClient * cl, WsXmlDocH doc, int index)
{
	char *strbuf, *origfile = output_file;
	int count;

	if (output_file) {
		count = strlen(output_file) + 16;
		strbuf = (char*)calloc(count, 1);
		snprintf(strbuf, count, "%s-%u.xml", output_file, index);
		output_file = strbuf;
	}
	wsman_output(cl, doc);
	if (origfile) {
		output_file = origfile;
		free(strbuf);
	}
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

    /*
	 * fprintf(stdout,"Authentication failed, please retry\n");
     *
	 * this message shall not be printed by this function as it cannot decide on the
	 * reason it was called for. It does not control the authentication process.
	 * wsmc_handler is better suited for such a decision making.
	 */

    if (username_given) {
		if (password_given) {
			/* Initially provided combination of password and username is not valid.
			 * Request user to type both. Here I assume, that wsmc_handler called back to
			 * this function after trying a first authentication using these credentials.
			 */
		} else {
            /* Initially no password was provided => no authentication tried during first
			 * iteration of while loop in wsmc_handler. Check previously typed credentials
			 */
			if (username_prev) {
			   /* This is a second call of this function, assuming only wsmc_handler is using it
			    * as a callback function. Therefore, there must have been a previous attempt to
				* authenticate, but this previous combination of username and password did not
				* lead to a successful authentication. Request new credentials, username_prev will
				* be set each time after user has provided a username.
				*/
			} else {
				/* First time wsmc_handler calls back to this function. No password given on the
				 * command line or via the environment variable. Therefore wsmc_handler cannot
				 * have tried http authentication. A username was given on the command line or
				 * via an environment variable. And the user wants us to try this name at least
				 * at first. So, let's do him a favour and use it. When we are called back again,
				 * we will ask the user to provide a new name or the same, but a different password.
				 */
				*username = u_strdup(username_given);
			}
		}
	}

    if (*username == NULL) {
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
	}

	/* after successfull receipt of a new username, store a copy at username_prev */
	if (*username) {
		if ( username_prev ) {
			u_free(username_prev);
			username_prev = NULL;
	}
		username_prev = u_strdup(*username);
	}

    /* but always ask for the password !? */
	pw = (char *)getpass("Password: ");
	*password = u_strdup_printf ("%s", pw);

    /* make backup, *password will become free'd when next try of http-auth fails */
    if (*password) {
		if (password_prev) {
			u_free(password_prev);
			password_prev = NULL;
		}
		password_prev = u_strdup(*password);
	}
}

static void
wsman_options_set_properties(client_opt_t *options)
{
	int c = 0;

	while (properties != NULL && properties[c] != NULL) {
		char *cc[3] = { NULL, NULL, NULL };
		u_tokenize1(cc, 2, properties[c], '=');
                wsmc_add_property(options, cc[0], cc[1]);
		c++;
	}
	return;
}


static void
wsman_options_set_option_set_values(client_opt_t *options)
{
	int c = 0;

	while (option_set_values != NULL && option_set_values[c] != NULL) {
		char *cc[3] = { NULL, NULL, NULL };
		u_tokenize1(cc, 2, option_set_values[c], '=');
                wsmc_add_option(options, cc[0], cc[1]);
		c++;
	}
	return;
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


static int wsman_read_client_config(dictionary * ini)
{
	if (iniparser_find_entry(ini, "client")) {
		agent = iniparser_getstr(ini, "client:agent");
		server_port = server_port ?
			server_port : iniparser_getint(ini, "client:port", 5985);
		authentication_method = authentication_method ?
			authentication_method :
			iniparser_getstr(ini, "client:authentication_method");
	} else {
		return 0;
	}
	return 1;
}

static void free_include_result_property(char **resultProps)
{
       if (NULL != resultProps) {
               char **tmp = resultProps;
               while (*tmp != NULL) { // iterate until list terminator
                       u_free(*tmp);
                       tmp++;
               }
               u_free(resultProps);
       }
}

static char ** get_include_result_property(int *propNum)
{
       char **resultProps = NULL;
       char *tok, *val, *copy;
       int idx = 0;

       *propNum = 0;
       if (NULL != enum_context) {
               copy = u_strdup(enum_context);
               for (tok = copy ; NULL != tok ; (*propNum)++, tok = strchr(tok, ',')) { // get count
                       tok++;
               }
               resultProps = (char **)u_calloc((*propNum + 1), sizeof(char *)); // 1 more for list terminator
               val = copy;
               while (val) {
                       tok = strchr(val, ',');
                       if (NULL != tok) {
                               *tok++ = '\0';
                       }
                       resultProps[idx++] = u_strdup(val);
                       val = tok;
               }
               resultProps[idx] = NULL; // list terminator
               u_free(copy);
       }
       return resultProps;
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
	WsXmlDocH resource = NULL;
	char *enumeration_mode, *binding_enumeration_mode,
		 *resource_uri_with_selectors;
	char *resource_uri = NULL;
	char subscontext[512];
	filter_t *filter = NULL;

        /* read credentials from environment */
        username = getenv("WSMAN_USER");
        password = getenv("WSMAN_PASS");
        event_username = getenv("WSMAN_EVENT_USER");
        event_password = getenv("WSMAN_EVENT_PASS");

        /* parse command line options
           might overwrite environment credentials */

	if (!wsman_parse_options(argc, argv)) {
		exit(EXIT_FAILURE);
	}

    /* save copies of username or password when given on the command line or via environment variables */
	if ( username != NULL ) {
		username_given = u_strdup(username);
	}
	if ( password != NULL ) {
		password_given = u_strdup(password);
	}

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
				cainfo ? "https" : "http",
				username,
				password);
	}

        if (non_interactive == 0) {
          wsmc_transport_set_auth_request_func(cl ,  &request_usr_pwd );
        }

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
		if (!cainfo)
			fprintf(stderr, "Warning: --cacert not set to enable SSL operation\n");
	}
	if (sslkey) {
		wsman_transport_set_key(cl, sslkey);
		if (!cainfo)
			fprintf(stderr, "Warning: --cacert not set to enable SSL operation\n");
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
	if (max_envelope_size) {
		options->max_envelope_size = max_envelope_size;
	}
	if (operation_timeout) {
		options->timeout = operation_timeout;
	}
	if (fragment) {
		options->fragment = fragment;
	}
	if (locale) {
		options->locale = locale;
	}

	wsman_options_set_properties(options);
	wsman_options_set_option_set_values(options);

	options->cim_ns = cim_namespace;
	if (cim_extensions) {
		wsmc_set_action_option(options, FLAG_CIM_EXTENSIONS);
	}

	switch (op) {
	case WSMAN_ACTION_TEST:
		rqstDoc = wsmc_read_file(input, wsmc_get_encoding(cl), 0);
		wsman_send_request(cl, rqstDoc);
		doc = wsmc_build_envelope_from_response(cl);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_ANON_IDENTIFY:
	case WSMAN_ACTION_IDENTIFY:
		doc = wsmc_action_identify(cl, options);
		wsman_output(cl, doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
		break;
	case WSMAN_ACTION_CUSTOM:
		if (input) {
			resource = wsmc_read_file(input, wsmc_get_encoding(cl), 0);
		}
		doc = wsmc_action_invoke(cl, resource_uri, options,
				invoke_method,
				resource);
		ws_xml_destroy_doc(resource);
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
			resource = wsmc_read_file(input, "UTF-8", 0);
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
			resource = wsmc_read_file(input, wsmc_get_encoding(cl), 0);
			doc =  wsmc_action_put(cl, resource_uri, options,
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
		doc = wsmc_action_pull(cl, resource_uri, options, filter, enum_context);
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
	case WSMAN_ACTION_ASSOCIATORS:
	case WSMAN_ACTION_REFERENCES:
	case WSMAN_ACTION_ENUMERATION:
		if ( op == WSMAN_ACTION_REFERENCES || op == WSMAN_ACTION_ASSOCIATORS ) {
			if (wsm_filter) {
				epr_t *epr = epr_from_string(wsm_filter);
				if(options->cim_ns) {
					epr_add_selector_text(epr, CIM_NAMESPACE_SELECTOR, options->cim_ns);
				}
				if (epr) {
					char **resultProperties = NULL;
					const char *assocClass, *resultClass, *role, *resultRole;
					int propNum;

					if (NULL != (assocClass = wsman_epr_selector_by_name(epr, "AssociationClassName")))
						epr_delete_selector(epr, "AssociationClassName");
					if (NULL != (resultClass = wsman_epr_selector_by_name(epr, "ResultClassName")))
						epr_delete_selector(epr, "ResultClassName");
					if (NULL != (role = wsman_epr_selector_by_name(epr, "Role")))
						epr_delete_selector(epr, "Role");
					if (NULL != (resultRole = wsman_epr_selector_by_name(epr, "ResultRole")))
						epr_delete_selector(epr, "ResultRole");
					resultProperties = get_include_result_property(&propNum);

					filter = filter_create_assoc(epr, (op == WSMAN_ACTION_ASSOCIATORS )?0:1,
								     assocClass, resultClass, role, resultRole, resultProperties, propNum);

					free_include_result_property(resultProperties);
				}
			} else {
				error("Filter Requied");
			}
		} else if (wsm_dialect && strcmp(wsm_dialect, WSM_SELECTOR_FILTER_DIALECT) == 0 ) {
			// fixme: Namespace
			if (wsm_filter) {
				hscan_t hs;
				hnode_t *hn;
				hash_t *selfilter = NULL;
				hash_t *selectors_new = NULL;
                                key_value_t *entry;
				selectors_new = hash_create2(HASHCOUNT_T_MAX, 0, 0);
				selfilter = u_parse_query(wsm_filter);
                                if (!selfilter) {
                                  error("Filter parse error");
                                  break;
                                }
				hash_scan_begin(&hs, selfilter);
				while ((hn = hash_scan_next(&hs))) {
                                        entry = u_malloc(sizeof(key_value_t));
					entry->type = 0;
					entry->v.text = (char *)hnode_get(hn);
					hash_alloc_insert(selectors_new, hnode_getkey(hn), entry);
				}

				if (hash_count(selectors_new) > 0 )
					filter = filter_create_selector(selectors_new);
			}
		} else {
			filter = filter_create_simple(wsm_dialect, wsm_filter);
		}
		enumeration_mode = enum_mode;
		binding_enumeration_mode = binding_enum_mode;

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
		if (enum_optimize) {
			wsmc_set_action_option(options,
					FLAG_ENUMERATION_OPTIMIZATION);
		}
		options->max_elements = enum_max_elements;

		if (enum_estimate) {
			wsmc_set_action_option(options,
					FLAG_ENUMERATION_COUNT_ESTIMATION);
		}
		enum_response = wsmc_action_enumerate(cl, resource_uri, options, filter);
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
			break;
		}

		if (step)
			break;

      int index = 0;
		while (enumContext != NULL && enumContext[0] != 0) {

			doc = wsmc_action_pull(cl, resource_uri, options, filter,
					enumContext);
			wsman_output_pull(cl, doc, ++index);

			if (wsmc_get_response_code(cl) != 200
					&& wsmc_get_response_code(cl) != 400
					&& wsmc_get_response_code(cl) != 500) {
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
		if(wsm_filter)
			filter = filter_create_simple(wsm_dialect, wsm_filter);

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
                retVal = 1;
	}
	wsmc_options_destroy(options);
	filter_destroy(filter);
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
