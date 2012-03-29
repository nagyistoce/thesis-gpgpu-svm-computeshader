#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"
#include <cuda_runtime.h>

namespace SVM_Framework{
	class CUDASVM : public ISVM{
	public:
		CUDASVM(GraphicsManagerPtr dxMgr);
		~CUDASVM();

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
	private:
		svm_precision *b_output;
		svm_precision *b_inputData;
		svm_precision *b_alpha;
		svm_precision *b_class;
		svm_precision *b_selfProd;
		unsigned int *b_trainingInds;
		unsigned int *b_testingInds;
		unsigned int *b_inputInds;

		unsigned int m_numDevices;
	};
}