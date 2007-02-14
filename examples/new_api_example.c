#include "wsman-client-api.h"

static char *endpoint = NULL;

int main(int argc, char** argv)
{
	int		sid;
	int		i = 1;
	char		*response;
	char 		retval = 0;
	u_error_t 	*error = NULL;
	u_uri_t		*uri;
	const char	*resource_uri =
	"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem";

	u_option_entry_t opt[] = {
	{ "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
		"Endpoint in form of a URL", "<uri>" },
	{ NULL }
	};


	u_option_context_t *opt_ctx;	
	opt_ctx = u_option_context_new("");
	u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
	u_option_context_add_main_entries(opt_ctx, opt, "adv api example");
	retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);
	u_option_context_free(opt_ctx);

	if (error) {
		if (error->message)
		printf ("%s\n", error->message);
		u_error_free(error);
		return 1;
	}
	u_error_free(error);

	if (endpoint) {
		u_uri_parse((const char *)endpoint, &uri);
	}
	if (!endpoint || !uri) {
		fprintf(stderr, "endpoint option required\n");
		return 1;
	}

	

	sid = wsman_session_open(uri->host, uri->port, uri->path, uri->scheme,
				uri->user, uri->pwd, 0);

/*	sid = wsman_session_open("localhost", 8889, "/wsman", "http", "den","den", 0);*/

	if (sid < 0) {
		printf("Open session failed\n");
		return 0;
	}


	printf("\n******** Opened session id %d ********\n\n", sid);

	response = wsman_session_enumerate(sid, resource_uri, NULL, NULL, 0);

	if (!response) {
		printf("******** Enumeration failed - %s ********\n\n",
			wsman_session_error(sid));
		return 0;
	}

	printf ("******** Enumeration response (id %d) ********\n%s\n",
					sid, response);

	while (wsman_session_enumerator_end(sid)) {
		response = wsman_session_enumerator_pull(sid);
		if (!response) {
			printf("******** Pull (%d) failed - %s ********\n\n",
			i, wsman_session_error(sid));
			break;
		}	
		printf("******** Pull response (%d) *******\n%s\n", i, response);
		i++;
	}

const char *ruri =
	"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem?Name=mstrcsd013.ims.intel.com&CreationClassName=OMC_UnitaryComputerSystem";

	response = wsman_session_transfer_get(sid, ruri, 0);

	if (!response) {
		printf("******** Transfer Get failed - %s ********\n\n",
			wsman_session_error(sid));
	}

	printf ("******** Transfer Get (first response) (id %d) ********\n%s\n",
					sid, response);

	wsman_session_locator_resource_uri(sid, resource_uri);
	wsman_session_locator_add_selector(sid,
					"Name",
					"mstrcsd013.ims.intel.com");
	wsman_session_locator_add_selector(sid,
					"CreationClassName",
					"OpenWBEM_UnitaryComputerSystem");

	response = wsman_session_transfer_get(sid, NULL, 0);

	if (!response) {
		printf("******** Transfer Get failed - %s ********\n\n",
			wsman_session_error(sid));
	}

	printf ("******** Transfer Get (second response) (id %d) ********\n%s\n",
					sid, response);

	wsman_session_close(sid);

	printf("******** Closed session id %d ********\n\n", sid);

	return 1;
}
