#include <iostream>
#include <cpp/OpenWsmanClient.h>
#include <u/libu.h>

using std::cout;

using namespace WsmanClientNamespace;
int main(int argc, char* argv[])
{
	const char *endpoint, *resource_uri;
	u_uri_t *uri;
	if (argc< 3) {
		fprintf(stderr, "Usage: %s <endpoint> <resource uri>\n", argv[0]);
	}
	endpoint= argv[1];
	resource_uri = argv[2];

	if (!endpoint || !resource_uri) {
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
	vector<string> vec;
	try {
		client->Enumerate(resource_uri, vec );
	}

	catch (GeneralWsmanException &e) {
		printf("GeneralWsmanException:\n");
		printf("%s\n", e.what());

	}
	for (vector<string>::iterator iter = vec.begin();
			iter != vec.end(); ++iter) {
		cout << "item: " << *iter << "\n";
	}
	return 0;
}
