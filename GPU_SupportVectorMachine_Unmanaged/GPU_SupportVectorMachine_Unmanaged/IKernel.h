#pragma once
#include "Instance.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	class IKernel{
	public:
		double eval(int i1, int i2, InstancePtr inst);

		virtual double evaluate(int i1, int i2, InstancePtr inst) = 0;
		virtual void setParameters(double p1, double p2 = 1.0, double p3 = 1.0) = 0;

		void initKernel(IEvaluationPtr data);
	protected:
		double dotProd(InstancePtr inst1, InstancePtr inst2);

		IEvaluationPtr m_data;
		std::vector<double> m_kernelPrecalc;

		std::vector<std::vector<double>> m_kernelMatrix;
	};
}

typedef boost::shared_ptr<SVM_Framework::IKernel> IKernelPtr;