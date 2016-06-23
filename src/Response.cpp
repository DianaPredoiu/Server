#include "Response.h"

namespace ssock{
	Response::Response()
	{
		http = "HTTP/1.1";
		content = "\r\nContent-Type: text/html; charset=ISO-8859-4\r\n\r\n";

	}

	void Response::setIsOkay()
	{
		isOkay = " 200 Okay";
	}

	void Response::setFile(string _file)
	{
		file = _file;
	}

	string Response::getHttp()
	{
		return http;
	}

	string  Response::toString()
	{
		string str;
		str = http + isOkay + content + file;
		return str;
	}
}
