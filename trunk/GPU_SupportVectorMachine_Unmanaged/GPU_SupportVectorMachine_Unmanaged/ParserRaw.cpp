#include "stdafx.h"
#include "ParserRaw.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	DataDocumentPtr ParserRaw::parse(boost::filesystem::path path){
		DataDocumentPtr doc = DataDocumentPtr(new DataDocument);
		beginParsing(path);

		unsigned int bufferPos = 0;

		// Parse attributes
		while(m_buffer[bufferPos] != '\n'){
			std::string& attribute = doc->newAttribute();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	'){
				attribute += m_buffer[bufferPos];
				bufferPos++;
			}
			if(m_buffer[bufferPos] == '	')
				bufferPos++;
		}
		bufferPos++;

		// Parse data
		std::string dataString;
		float numberTest;
		while(bufferPos < m_size){
			dataString.clear();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	' && bufferPos < m_size){
				dataString += m_buffer[bufferPos];
				bufferPos++;
			}
			std::stringstream sStream(dataString);
			sStream >> numberTest;
			if(sStream.fail()){
				// non-numeric value
				doc->addValue(0.0f,true);
			}
			else{
				// numeric value
				doc->addValue(numberTest);
			}
			
			bufferPos++;
		}

		doc->buildInstances();
		endParsing();
		return doc;
	}
}