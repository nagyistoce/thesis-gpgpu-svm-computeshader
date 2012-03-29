#pragma once
#include "IEvaluation.h"

namespace SVM_Framework{
	class CrossValidation : public IEvaluation{
	public:
		CrossValidation(unsigned int folds);

		bool advance();
		void init();
		bool isFinalStage();
	private:
		std::vector<InstancePtr> m_validationSet;
	};
}

typedef boost::shared_ptr<SVM_Framework::CrossValidation> CrossValidationPtr;