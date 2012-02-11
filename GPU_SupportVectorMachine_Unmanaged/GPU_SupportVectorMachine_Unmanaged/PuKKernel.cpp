#include "stdafx.h"
#include "PuKKernel.h"

namespace SVM_Framework{
	PuKKernel::PuKKernel(){
		m_omega = 1.0;
		m_sigma = 1.0;

		computeFactor();
	}

	double PuKKernel::evaluate(int i1, int i2, InstancePtr inst){
		if (i1 == i2) {
			return 1.0;
		}
		else{
			double precalc1;
			if (i1 == -1)
				precalc1 = dotProd(inst, inst);
			else
				precalc1 = m_kernelPrecalc[i1];
			InstancePtr inst2 = m_data->getTrainingInstance(i2);
			double squaredDifference = -2.0 * dotProd(inst, inst2) + precalc1 + m_kernelPrecalc[i2];
			double intermediate = m_factor * sqrt(squaredDifference);
			double result = 1.0 / pow(1.0 + intermediate * intermediate,m_omega);

			return result;
		}
	}

	void PuKKernel::setParameters(double p1, double p2, double p3){
		m_omega = p1;
		m_sigma = p2;
		computeFactor();
	}

	void PuKKernel::computeFactor(){
		double root = sqrt(pow(2.0, 1.0 / m_omega) - 1);
		m_factor = (2.0 * root) / m_sigma;
	}
}