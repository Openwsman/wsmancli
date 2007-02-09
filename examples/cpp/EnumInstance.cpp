#include <iostream>
#include <cpp/WsmanClient.h>

using std::cout;

using namespace WsmanClientNamespace;
int main(int argc, char* argv[])
{
	WsmanClient client = WsmanClient("http://wsman:secret@192.168.1.41:8889/wsman");
	vector<string> vec;
	client.Enumerate("http:///cws.sblim.sf.net/wbem/wscim/1/cim-schema/2/CWS_Instance", vec );
	for (vector<string>::iterator iter = vec.begin();
			iter != vec.end(); ++iter) {
		cout << "item: " << *iter << "\n";
	}
	return 0;
}
