#include "stdafx.h"
#include "ParserARFF.h"

namespace SVM_Framework{
	DataDocumentPtr ParserARFF::parse(std::string filename){
		DataDocumentPtr doc;
		beginParsing(filename);

		endParsing();
		return doc;
	}
}