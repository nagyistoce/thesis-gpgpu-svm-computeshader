#include "stdafx.h"
#include "RBFKernel.h"

namespace SVM_Framework{
	RBFKernel::RBFKernel(){
		m_gamma = 0.1;
		m_id = 1;
	}

	double RBFKernel::evaluate(int i1, int i2, InstancePtr inst){
		if (i1 == i2){
			return 1.0;
		} 
		else{
			double precalc1;
			if (i1 == -1)
				precalc1 = dotProd(inst, inst);
			else
				precalc1 = m_kernelPrecalc[i1];
			InstancePtr inst2 = m_data->getTrainingInstance(i2);
			double result = (m_gamma * (2. * dotProd(inst, inst2) - precalc1 - m_kernelPrecalc[i2]));

			return result;
		}
	}

	void RBFKernel::setParameters(double p1, double p2, double p3){
		m_gamma = p1;
	}

	double RBFKernel::getParameter(int i){
		return m_gamma;
	}
}