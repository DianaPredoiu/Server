#pragma once
#include <iostream>
#include "BaseException.h"

namespace ssock{
	class ServerException : public BaseException {

	private:
		string msg_;

	public:
		ServerException(const string& msg) : msg_(msg)
		{
		}
		~ServerException() {}

		string getMessage() const { return(msg_); }

	};
}
