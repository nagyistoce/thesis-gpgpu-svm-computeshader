#pragma once
#include "IKernel.h"

namespace SVM_Framework{
	class PuKKernel : public IKernel{
	public:
		PuKKernel();

		double evaluate(int i1, int i2, InstancePtr inst);
		void setParameters(double p1, double p2 = 1.0, double p3 = 1.0);
	private:
		void computeFactor();

		double	m_omega, 
				m_sigma,
				m_factor;
	};
}