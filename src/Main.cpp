#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>// pt getaddrinfo
#include <string.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include "SocketServer.h"
#include "FileNotFoundException.h"
#include "Shlwapi.h"

#include "Jinja2CppLight.h"
using namespace ssock;
using namespace Jinja2CppLight;
using namespace std;
int main()
{

	
	try
	{
		SocketServer test;
		test.connection();

	} catch (FileNotFoundException& msg)
	{
		cout << msg.getMessage()<< endl;
	}
	catch (const char* msg)
	{
		cout << msg << endl;
	}
	
	
	cin.get();
	return 0;
}