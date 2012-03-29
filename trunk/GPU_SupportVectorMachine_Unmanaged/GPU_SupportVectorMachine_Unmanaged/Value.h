#pragma once

namespace SVM_Framework{
	class Value{
	public:
		typedef float v_precision;

		Value(v_precision value){
			m_value = value;
		}

		v_precision getValue() {return m_value;}
	private:
		v_precision m_value;
	};
}

typedef boost::shared_ptr<SVM_Framework::Value> ValuePtr;