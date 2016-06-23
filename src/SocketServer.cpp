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
		client_socket.startUp();//verific versiunile de winsock

		client_socket.setAddrServer();//setez adresa pt server

		client_socket.listenSocket();

		client_socket.bind();

		client_socket.listenToSocket();

		client_socket.threadFunction();
	}

	SocketServer::~SocketServer()
	{

	}

}