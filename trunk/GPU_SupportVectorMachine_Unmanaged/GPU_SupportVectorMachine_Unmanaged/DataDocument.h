#pragma once
#include "Value.h"
#include "Instance.h"

namespace SVM_Framework{
	class DataDocument{
	public:
		enum InputFormat { IF_NUMERIC, IF_NOMINAL, IF_CLASS, IF_NULL };

		DataDocument();

		InstancePtr getInstance(unsigned int index);
		unsigned int getNumInstances();
		unsigned int getNumAttributes();

		std::string& newAttribute();
		void addValue(Value value, bool missing = false);
		void buildInstances();

		std::vector<std::string> m_attributes;
		std::vector<std::string> m_idCollumn;
		std::vector<Value> m_data;
		
		std::set<unsigned int> m_missing;

		std::vector<InstancePtr> m_instances;

		unsigned int m_classAttributeId;
		std::vector<InputFormat> m_format;

		int m_cl1Value,m_cl2Value;

		bool m_normalize;

		unsigned int	m_numCl1,
						m_numCl2;
	};
}

typedef boost::shared_ptr<SVM_Framework::DataDocument> DataDocumentPtr;