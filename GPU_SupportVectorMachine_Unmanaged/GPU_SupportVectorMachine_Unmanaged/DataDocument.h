#pragma once
#include "Value.h"
#include "Instance.h"

namespace SVM_Framework{
	class DataDocument{
	public:
		DataDocument();

		InstancePtr getInstance(unsigned int index);
		unsigned int getNumInstances();
		unsigned int getNumAttributes();

		std::string& newAttribute();
		void addValue(Value value, bool missing = false);
		void buildInstances();

		std::vector<std::string> m_attributes;
		std::vector<Value> m_data;
		std::set<unsigned int> m_missing;

		std::vector<InstancePtr> m_instances;

		unsigned int m_classAttributeId;
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::DataDocument> DataDocumentPtr;