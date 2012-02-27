#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"

namespace SVM_Framework{
	class CPUSVM : public ISVM{
	public:
		CPUSVM(GraphicsManagerPtr dxMgr);
		~CPUSVM();
	
	protected:
		void execute();
		void lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(float f, int i, int id);
		double SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(int id);

		std::vector<boost::shared_ptr<boost::thread>> m_threads;
	};
}