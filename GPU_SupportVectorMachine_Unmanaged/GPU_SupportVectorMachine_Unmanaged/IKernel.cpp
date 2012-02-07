#include "stdafx.h"
#include "IKernel.h"

namespace SVM_Framework{
	double IKernel::eval(int i1, int i2, InstancePtr inst){
		if(i1 >= 0){
			double result = (i1 > i2) ? m_kernelMatrix[i1][i2] : m_kernelMatrix[i2][i1];
			return result;
		}

		return evaluate(i1, i2, inst);
	}

	void IKernel::initKernel(IEvaluationPtr data){
		m_data = data;
		m_kernelPrecalc.clear();
		m_kernelPrecalc.reserve(m_data->getNumTrainingInstances());
		m_kernelMatrix.clear();

		for(unsigned int i = 0; i < m_data->getNumTrainingInstances(); i++)
			m_kernelPrecalc.push_back(dotProd(m_data->getTrainingInstance(i), m_data->getTrainingInstance(i)));

		m_kernelMatrix.assign(m_data->getNumTrainingInstances(),std::vector<double>());
		for(unsigned int i = 0; i < m_data->getNumTrainingInstances(); i++) {
			m_kernelMatrix[i].reserve(i+1);
			for(unsigned int j = 0; j <= i; j++){
				m_kernelMatrix[i].push_back(evaluate(i, j, m_data->getTrainingInstance(i)));
			}
		}
	}

	double IKernel::dotProd(InstancePtr inst1, InstancePtr inst2){
		double result = 0;

		// we can do a fast dot product
		int n1 = inst1->numValues();
		int n2 = inst2->numValues();
		for (int p1 = 0, p2 = 0; p1 < n1 && p2 < n2;) {
			int ind1 = inst1->index(p1);
			int ind2 = inst2->index(p2);
			if (ind1 == ind2) {
				result += inst1->getValue(p1) * inst2->getValue(p2);
				p1++;
				p2++;
			} 
			else if (ind1 > ind2) {
				p2++;
			} 
			else {
				p1++;
			}
		}
		return (result);
	}
}