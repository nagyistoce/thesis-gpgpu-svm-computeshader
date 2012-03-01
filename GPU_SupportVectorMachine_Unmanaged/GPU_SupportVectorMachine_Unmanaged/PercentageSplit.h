#pragma once
#include "IEvaluation.h"

namespace SVM_Framework{
	class PercentageSplit : public IEvaluation{
	public:
		PercentageSplit(float splitPercentage);

		bool advance();
		void init();
	private:
		bool m_advance;
		float m_splitPercentage;
	};
}