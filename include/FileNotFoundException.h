#pragma once
#include <iostream>
#include "BaseException.h"

namespace ssock{
	class FileNotFoundException : public BaseException {

	private:
		string msg_;

	public:
		FileNotFoundException(const string& msg) : msg_(msg)
		{
		}
		~FileNotFoundException() {}

		string getMessage() const { return(msg_); }

	};
}
