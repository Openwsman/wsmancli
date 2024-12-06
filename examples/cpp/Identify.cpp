#include <iostream>
#include <cpp/OpenWsmanClient.h>
#include <u/libu.h>

int facility = LOG_DAEMON;

using std::cout;

using namespace WsmanClientNamespace;
int main(int argc, char* argv[])
{
	const char *endpoint, *resource_uri;
	string out;
	u_uri_t *uri;
	if (argc< 2) {
		fprintf(stderr, "Usage: %s <endpoint>\n", argv[0]);
	}
	endpoint= argv[1];

	if (!endpoint) {
		fprintf(stderr, "endpoint option required\n");
		return 1;
	}
	if (endpoint != NULL)
		if (u_uri_parse((const char *) endpoint, &uri) != 0 )
			return 1;
	OpenWsmanClient *client = new OpenWsmanClient( uri->host,
			uri->port,
			uri->path,
			uri->scheme,
			"digest",
			uri->user,
			uri->pwd);
	try {
		out = client->Identify();
	}

	catch (GeneralWsmanException &e) {
		printf("GeneralWsmanException:\n");
		printf("%s\n", e.what());

	}
	cout << "Identify: " << out << "\n";
	return 0;
}
