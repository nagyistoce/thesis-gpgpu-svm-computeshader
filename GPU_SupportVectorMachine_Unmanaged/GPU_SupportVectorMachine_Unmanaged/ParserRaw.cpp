#include "stdafx.h"
#include "ParserRaw.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	DataDocumentPtr ParserRaw::parse(boost::filesystem::path path){
		DataDocumentPtr doc = DataDocumentPtr(new DataDocument);
		beginParsing(path);

		unsigned int bufferPos = 0;
		int colId = 0;
		int nameCol = -1;

		// Parse attributes
		while(m_buffer[bufferPos] != '\n'){
			std::string& attribute = doc->newAttribute();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	'){
				attribute += m_buffer[bufferPos];
				bufferPos++;
			}

			if(attribute.find("class") != std::string::npos)
				doc->m_classAttributeId = colId;
			else if(attribute.compare("name") == 0){
				nameCol = colId;
				colId--;
				doc->m_attributes.pop_back();
			}

			if(m_buffer[bufferPos] == '	')
				bufferPos++;
			colId++;
		}
		bufferPos++;

		colId = 0;

		// Parse data
		std::string dataString;
		Value::v_precision numberTest;
		while(bufferPos < m_size){
			dataString.clear();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	' && bufferPos < m_size){
				if(m_buffer[bufferPos] != ' ')
					dataString += m_buffer[bufferPos];
				bufferPos++;
			}
			if(colId != nameCol){
				std::stringstream sStream(dataString);
				sStream >> numberTest;
				if(sStream.fail()){
					// non-numeric value
					doc->addValue(0.0,true);
				}
				else{
					// numeric value
					doc->addValue(numberTest);
				}
			}
			
			if(m_buffer[bufferPos] == '\n')
				colId = 0;
			else
				colId++;
			bufferPos++;
		}

		//// Parse data
		//std::string dataString;
		//Value::v_precision numberTest;
		//unsigned int bufferPostPos;
		//doc->m_data.reserve((m_size - bufferPos)/5);
		//while(bufferPos < m_size){
		//	dataString.clear();
		//	while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	' && bufferPos < m_size){
		//		if(m_buffer[bufferPos] != ' ')
		//			dataString += m_buffer[bufferPos];
		//		bufferPos++;
		//	}

		//	//dataString.assign(&m_buffer[bufferPostPos],bufferPos-bufferPostPos);
		//	try{
		//		numberTest = boost::lexical_cast<Value::v_precision,std::string>(dataString);
		//		doc->addValue(numberTest);
		//	}
		//	catch(boost::bad_lexical_cast &){
		//		doc->addValue(0.0f,true);
		//	}
		//	
		//	bufferPos++;
		//}

		doc->m_data.shrink_to_fit();

		doc->buildInstances();
		endParsing();
		return doc;
	}
}