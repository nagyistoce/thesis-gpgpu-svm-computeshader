#pragma once
#include "Value.h"
#include "Instance.h"

namespace SVM_Framework{
	class DataDocument{
	public:
		DataDocument();
	private:
		std::vector<std::string> m_attributes;
		std::vector<Value> m_data;

		std::vector<InstancePtr> m_instances;
	};
}

typedef boost::shared_ptr<SVM_Framework::DataDocument> DataDocumentPtr;