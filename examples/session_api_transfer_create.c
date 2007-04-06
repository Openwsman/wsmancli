#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"

#define CLASSNAME "EXL_ExamplePolicy"

SER_TYPEINFO_UINT32;

struct __EXL_ExamplePolicy
{
    XML_TYPE_STR ElementName;
    XML_TYPE_STR Description;
    XML_TYPE_STR Caption;
    XML_TYPE_STR InstanceID;
    XML_TYPE_STR PolicyName;
    XML_TYPE_UINT32   PolicyPrecedence;
    XML_TYPE_DYN_ARRAY Handles;
    XML_TYPE_BOOL   DefaultTest;

};
typedef struct __EXL_ExamplePolicy EXL_ExamplePolicy;

SER_START_ITEMS(EXL_ExamplePolicy)
SER_STR("ElementName", 1),
SER_STR("Description", 1),
SER_STR("Caption", 1),
SER_STR("InstanceID", 1),
SER_STR("PolicyName", 1),
SER_UINT32("PolicyPrecedence", 1 ),
SER_DYN_ARRAY("Handles", 1, 10, uint32),
SER_BOOL("DefaultTest", 1),
SER_END_ITEMS(EXL_ExamplePolicy);

int main(int argc, char** argv)
{
	int		sid;
	int		i = 0;
	char		*res;
	const char	*resource_uri =
	"http://example.com/wbem/wscim/1/schema/1/EXL_ExamplePolicy";
	int		retval;
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
	wsman_session_resource_locator_set(sid, resource_uri);

	EXL_ExamplePolicy *d = u_malloc(sizeof(EXL_ExamplePolicy));
	d->ElementName = u_strdup("name");
	d->DefaultTest = 1;

	int *array = NULL;
	int count = 4;
	array = (int *) malloc (sizeof (int) * count);
	array[0] = 1;
	array[1] = 0;
	array[2] = 3;
	array[3] = 5;
	d->Handles.count = count;
	d->Handles.data = array;

	printf("\n******** Opened session id %d ********\n\n", sid);

	res = wsman_session_serialize(sid, d, EXL_ExamplePolicy_TypeInfo);

	res = wsman_session_transfer_create(sid, res, 0);

	if (!res) {
		printf("******** Transfer Create failed - %s ********\n\n",
			wsman_session_error(sid));
		return 0;
	}

	printf ("******** Transfer Create response ********\n%s\n", res);

	wsman_session_close(sid);

	printf("******** Closed session id %d ********\n\n", sid);

	return 1;
}
