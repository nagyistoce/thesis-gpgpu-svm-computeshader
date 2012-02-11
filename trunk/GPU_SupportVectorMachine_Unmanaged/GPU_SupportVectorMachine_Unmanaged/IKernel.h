#pragma once
#include "Instance.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	class IKernel{
	public:
		IKernel();

		double eval(int i1, int i2, InstancePtr inst);

		virtual double evaluate(int i1, int i2, InstancePtr inst) = 0;
		virtual void setParameters(double p1, double p2 = 1.0, double p3 = 1.0) = 0;

		void initKernel(IEvaluationPtr data);

		unsigned int getKernelEvals() { return m_kernelEval; }
		unsigned int getCacheHits() { return m_cacheHits; }
	protected:
		double dotProd(InstancePtr inst1, InstancePtr inst2);

		IEvaluationPtr m_data;
		std::vector<double> m_kernelPrecalc;

		std::vector<std::vector<double>> m_kernelMatrix;
		std::vector<double> m_storage;
		std::vector<long> m_keys;

		bool m_cacheActivated;
		bool m_fullCache;
		long m_cacheSize;
		unsigned int m_cacheSlots;

		long m_cacheHits;
		long m_kernelEval;
	};
}

typedef boost::shared_ptr<SVM_Framework::IKernel> IKernelPtr;