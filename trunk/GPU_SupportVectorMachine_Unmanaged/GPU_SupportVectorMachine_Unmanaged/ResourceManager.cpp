#include "stdafx.h"
#include "ResourceManager.h"
#include "ParserRaw.h"
#include "ParserARFF.h"

namespace SVM_Framework{
	std::string ResourceManager::m_resourcePath = "..\\Resources\\";

	ResourceManager::ResourceManager(){
		// Add parsers
		addParser(".txt",IParserPtr(new ParserRaw));
		addParser(".arff",IParserPtr(new ParserARFF));
	}

	void ResourceManager::addParser(std::string name, IParserPtr parser){
		m_parsers[name] = parser;
	}

	DataDocumentPtr ResourceManager::parseDocument(std::string filename){
		std::map<std::string,IParserPtr>::iterator parserItr;
		if((parserItr = m_parsers.find(filename.substr(filename.find_last_of('.'),filename.size()))) == m_parsers.end()){
			TRACE_DEBUG("No apropriate parser found for that fileformat.");
			return DataDocumentPtr();
		}

		m_loadedDocuments[filename] = parserItr->second->parse(filename);
		return m_loadedDocuments[filename];
	}

	boost::filesystem::path ResourceManager::findFilePath(std::string filename, std::string dir){
		boost::filesystem::path path;
		if(boost::filesystem::exists(dir)){
			if(boost::filesystem::is_directory(dir)){
				boost::filesystem::directory_iterator itr(dir);
				while(itr != boost::filesystem::directory_iterator()){
					boost::filesystem::directory_entry entry = *itr;
					if(boost::filesystem::is_directory(entry.path().generic_string())){
						path = findFilePath(filename, entry.path().generic_string());
						if(filename.compare(path.filename().generic_string()) == 0){
							break;
						}
					}
					else if(filename.compare(entry.path().filename().generic_string()) == 0){
						path = entry.path();
						break;
					}
					itr++;
				}
			}
			else{
				path = boost::filesystem::path(filename);
			}
		}
		else if(m_resourcePath.compare(dir) != 0){
			path = findFilePath(filename,m_resourcePath);
		}
		else{
			TRACE_DEBUG("Path does not exist.");
		}

		return path;
	}

	std::vector<boost::filesystem::path> ResourceManager::getFilesInFolder(std::string dir){
		std::vector<boost::filesystem::path> files;

		if(boost::filesystem::is_directory(dir)){
			boost::filesystem::directory_iterator itr(dir);
			while(itr != boost::filesystem::directory_iterator()){
				files.push_back(itr->path());
				itr++;
			}
		}

		return files;
	}
}