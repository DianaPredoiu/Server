#pragma once
#include <iostream>
using namespace std;

namespace ssock{
	class Response{

	private:
		string http;
		string isOkay;
		string content;
		string file;

	public:
		Response();

		void setIsOkay();

		void setFile(string _file);

		string getHttp();

		string toString();

	};
}
