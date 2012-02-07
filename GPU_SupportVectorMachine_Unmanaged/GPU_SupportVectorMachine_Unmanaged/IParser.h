#pragma once
#include "DataDocument.h"

namespace SVM_Framework{
	class IParser{
	public:
		virtual DataDocumentPtr parse(boost::filesystem::path path) = 0;
	protected:
		void beginParsing(boost::filesystem::path path);
		void endParsing();

		unsigned int m_size;
		char* m_buffer;
	};
}

typedef boost::shared_ptr<SVM_Framework::IParser> IParserPtr;