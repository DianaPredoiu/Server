#include "SocketConnection.h"
#include <algorithm>

namespace ssock{

	SocketConnection::SocketConnection()
	{
		setRequests();
	}

	SOCKET SocketConnection::getClientSocket()
	{
		return client_socket;
	}

	SOCKET SocketConnection::getlistenSocket()
	{
		return listen_socket;
	}

	void SocketConnection::startUp()
	{// initializez un Winsock si vad daca wVersion e la fel ca wHighVersion, adica 2.2
		//WSADATA wsaData;
		//int iResult;//pentru erori
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			//cout << "WSAStartup FAILED : " << iResult;
			//return;
			throw ServerException("Start up failed");
		}
	}

	addrinfo SocketConnection::getServer()
	{
		struct addrinfo hints;// socket address pe care il vreau eu 
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET; // pentru IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;// asigura trimiterea unui stream??? intre aplicatiile care comunica pe un IP
		hints.ai_flags = AI_PASSIVE;// => o sa folosesc adresa socketului returnat intr un apel de functie bind
		// flagul se schimba cand apelez functia getaddrinfo, daca functia returneaza null  => flagul este setat pentru IPv4

		return hints;
	}

	int SocketConnection::validGetAddrInfo()
	{
		if (returnValue != 0)
		{
			WSACleanup();// sa inchida winsock 2 DLL / libraria 
			//string msg = "Get address info failed with error : ";
			////msg =msg + returnValue;
			////throw new ServerException("get adr info with error");
			//cout << "GETADDRINFO FAILED with error :  " << returnValue << endl;
			return 1;
			//throw ServerException("GETADDRINFO FAILED with error :"+returnValue);
		}
		else
		{
			cout << "Used port : " << DEFAULT_PORT << endl;
			cout << "GETADDRINFO with success" << endl;
		}
		return 0;

	}

	void SocketConnection::setAddrServer()
	{
		//SET ADDR FOR SERVER

		returnValue = getaddrinfo(NULL, DEFAULT_PORT, &getServer(), &result);//retine eroarea daca este cazul
		if (validGetAddrInfo() == 1)
			throw ServerException("Set address server failed");

		//	return *result;
		//return addrinfo{NULL};//aici
	}

	int SocketConnection::validListenSocket()
	{
		if (listen_socket == INVALID_SOCKET)
		{
			cout << "INVALID SOCKET" << WSAGetLastError();
			freeaddrinfo(result);//golesc addrinfo de result pt ca e invalid socketul
			WSACleanup();
			return 1;

		}
		else {
			cout << "VALID socket" << endl;
		}
		return 0;
	}

	void SocketConnection::listenSocket()
	{
		listen_socket = INVALID_SOCKET;// socket care sa "asculte" conexiunile clientilor 
		//struct addrinfo *result = &setAddrServer();
		//result = &setAddrServer();
		//trebuie sa ii dau adresa result pt server sa "asculte" conexiunile clientilor
		listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);// 2- IPv4 , 1 - AF_INET, 6 - daca ai_fam e AF_INET si socket type  = stream

		//LISTEN SOCKET
		if (validListenSocket() == 1)
			throw ServerException("Invalid socket exception");
	}

	void SocketConnection::bind()
	{
		//BIND ca serverul meu sa accepte conexiunile clientilor
		//result = &setAddrServer();
		iResult = ::bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			cout << "BIND FAILED" << WSAGetLastError();
			//throw "Bind failed with error :";
			freeaddrinfo(result);//eliberez adresa 
			closesocket(listen_socket);//daca se inchide cu succes returneaza 0 
			WSACleanup();
			throw ServerException("Bind failed");
		}
		else
		{
			cout << "BIND  with success" << endl;
			freeaddrinfo(result);// odata ce s-a facut legatura cu succes nu mai este nevoie de adresa returnata de getaddrinfo
		}
	}

	void SocketConnection::listenToSocket()
	{
		if (listen(listen_socket, 1) == SOCKET_ERROR)// sau 1 pt o conexiune in coada in loc de somaxconn
		{
			//cout << "LISTEN FAILED with error : " << WSAGetLastError();
			closesocket(listen_socket);
			WSACleanup();
			throw ServerException("Listen failed with error : " + WSAGetLastError());
		}
		else
		{
			cout << "LISTEN with success!" << endl;
		}
	}

	int SocketConnection::validClientSocket()
	{
		if (client_socket == INVALID_SOCKET)
		{
			cout << "ACCEPT FAILED with error : " << WSAGetLastError() << endl;
			closesocket(listen_socket);
			WSACleanup();
			return 1;

		}
		else
		{
			cout << "Connection accepted!" << endl;
		}
		return 0;
	}

	void SocketConnection::sendReceive(SOCKET listen_socket)
	{

		SOCKET clientSocket;//socket temporar care accepta conexiunile de la clienti
		//clientSocket = accept(listenSocket, NULL, NULL);// cu functia accept permite o incercare de conexiune la socketul care "asculta"
		// 1 NULL  - pointer catre un buffer care retine adresa conexiunii, 2 NULL - lungimea adresei de conexiune(1 NULL)

		clientSocket = accept(listen_socket, NULL, NULL);
		client_socket = clientSocket;

		vector<string> handle;
		if (validClientSocket() == 1)
			throw ServerException("SERVER EXCEPTION : accept failed with error - " + WSAGetLastError());

		int iResult;//pentru erori
		do {
			char receiveBuf[DEFAULT_BUFLEN];
			int receiveBufLen = DEFAULT_BUFLEN - 1;

			iResult = recv(clientSocket, receiveBuf, receiveBufLen, 0);// primesc date de la un socket conectat la server
			//cout << receiveBuf << endl;

			try{

				if (receiveBuf[0] == 'G')// sa intre doar daca exista o cerere
				{
					handle = splitGet(receiveBuf);

					if (handle[0] == "GET" && handle.size() > 1)
					{
						vector<string> getRequest = splitRequest(handle[1]);//sa schimb sa nu mai folosesc un vector intermediar!!!!
						for (int i = 1; i < getRequest.size() - 1; i += 2)
							getParams[getRequest[i]] = getRequest[i + 1];

						it = requests.find(getRequest[0]);

						if (it != requests.end() && response.getHttp() == handle[2])
						{
							response.setFile(it->second(getParams));//dau ca si param un map pentru variabile

							response.setIsOkay();
							int send_res = send(clientSocket, &response.toString()[0], strlen(&response.toString()[0]), 0);
							if (send_res == -1)
							{
								throw "Server error : can not send to this page ";
							}


						}
						else
						{
							cout << "HANDLE NOT FOUND!" << endl;
							int send_res = send(clientSocket, &pageNotFound()[0], strlen(&pageNotFound()[0]), 0);
							if (send_res == -1)
							{
								throw "Server error : can not send to this page ";
							}
						}

					}


				}
				else  if (receiveBuf[0] == 'P')
				{
					string str = receiveBuf;
					string split = "var";
					vector<string> values;
					values = splitPost(split, str);

					for (int i = 0; i < values.size() - 1; i += 2)
						postParams[values[i]] = values[i + 1];

					it = requests.find("/process");

					if (it != requests.end() && response.getHttp() == "HTTP/1.1")
					{
						response.setFile(it->second(postParams));//dau ca si param un map pentru variabile

						response.setIsOkay();
						int send_res = send(clientSocket, &response.toString()[0], strlen(&response.toString()[0]), 0);
						if (send_res == -1)
						{
							throw "Server error : can not send to this page ";
						}


					}
					else
					{
						cout << "HANDLE NOT FOUND!" << endl;
						int send_res = send(clientSocket, &pageNotFound()[0], strlen(&pageNotFound()[0]), 0);
						if (send_res == -1)
						{
							throw "Server error : can not send to this page ";
						}
					}

				}
			}
			catch (FileNotFoundException& msg)
			{
				int send_res = send(clientSocket, &pageNotFound()[0], strlen(&pageNotFound()[0]), 0);

			}
			catch (ServerException& e)
			{
				cout << e.getMessage() << endl;
			}
			catch (const char* msg)
			{
				cout << msg << endl;
			}
		} while (iResult > 0);


		//inchid conexiunea din mom ce am terminat 
		try{
			iResult = shutdown(clientSocket, SD_SEND);//inchid operatia de trimitere ca de primit nu mai primeste 
			if (iResult == SOCKET_ERROR) {
				closesocket(clientSocket);
				throw ServerException("SERVER EXCEPTION : shut down failed with error - " + WSAGetLastError());
			}
			else
				cout << "SHUT DOWN" << endl;

			closesocket(clientSocket);
		}
		catch (ServerException& e)
		{
			cout << e.getMessage() << endl;
		}


	}

	vector<string> SocketConnection::splitPost(string & str, string & obj) {

		vector<string> v;
		size_t pos = obj.find(str);
		size_t pos2 = obj.find("Ì");
		string values = obj.substr(pos, pos2 - pos);


		pos = 0;
		while ((pos2 = values.find_first_of("=&", pos)) != std::string::npos)
		{
			if (pos2 > pos)
				v.push_back(values.substr(pos, pos2 - pos));
			pos = pos2 + 1;
		}
		if (pos < values.length())
			v.push_back(values.substr(pos, std::string::npos));

		return v;
	}



	void SocketConnection::threadFunction() throw (FileNotFoundException&)
	{
		while (1)
		{
			std::vector<std::thread> threads;

			for (int i = 0; i < 10; i++) {
				threads.push_back(std::thread(&SocketConnection::sendReceive, this, listen_socket));//ii dau adresa functiei si campul din clasa
			}
			for (auto& thread : threads) {
				thread.join();
			}
		}
		WSACleanup();
	}

	vector<string> SocketConnection::splitGet(string path)
	{
		vector<string> split;
		string buf;
		stringstream ss(path); // Insert the string into a stream
		//int count = 0;
		while (ss >> buf && split.size() < 5)
		{
			split.push_back(buf);

		}

		return split;
	}

	vector<string> SocketConnection::splitRequest(string path)
	{
		vector<string> v; //Use vector to add the words

		std::size_t prev_pos = 0, pos;
		while ((pos = path.find_first_of("?=&", prev_pos)) != std::string::npos)
		{
			if (pos > prev_pos)
				v.push_back(path.substr(prev_pos, pos - prev_pos));
			prev_pos = pos + 1;
		}
		if (prev_pos < path.length())
			v.push_back(path.substr(prev_pos, std::string::npos));

		return v;
	}

	string SocketConnection::pageNotFound()
	{
		ifstream fin;
		string message = "HTTP/1.1 200 Okay\r\nContent-Type: text/html; charset=ISO-8859-4\r\n\r\n";

		fin.open("..\\Entries\\notFoundPage.txt", std::ios::binary);
		fin.seekg(0, std::ios::end);
		size_t count = fin.tellg();

		char* html = new char[count];
		fin.seekg(0, std::ios::beg);
		fin.read(&html[0], count);
		html[count] = '\0';

		message += html;
		fin.close();
		return message;
	}

	void SocketConnection::setRequests()
	{
		requests["/"] = [](map<string, string> getParams)->string{cout << "GET -> HOME REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\Home.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		/*Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
		mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();*/
		//cout << message << endl;
		return message; };

		requests["/home"] = [](map<string, string> getParams)->string{cout << "GET -> HOME REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;
		string path;
		try {
			fin.open("..\\Entries\\Home.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		//string source = message;

		//Jinja2CppLight::Template mytemplate(message);
		//for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		//{
		//	mytemplate.setValue(it->first, it->second);
		//}
		//message = mytemplate.render();
		//cout << message << endl;
		return message; };

		//css e pagina la care ma trimit cand dau submit la form.txt
		requests["/css"] = [](map<string, string> getParams)->string{cout << "GET -> CSS REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\test.css", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
			mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();
		return message;

		};

		requests["/form"] = [](map<string, string> getParams)->string{cout << "GET -> FORM REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\form.txt.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		if (getParams.empty())
		{
			return message;
		}
		else
		{
			Jinja2CppLight::Template mytemplate(message);
			for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
			{
				mytemplate.setValue(it->first, it->second);
			}
			message = mytemplate.render();
		}

		return message;

		};

		requests["/about"] = [](map<string, string> getParams)->string{cout << "GET -> ABOUT REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\about.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		/*Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
		mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();*/
		//cout << message << endl;
		return message; };

		requests["/restaurants"] = [](map<string, string> getParams)->string{cout << "GET -> RESTAURANTE REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\restaurante.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		/*Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
		mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();*/
		//cout << message << endl;
		return message; };

		requests["/reviews"] = [](map<string, string> getParams)->string{cout << "GET -> REVIEWS REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message;

		try {
			fin.open("..\\Entries\\reviews.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		/*Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
		mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();*/
		//cout << message << endl;
		return message; };

		requests["/process"] = [](map<string, string> getParams)->string{cout << "GET -> PROCESS REQUEST" << endl;
		ifstream fin;
		fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		string message = "", param, restaurant;

		try {
			fstream fin;
			fin.open("..\\Entries\\reviews.txt", fstream::in | fstream::out | fstream::binary);
			//fin.open("..\\Entries\\reviews.txt", std::ios::binary);
			if (fin.is_open())
			{
				char* html = nullptr;
				if (!fin.eof())
				{
					fin.seekg(0, std::ios::end);
					size_t count = fin.tellg();
					char* html = nullptr;
					html = new char[count];
					fin.seekg(0, std::ios::beg);
					fin.read(&html[0], count);
					html[count] = '\0';
					message += html;
				}
				fin.close();
			}
			std::map<string, string>::iterator it = getParams.begin();
			param = it->first;
			const char* write = nullptr;
			string _where;

			if (param == "varR1_name") _where = "<!-- LAST R1 -->";
			else if (param == "varR2_name") _where = "<!-- LAST R2 -->";
			else if (param == "varR3_name") _where = "<!-- LAST R3 -->";
			else if (param == "varR4_name") _where = "<!-- LAST R4 -->";
			else if (param == "varR5_name") _where = "<!-- LAST R5 -->";
			else if (param == "varR6_name") _where = "<!-- LAST R6 -->";
			else if (param == "varR7_name") _where = "<!-- LAST R7 -->";
			else if (param == "varR8_name") _where = "<!-- LAST R8 -->";
			else if (param == "varR9_name") _where = "<!-- LAST R9 -->";
			else if (param == "varR10_name") _where = "<!-- LAST R10 -->";

			size_t pos = message.find(_where) + 17;

			param = "\n <br>\n<div class=\"comment\"><h4>";
			param += it->second.c_str();
			param += "</h4><br>";
			it++;
			param += "<p>";
			param += it->second.c_str();
			param += "</p></div><br><br>";
			std::replace(param.begin(), param.end(), '+', ' ');
			message.insert(pos, param);
			fin.open("..\\Entries\\reviews.txt", std::fstream::out | std::fstream::trunc);
			write = &message[0];
			fin.write(write, strlen(write));

			fin.close();

		}
		catch (std::ifstream::failure e) {
			cout << "File not found" << e.code() << endl;
			throw FileNotFoundException("page not found");
		}
		/*Jinja2CppLight::Template mytemplate(message);
		for (std::map<string, string>::iterator it = getParams.begin(); it != getParams.end(); ++it)
		{
		mytemplate.setValue(it->first, it->second);
		}
		message = mytemplate.render();*/
		//cout << message << endl;
		return message; };
	}

	SocketConnection::~SocketConnection()
	{

	}
}
