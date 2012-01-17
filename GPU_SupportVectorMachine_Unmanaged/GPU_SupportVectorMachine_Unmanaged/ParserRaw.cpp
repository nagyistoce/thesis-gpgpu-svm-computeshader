#include "stdafx.h"
#include "ParserRaw.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	DataDocumentPtr ParserRaw::parse(std::string filename){
		DataDocumentPtr doc;
		beginParsing(filename);

		endParsing();
		return doc;
	}
}