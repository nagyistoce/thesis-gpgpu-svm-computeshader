#include "stdafx.h"
#include "IParser.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	void IParser::beginParsing(boost::filesystem::path path){
		std::ifstream open(path.generic_string(),std::ios_base::binary);
		m_size = boost::filesystem::file_size(path);
		m_buffer = new char[m_size];
		open.read(m_buffer,m_size);
		open.close();
	}

	void IParser::endParsing(){
		delete[] m_buffer;
	}
}