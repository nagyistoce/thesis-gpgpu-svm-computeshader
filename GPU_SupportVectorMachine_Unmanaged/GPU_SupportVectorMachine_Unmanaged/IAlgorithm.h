#pragma once
#include "AlgorithmDataPack.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	class IAlgorithm{
	public:
		virtual ~IAlgorithm(){}
		virtual void run(AlgorithmDataPackPtr data) = 0;
	protected:
	};
}

typedef boost::shared_ptr<SVM_Framework::IAlgorithm> IAlgorithmPtr;