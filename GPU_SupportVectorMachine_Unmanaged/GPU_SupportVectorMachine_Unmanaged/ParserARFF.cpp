#include "stdafx.h"
#include "ParserARFF.h"

namespace SVM_Framework{
	DataDocumentPtr ParserARFF::parse(boost::filesystem::path path){
		DataDocumentPtr doc;
		beginParsing(path);

		endParsing();
		return doc;
	}
}