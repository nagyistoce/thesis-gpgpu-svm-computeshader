#pragma once

namespace SVM_Framework{
	class Value : public boost::enable_shared_from_this<Value>{
	public:
		Value(float value){
			m_value = value;
		}
	private:
		float m_value;
	};
}

typedef boost::shared_ptr<SVM_Framework::Value> ValuePtr;