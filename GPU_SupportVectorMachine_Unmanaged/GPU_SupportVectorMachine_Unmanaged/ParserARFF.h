#pragma once
#include "IParser.h"

namespace SVM_Framework{
	class ParserARFF : public IParser{
	public:
		DataDocumentPtr parse(boost::filesystem::path path);
	private:
	};
}