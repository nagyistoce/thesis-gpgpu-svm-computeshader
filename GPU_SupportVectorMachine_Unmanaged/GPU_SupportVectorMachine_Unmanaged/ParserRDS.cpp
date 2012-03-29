#include "stdafx.h"
#include "ParserRDS.h"

namespace SVM_Framework{
	DataDocumentPtr ParserRDS::parse(boost::filesystem::path path){
		DataDocumentPtr doc = DataDocumentPtr(new DataDocument);
		beginParsing(path);

		unsigned int bufferPos = 0;
		unsigned int idCol = 0;

		unsigned int colCount = 0;

		// Parse input format
		std::string format;
		while(m_buffer[bufferPos] != '\n'){
			format.clear();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	'){
				format += m_buffer[bufferPos];
				bufferPos++;
			}

			if(format.compare("class") == 0){
				doc->m_classAttributeId = doc->m_format.size();
				doc->m_format.push_back(DataDocument::IF_CLASS);
			}
			else if(format.compare("id") == 0){
				idCol = colCount;
			}
			else if(format.compare("numeric") == 0){
				doc->m_format.push_back(DataDocument::IF_NUMERIC);
			}
			else if(format.compare("nominal") == 0){
				doc->m_format.push_back(DataDocument::IF_NOMINAL);
			}
			else{
				doc->m_format.push_back(DataDocument::IF_NULL);
			}

			if(m_buffer[bufferPos] == '	')
				bufferPos++;

			colCount++;
		}
		bufferPos++;

		colCount = 0;
		// Parse attributes
		while(m_buffer[bufferPos] != '\n'){
			if(colCount == idCol)
				doc->m_idCollumn.push_back("");
			std::string& attribute = (colCount != idCol) ? doc->newAttribute() : doc->m_idCollumn.back();
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	'){
				attribute += m_buffer[bufferPos];
				bufferPos++;
			}

			if(m_buffer[bufferPos] == '	'){
				colCount++;
				bufferPos++;
			}
			else if(m_buffer[bufferPos] == '\n'){
				colCount = 0;
			}
		}
		bufferPos++;

		colCount = 0;

		// Parse data
		std::string dataString;
		Value::v_precision numberTest;
		unsigned int bufferPostPos;
		doc->m_data.reserve((m_size - bufferPos)/5);
		while(bufferPos < m_size){
			dataString.clear();
			while((m_buffer[bufferPos] == ' ' || m_buffer[bufferPos] == '	') && bufferPos < m_size){
				bufferPos++;
			}
			bufferPostPos = bufferPos;
			while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	' && m_buffer[bufferPos] != ' ' && bufferPos < m_size){
				bufferPos++;
			}

			dataString.assign(&m_buffer[bufferPostPos],bufferPos-bufferPostPos);

			if(colCount == idCol)
				doc->m_idCollumn.push_back(dataString);
			else{
				try{
					numberTest = boost::lexical_cast<Value::v_precision,std::string>(dataString);
					doc->addValue(numberTest);
				}
				catch(boost::bad_lexical_cast &){
					doc->addValue(0.0f,true);
				}
			}

			if(m_buffer[bufferPos] == '	'){
				colCount++;
			}
			else if(m_buffer[bufferPos] == '\n'){
				colCount = 0;
			}
			
			bufferPos++;
		}

		//// Parse data
		//std::string dataString;
		//Value::v_precision numberTest;
		//doc->m_data.reserve((m_size - bufferPos)/5);
		//while(bufferPos < m_size){
		//	dataString.clear();
		//	while(m_buffer[bufferPos] != '\n' && m_buffer[bufferPos] != '	' && bufferPos < m_size){
		//		if(m_buffer[bufferPos] != ' ')
		//			dataString += m_buffer[bufferPos];
		//		bufferPos++;
		//	}

		//	if(colCount == idCol)
		//		doc->m_idCollumn.push_back(dataString);
		//	else{
		//		std::stringstream sStream(dataString);
		//		sStream >> numberTest;
		//		if(sStream.fail()){
		//			// non-numeric value
		//			doc->addValue(0.0f,true);
		//		}
		//		else{
		//			// numeric value
		//			doc->addValue(numberTest);
		//		}
		//	}

		//	if(m_buffer[bufferPos] == '	'){
		//		colCount++;
		//	}
		//	else if(m_buffer[bufferPos] == '\n'){
		//		colCount = 0;
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