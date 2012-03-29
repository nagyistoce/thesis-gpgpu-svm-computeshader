#pragma once
#include "IKernel.h"

namespace SVM_Framework{
	class RBFKernel : public IKernel{
	public:
		RBFKernel(bool cache, bool fullCache);

		double evaluate(int i1, int i2, InstancePtr inst);
		void setParameters(double p1, double p2 = 1.0, double p3 = 1.0);
		double getParameter(int i);
	private:
		double m_gamma;
	};
}