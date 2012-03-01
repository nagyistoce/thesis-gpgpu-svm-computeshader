#pragma once
#include "IEvaluation.h"

namespace SVM_Framework{
	class CrossValidation : public IEvaluation{
	public:
		CrossValidation(unsigned int folds);

		bool advance();
		void init();
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::CrossValidation> CrossValidationPtr;