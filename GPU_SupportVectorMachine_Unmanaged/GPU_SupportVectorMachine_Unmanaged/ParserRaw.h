#pragma once
#include "IParser.h"

namespace SVM_Framework{
	class ParserRaw : public IParser{
	public:
		DataDocumentPtr parse(std::string filename);
	private:
	};
}