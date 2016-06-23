#pragma once
#include <iostream>
using namespace std;

namespace ssock{
	class BaseException {

	private:
		string msg_;

	public:
		//BaseException(const string& msg) : msg_(msg) {}
		//~BaseException() {}

		string getMessage() const { return(msg_); }


	};
}
