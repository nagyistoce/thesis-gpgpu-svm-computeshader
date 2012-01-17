#pragma once
#include "DataDocument.h"

namespace SVM_Framework{
	class IParser{
	public:
		virtual DataDocumentPtr parse(std::string filename) = 0;
	protected:
		void beginParsing(std::string filename);
		void endParsing();

		unsigned int m_size;
		char* m_buffer;
	};
}

typedef boost::shared_ptr<SVM_Framework::IParser> IParserPtr;