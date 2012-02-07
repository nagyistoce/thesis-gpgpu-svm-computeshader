#pragma once

namespace SVM_Framework{
	class Value : public boost::enable_shared_from_this<Value>{
	public:
		Value(double value){
			m_value = value;
		}

		double getValue() {return m_value;}
	private:
		double m_value;
	};
}

typedef boost::shared_ptr<SVM_Framework::Value> ValuePtr;