#include "wsman-client-api.h"

int main(int argc, char** argv)
{
	int		sid;
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
		return 0;
	}
	u_error_free(error);

	if (!user || !passwd) {
		printf("\t new_api_example: user and passwd are required\n");
		return 0;
	}

	sid = wsman_session_open("localhost", 8889, "/wsman", "http",
				user, passwd, 0);

	if (sid < 0) {
		printf("Open session failed\n");
		return 0;
	}


	printf("\n******** Opened session id %d ********\n\n", sid);

	response = wsman_session_identify(sid, 0);
	if (!response) {
		printf("******** Identify failed - %s ********\n\n",
			wsman_session_error(sid));
		goto end;
	}
	printf("******** Identify response *******\n%s\n", response);

 end:
	wsman_session_close(sid);
	printf("******** Closed session id %d ********\n\n", sid);

	return retval;
}
