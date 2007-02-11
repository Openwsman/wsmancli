#include <iostream>
#include <cpp/WsmanClient.h>

using std::cout;

using namespace WsmanClientNamespace;
int main(int argc, char* argv[])
{
	WsmanClient client = WsmanClient("http://wsman:secret@192.168.1.41:8889/wsman");
	NameValuePairs selectors = NameValuePairs();
	selectors["CreationClassName"] =  "CWS_Instance";
	selectors["Id"] = "Instance #1";
	string r = client.Get("http:///cws.sblim.sf.net/wbem/wscim/1/cim-schema/2/CWS_Instance", &selectors );
	cout << "resource: \n" << r << "\n";
	return 0;
}
