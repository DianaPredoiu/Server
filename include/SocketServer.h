#pragma once
#include "SocketConnection.h"

namespace ssock{
	class SocketServer
	{
		SocketConnection client_socket;
		//pool of handles
		//call all the functions from SocketConnection for conn
		//write here the receive and send methods? to check if the receive buff is one of my handles
		//the server parses the http
		//how to make the server..


	public:

		SocketServer(SocketConnection client);

		SocketServer();

		void connection();

		~SocketServer();

	};

}