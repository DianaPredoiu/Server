#pragma once
#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>// pt getaddrinfo
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <sstream>
#include <functional>
#include <fstream>
#include "BaseException.h"
#include "FileNotFoundException.h"
#include "ServerException.h"
#include "Response.h"

#include "Jinja2CppLight.h"

#define CRT_SECURE_NO_WARNINGS
namespace ssock{
	using namespace std;
	using namespace Jinja2CppLight;
	const PCSTR DEFAULT_PORT = "8989";
	const unsigned DEFAULT_BUFLEN = 1024;


	class SocketConnection
	{
	

		SOCKET listen_socket;
		SOCKET client_socket;
		WSADATA wsaData;
		int iResult;//pentru erori
		DWORD returnValue;//pentru erori
		struct addrinfo *result;// socketul returnat de getaddrinfo pe care serverul il va folosi
		map< string, std::function<string(map<string, string>)>> requests;
		map<string, std::function<string(map<string, string>)>>::iterator it;

		map<string, string> getParams;
		map<string, string> postParams;
		map<string, string>::iterator iter;

		Response response;



		//map<string, double> getValues;//pt var si var2 GET
		//vector<string> getRequest;//pass it to lambda , 0 for key

	public:

		SocketConnection();

		void setRequests();

		SOCKET getlistenSocket();

		SOCKET getClientSocket();

		void startUp();

		addrinfo getServer();

		int validGetAddrInfo();

		void setAddrServer();

		void listenSocket();

		int validListenSocket();

		void bind();

		void listenToSocket();

		int validClientSocket();

		void sendReceive(SOCKET listen_socket);

		void shutDown();

		void threadFunction();

		vector<string> splitGet(string path);

		vector<string> splitRequest(string path);

		string pageNotFound();

		vector<string> splitPost(string &str, string &obj);

		vector<string> splitParamPost(string &aux);

		~SocketConnection();
	};

}
