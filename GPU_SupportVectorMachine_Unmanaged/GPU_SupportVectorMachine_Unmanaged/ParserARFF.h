#pragma once
#include "IParser.h"

namespace SVM_Framework{
	class ParserARFF : public IParser{
	public:
		DataDocumentPtr parse(std::string filename);
	private:
	};
}