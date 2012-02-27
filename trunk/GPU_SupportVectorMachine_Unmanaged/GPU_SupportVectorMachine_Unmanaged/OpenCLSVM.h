#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"

namespace SVM_Framework{
	class OpenCLSVM : public ISVM{
	public:
		OpenCLSVM(GraphicsManagerPtr dxMgr);
		~OpenCLSVM();

	protected:
		void execute();
		void lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(float f, int i, int id);
		double SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(int id);
	private:
	};
}