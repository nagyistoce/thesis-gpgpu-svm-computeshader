#include "stdafx.h"
#include "DataDocument.h"

namespace SVM_Framework{
	DataDocument::DataDocument():m_classAttributeId(0){
		
	}

	InstancePtr DataDocument::getInstance(unsigned int index){
		if(index >= m_instances.size()){
			return InstancePtr();
		}

		return m_instances[index];
	}

	unsigned int DataDocument::getNumInstances(){
		return m_instances.size();
	}
	
	unsigned int DataDocument::getNumAttributes(){
		return m_attributes.size()-1;
	}

	std::string& DataDocument::newAttribute(){
		m_attributes.push_back("");
		return m_attributes.back();
	}

	void DataDocument::addValue(Value value, bool missing){
		if(missing)
			m_missing.insert(m_data.size());
		m_data.push_back(value);
	}

	void DataDocument::buildInstances(){
		InstancePtr inst = InstancePtr(new Instance(m_classAttributeId,m_attributes.size()));
		m_instances.reserve(m_data.size()/m_attributes.size());
		for(unsigned int i=0; i<m_data.size(); i++){
			if(i != 0 && (i % m_attributes.size()) == 0){
				m_instances.push_back(inst);
				inst = InstancePtr(new Instance(m_classAttributeId,m_attributes.size()));
				if(m_missing.find(i) == m_missing.end())
					inst->insertValue(&m_data[i]);
				else
					inst->insertValue(NULL);
			}
			else{
				if(m_missing.find(i) == m_missing.end())
					inst->insertValue(&m_data[i]);
				else
					inst->insertValue(NULL);
			}
		}
		m_instances.push_back(inst);

		// Calculate means and modes for attributes
		std::vector<double> meansModes;
		meansModes.reserve(getNumAttributes());
		unsigned int num;
		for(unsigned int j=0; j<getNumAttributes(); j++){
			num = 0;
			meansModes.push_back(0);
			for(unsigned int i=0; i<m_instances.size(); i++){
				if(!m_instances[i]->missing(j)){
					meansModes[meansModes.size()-1] += m_instances[i]->getValue(j);
					num++;
				}
			}
			meansModes[meansModes.size()-1] /= num;
		}

		std::vector<double> max,min;
		max.assign(getNumAttributes(),-DBL_MAX);
		min.assign(getNumAttributes(),DBL_MAX);
		unsigned int attribute,instance;
		for(unsigned int i=0; i<m_data.size(); i++){
			attribute = i % m_attributes.size();
			instance = i / m_attributes.size();
			if(attribute != m_classAttributeId){
				if(attribute > m_classAttributeId)
					attribute--;
				
				if(m_missing.find(i) != m_missing.end()){
					m_data[i] = meansModes[attribute];
					assert(m_instances[instance]->m_valueVector[attribute] == NULL);
					m_instances[instance]->m_valueVector[attribute] = &m_data[i];
				}

				if(m_instances[instance]->m_valueVector[attribute]->getValue() < min[attribute]){
					min[attribute] = m_instances[instance]->m_valueVector[attribute]->getValue();
				}
				if(m_instances[instance]->m_valueVector[attribute]->getValue() > max[attribute]){
					max[attribute] = m_instances[instance]->m_valueVector[attribute]->getValue();
				}
			}
		}

		// Normalize
		double scale=2,translate=-1,value;
		for(unsigned int i=0; i<m_instances.size(); i++){
			for(unsigned int j=0; j<getNumAttributes(); j++){
				value = m_instances[i]->m_valueVector[j]->getValue();
				*m_instances[i]->m_valueVector[j] = Value(((value - min[j]) / (max[j] - min[j]) * scale + translate));
			}
		}
	}
}