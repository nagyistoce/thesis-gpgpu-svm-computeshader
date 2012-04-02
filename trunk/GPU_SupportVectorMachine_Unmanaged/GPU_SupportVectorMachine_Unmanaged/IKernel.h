#pragma once
#include "Instance.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	class IKernel{
	public:
		IKernel(bool cache, bool fullCache);

		Value::v_precision eval(int i1, int i2, InstancePtr inst);

		virtual double evaluate(int i1, int i2, InstancePtr inst) = 0;
		virtual void setParameters(double p1, double p2 = 1.0, double p3 = 1.0) = 0;
		virtual double getParameter(int i) = 0;

		void initKernel(IEvaluationPtr data);
		void resetCounters();

		unsigned int getId() { return m_id; }
		long getKernelEvals() { return m_kernelEval; }
		long getCacheHits() { return m_cacheHits; }

		bool isCached(int i1, int i2, Value::v_precision &result);
		void insertIntoCache(std::vector<unsigned int> &inds, std::vector<Value::v_precision> &values, int index, int index2 = -1);

		Value::v_precision dotProd(InstancePtr inst1, InstancePtr inst2);
	protected:
		IEvaluationPtr m_data;
		std::vector<Value::v_precision> m_kernelPrecalc;

		std::vector<std::vector<Value::v_precision>> m_kernelMatrix;
		std::vector<Value::v_precision> m_storage;
		std::vector<long> m_keys;

		bool m_cacheActivated;
		bool m_fullCache;
		long m_cacheSize;
		unsigned int m_cacheSlots;

		unsigned long m_cacheHits;
		unsigned long m_kernelEval;

		unsigned int m_id;

		long m_key;
		int m_location,
			m_loc;
	};
}

typedef boost::shared_ptr<SVM_Framework::IKernel> IKernelPtr;