#include "stdafx.h"
#include "IKernel.h"

namespace SVM_Framework{
	IKernel::IKernel():m_cacheActivated(true),m_fullCache(false){
		m_cacheSize = 2000000;
		m_cacheSlots = 4;

		if(m_cacheActivated && !m_fullCache){
			m_storage.assign(m_cacheSize * m_cacheSlots,0);
			m_keys.assign(m_cacheSize * m_cacheSlots,0);
		}
	}

	double IKernel::eval(int i1, int i2, InstancePtr inst){
		if(i1 >= 0 && m_cacheActivated){
			double result;
			if(m_fullCache){
				result = (i1 > i2) ? m_kernelMatrix[i1][i2] : m_kernelMatrix[i2][i1];
				m_cacheHits++;
			}
			else{
				long key = -1;
				int location = -1;

				// Use LRU cache
				if (i1 > i2) {
					key = (i1 + ((long) i2 * m_data->getNumTrainingInstances()));
				}
				else{
					key = (i2 + ((long) i1 * m_data->getNumTrainingInstances()));
				}
				location = (int) (key % m_cacheSize) * m_cacheSlots;
				int loc = location;
				for (int i = 0; i < m_cacheSlots; i++) {
					long thiskey = m_keys[loc];
					if (thiskey == 0)
						break; // empty slot, so break out of loop early
					if (thiskey == (key + 1)) {
						m_cacheHits++;
						// move entry to front of cache (LRU) by swapping
						// only if it's not already at the front of cache
						if (i > 0) {
							double tmps = m_storage[loc];
							m_storage[loc] = m_storage[location];
							m_keys[loc] = m_keys[location];
							m_storage[location] = tmps;
							m_keys[location] = thiskey;
							return tmps;
						}
						else
							return m_storage[loc];
					}
					loc++;
				}

				result = evaluate(i1, i2, inst);

				m_kernelEval++;
				// store result in cache
				if ( (key != -1) && (m_cacheSize != -1) ) {
					// move all cache slots forward one array index
					// to make room for the new entry
					for(unsigned int i=location+(m_cacheSlots-1); i>location; i--){
						m_keys[i] = m_keys[i-1];
						m_storage[i] = m_storage[i-1];
					}
					m_storage[location] = result;
					m_keys[location] = (key + 1);
				}
			}
			return result;
		}

		m_kernelEval++;
		return evaluate(i1, i2, inst);
	}

	void IKernel::initKernel(IEvaluationPtr data){
		m_data = data;
		m_kernelPrecalc.clear();
		m_kernelPrecalc.reserve(m_data->getNumTrainingInstances());
		m_kernelMatrix.clear();

		m_cacheHits = 0;
		m_kernelEval = 0;

		for(unsigned int i = 0; i < m_data->getNumTrainingInstances(); i++)
			m_kernelPrecalc.push_back(dotProd(m_data->getTrainingInstance(i), m_data->getTrainingInstance(i)));

		if(m_cacheActivated && m_fullCache){
			m_kernelMatrix.assign(m_data->getNumTrainingInstances(),std::vector<double>());
			for(unsigned int i = 0; i < m_data->getNumTrainingInstances(); i++) {
				m_kernelMatrix[i].reserve(i+1);
				for(unsigned int j = 0; j <= i; j++){
					m_kernelEval++;
					m_kernelMatrix[i].push_back(evaluate(i, j, m_data->getTrainingInstance(i)));
				}
			}
		}
	}

	double IKernel::dotProd(InstancePtr inst1, InstancePtr inst2){
		double result = 0;

		// we can do a fast dot product
		int n1 = inst1->numValues();
		int n2 = inst2->numValues();
		for (int p1 = 0, p2 = 0; p1 < n1 && p2 < n2;) {
			if (p1 == p2) {
				result += inst1->getValue(p1) * inst2->getValue(p2);
				p1++;
				p2++;
			} 
			else if (p1 > p2) {
				p2++;
			} 
			else {
				p1++;
			}
		}
		return (result);
	}
}