#include "wsman-client-api.h"

static char *endpoint = NULL;

int main(int argc, char** argv)
{
	int		sid;
	int		eid;
	int		sid1;
	int		i = 0;
	char		*response;
	char 		retval = 0;
	const char	*resource_uri =
	"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem";
	u_error_t 	*error = NULL;
	char		*user = NULL;
	char		*passwd = NULL;

	u_option_entry_t opt[] = {
	{ "user",	'u',	U_OPTION_ARG_STRING,	&user,
		"user name", "<user>" },
	{ "passwd",	'p',	U_OPTION_ARG_STRING,	&passwd,
		"password", "<passwd>" },
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

	if (!user || !passwd) {
		printf("\t new_api_example: user and passwd are required\n");
		return 1;
	}

	sid = wsman_session_open("localhost", 8889, "/wsman", "http",
				user, passwd, 0);

	if (sid < 0) {
		printf("Open session failed\n");
		return 0;
	}


	printf("\n******** Opened session id %d ********\n\n", sid);

	eid = wsman_session_enumerate(sid, resource_uri, NULL, NULL,
						FLAG_ENUMERATION_ENUM_EPR);

	if (eid < 0) {
		printf("******** Enumeration failed - %s ********\n\n",
			wsman_session_error(sid));
		return 0;
	}

	while (wsman_enumerator_end(eid)) {
		i++;
		response = wsman_enumerator_pull(eid);
		if (!response) {
			printf("******** Pull (%d) failed - %s ********\n\n",
			i, wsman_session_error(eid));
			break;
		}
		printf("******** Pull response (%d) *******\n%s\n", i,
			response);
		sid1 = wsman_session_resource_locator_new(sid, response);
		response = wsman_session_transfer_get(sid1, 0);

		if (!response) {
			printf("******** Transfer Get failed - %s ********\n\n",
				wsman_session_error(sid1));
			goto continuep;
		}
		printf ("******** Transfer Get response ********\n%s\n",
			response);

		response = wsman_session_transfer_put(sid1, response, 0);

		if (!response) {
			printf("******** Transfer Put failed - %s ********\n\n",
				wsman_session_error(sid1));
			goto continuep;
		}
		printf ("******** Transfer Put response ********\n%s\n",
			response);
  continuep:
		wsman_session_close(sid1);
	}

	wsman_session_close(sid);

	printf("******** Closed session id %d ********\n\n", sid);

	return 1;
}
