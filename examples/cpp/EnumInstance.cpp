#include <iostream>
#include <cpp/WsmanClient.h>

using std::cout;

using namespace WsmanClientNamespace;
int main(int argc, char* argv[])
{
	const char *endpoint, *resource_uri;
	if (argc< 3) {
		fprintf(stderr, "Usage: %s endpoint resource uri\n", argv[0]);
	}
	endpoint= argv[1];
	resource_uri = argv[2];

	if (!endpoint || !resource_uri) {
		fprintf(stderr, "endpoint option required\n");
		return 1;
	}
	WsmanClient client = WsmanClient(endpoint);
	vector<string> vec;
	try {
		client.Enumerate(resource_uri, vec );
	}

	catch (WsmanException &e) {
		cout << "Fault:\n";
		cout << "\tCode:\t\t" << e.GetFaultCode() << "\n";
		cout << "\tSubCode:\t" << e.GetFaultSubcode() << "\n";
		cout << "\tDetail:\t\t" << e.GetFaultDetail() << "\n";
		cout << "\tReson:\t\t" << e.GetFaultReason() << "\n";

	}
	for (vector<string>::iterator iter = vec.begin();
			iter != vec.end(); ++iter) {
		cout << "item: " << *iter << "\n";
	}
	return 0;
}
