#pragma once
#include "IEvaluation.h"

namespace SVM_Framework{
	class CrossValidation : public IEvaluation{
	public:
		CrossValidation(unsigned int folds);

		InstancePtr getTrainingInstance(unsigned int index);
		InstancePtr getTestingInstance(unsigned int index);

		unsigned int getNumTrainingInstances();
		unsigned int getNumTestingInstances();

		bool advance();
		void init();

		void setNumFolds(unsigned int folds);
		void setFold(unsigned int fold);
	private:
		unsigned int	m_folds;
		unsigned int	m_currentFold;

		unsigned int	m_foldStart,
						m_foldEnd;
	};
}

typedef boost::shared_ptr<SVM_Framework::CrossValidation> CrossValidationPtr;