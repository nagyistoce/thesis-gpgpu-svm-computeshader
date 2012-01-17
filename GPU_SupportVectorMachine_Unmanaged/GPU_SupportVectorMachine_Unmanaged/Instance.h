#pragma once

namespace SVM_Framework{
	class Instance{
	public:
	private:
		std::vector<ValuePtr> m_valueVector;
	};
}

typedef boost::shared_ptr<SVM_Framework::Instance> InstancePtr;