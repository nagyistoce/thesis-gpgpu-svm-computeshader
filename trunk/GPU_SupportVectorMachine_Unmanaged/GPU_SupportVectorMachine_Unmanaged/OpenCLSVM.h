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
		int examineExample(int i2);
		int takeStep(int i1, int i2, double F2);
		double SVMOutput(int index, InstancePtr inst);
	private:
	};
}