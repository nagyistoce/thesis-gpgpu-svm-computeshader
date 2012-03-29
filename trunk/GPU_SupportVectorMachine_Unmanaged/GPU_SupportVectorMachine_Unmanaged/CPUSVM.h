#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"

namespace SVM_Framework{
	class CPUSVM : public ISVM{
	public:
		CPUSVM(GraphicsManagerPtr dxMgr);
		~CPUSVM();
	
	protected:
		void beginExecute();
		void endExecute();
		void lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(svm_precision f, int i, int id);
		svm_precision SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(std::vector<svm_precision> &finalResult, int id);
		void kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id);
	};
}