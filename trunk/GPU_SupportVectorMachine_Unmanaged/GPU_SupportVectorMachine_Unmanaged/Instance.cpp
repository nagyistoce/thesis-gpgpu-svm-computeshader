#include "stdafx.h"
#include "Instance.h"

namespace SVM_Framework{
	Instance::Instance(int classInd, unsigned int size):m_weight(1.0),m_classInd(classInd),m_classValue(NULL){
		m_valueVector.reserve(size);
	}

	void Instance::insertValue(Value* value){
		if(m_valueVector.size() == m_classInd && !m_classValue)
			m_classValue = value;
		else
			m_valueVector.push_back(value);
	}

	int Instance::classValue(){
		return int(m_classValue->getValue());
	}
		
	double Instance::weight(){
		return m_weight;
	}
		
	unsigned int Instance::numValues(){
		return m_valueVector.size();
	}
		
	double Instance::getValue(unsigned int ind){
		return m_valueVector[ind] ? m_valueVector[ind]->getValue() : 0;
	}
	
	bool Instance::missing(unsigned int ind){
		return m_valueVector[ind] ? false : true;
	}

	unsigned int Instance::index(unsigned int pos){
		return pos;
	}
}