#pragma once
#include "IAlgorithm.h"
#include "IKernel.h"
#include "SMOSet.h"

namespace SVM_Framework{
	class ISVM : public IAlgorithm{
	public:
		ISVM();
		~ISVM(){}

		void run(AlgorithmDataPackPtr data);
	protected:
		virtual void execute() = 0;

		DataDocumentPtr m_document;
		AlgorithmDataPackPtr m_data;
	};
}