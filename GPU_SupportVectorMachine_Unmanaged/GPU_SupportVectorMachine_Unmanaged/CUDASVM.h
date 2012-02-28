#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"

namespace SVM_Framework{
	class CUDASVM : public ISVM{
	public:
		CUDASVM(GraphicsManagerPtr dxMgr);
		~CUDASVM();

	protected:
		void execute();
		void lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(float f, int i, int id);
		double SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(int id);
		void kernelEvaluations(std::vector<int> &inds, std::vector<float> &result, int id);
	private:
	};
}