#include "stdafx.h"
#include "ConfigManager.h"

namespace SVM_Framework{
	unsigned int ConfigManager::m_timerId = 0;
	std::map<unsigned int,boost::shared_ptr<boost::timer>> ConfigManager::m_timers = std::map<unsigned int,boost::shared_ptr<boost::timer>>();
	std::map<std::string,std::string> ConfigManager::m_configValues = std::map<std::string,std::string>();

	ConfigManager::ConfigManager(){
	}

	void ConfigManager::initialize(){
		char* buffer;
		boost::filesystem::path path = "..\\Resources\\Config.cfg";
		if(!boost::filesystem::exists(path))
			return;
		unsigned int size = boost::filesystem::file_size(path);
		buffer = new char[size];

		std::ifstream open(path.generic_string(),std::ios_base::binary);
		open.read(buffer,size);
		open.close();

		std::string line;
		unsigned int pos = 0;
		while(pos < size){
			if(buffer[pos] == '\n'){
				unsigned int middle = line.find('=');
				m_configValues[line.substr(0,middle)] = line.substr(middle+1,line.size());
				line.clear();
			}
			else
				line += buffer[pos];
			pos++;
		}
		if(!line.empty()){
			unsigned int middle = line.find('=');
			m_configValues[line.substr(0,middle)] = line.substr(middle+1,line.size());
		}

		delete[] buffer;
	}

	unsigned int ConfigManager::startTimer(){
		unsigned int id = m_timerId++;
		m_timers[id] = boost::shared_ptr<boost::timer>(new boost::timer);
		return id;
	}
	
	void ConfigManager::removeTimer(unsigned int timer){
		m_timers.erase(timer);
	}

	void ConfigManager::resetTimer(unsigned int timer){
		m_timers[timer]->restart();	
	}

	double ConfigManager::getTime(unsigned int timer){
		return m_timers[timer]->elapsed();
	}

	std::string ConfigManager::getSetting(std::string setting){
		return m_configValues[setting];
	}
}