#pragma once
#include "Value.h"

namespace SVM_Framework{
	class Instance{
	public:
		Instance(int classInd, unsigned int size, unsigned int index);

		void insertValue(Value* value);

		int classValue();
		Value::v_precision weight();
		unsigned int numValues();
		Value::v_precision getValue(unsigned int ind);
		bool missing(unsigned int ind);

		unsigned int getIndex();
		
	private:
		std::vector<Value*> m_valueVector;
		Value* m_classValue;

		unsigned int m_classInd;
		unsigned int m_index;
		Value::v_precision m_weight;

		friend class DataDocument;
	};
}

typedef boost::shared_ptr<SVM_Framework::Instance> InstancePtr;