#pragma once
#include "Value.h"

namespace SVM_Framework{
	class Instance{
	public:
		Instance(int classInd, unsigned int size);

		void insertValue(Value* value);

		int classValue();
		double weight();
		unsigned int numValues();
		double getValue(unsigned int ind);
		bool missing(unsigned int ind);

		unsigned int index(unsigned int pos);
		
	private:
		std::vector<Value*> m_valueVector;
		Value* m_classValue;

		unsigned int m_classInd; 
		double m_weight;

		friend class DataDocument;
	};
}

typedef boost::shared_ptr<SVM_Framework::Instance> InstancePtr;