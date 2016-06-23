#include "SocketServer.h"

namespace ssock{

	SocketServer::SocketServer(SocketConnection client)
	{
		client_socket = client;
	}

	SocketServer::SocketServer( )
	{
		
	}

	void SocketServer::connection()
	{
		try{
			client_socket.startUp();//verific versiunile de winsock

			client_socket.setAddrServer();//setez adresa pt server

			client_socket.listenSocket();

			client_socket.bind();

			client_socket.listenToSocket();

			client_socket.threadFunction();
		}
		catch (ServerException& e)
		{
			cout << e.getMessage() << endl;
		}
		catch (const char* msg)
		{
			cout << msg << endl;
		}
		
	}

	SocketServer::~SocketServer()
	{

	}

}