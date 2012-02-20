#pragma once

namespace SVM_Framework{
	class Value{
	public:
		Value(float value){
			m_value = value;
		}

		float getValue() {return m_value;}
	private:
		float m_value;
	};
}

typedef boost::shared_ptr<SVM_Framework::Value> ValuePtr;