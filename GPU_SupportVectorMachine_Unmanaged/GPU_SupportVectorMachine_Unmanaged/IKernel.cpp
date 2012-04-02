#include "stdafx.h"
#include "IKernel.h"

namespace SVM_Framework{
	IKernel::IKernel(bool cache, bool fullCache):m_cacheActivated(cache),m_fullCache(fullCache){
		m_cacheSize = 30000000;
		m_cacheSlots = 4;

		if(m_cacheActivated && !m_fullCache){
			m_storage.assign(m_cacheSize * m_cacheSlots,0);
			m_keys.assign(m_cacheSize * m_cacheSlots,0);
		}
	}

	void IKernel::resetCounters(){
		m_cacheHits = 0;
		m_kernelEval = 0;
	}

	bool IKernel::isCached(int i1, int i2, Value::v_precision &result){
		if(!m_cacheActivated)
			false;

		if(i1 == i2){
			result = evaluate(i1,i2,m_data->getTrainingInstance(i1));
			return true;
		}

		if(m_fullCache){
			result = (i1 > i2) ? m_kernelMatrix[i1][i2] : m_kernelMatrix[i2][i1];
			m_cacheHits++;
		}
		else{
			m_key = -1;
			m_location = -1;

			// Use LRU cache
			if (i1 > i2) {
				m_key = (i1 + ((long) i2 * m_data->getNumTrainingInstances()));
			}
			else{
				m_key = (i2 + ((long) i1 * m_data->getNumTrainingInstances()));
			}
			m_location = (int) (m_key % m_cacheSize) * m_cacheSlots;
			m_loc = m_location;
			if(m_loc >= 0 && m_loc < m_keys.size()){
				for (int i = 0; i < m_cacheSlots; i++) {
					long thiskey = m_keys[m_loc];
					if (thiskey == 0)
						return false; // empty slot, so break out of loop early
					if (thiskey == (m_key + 1)) {
						m_cacheHits++;
						// move entry to front of cache (LRU) by swapping
						// only if it's not already at the front of cache
						if (i > 0) {
							Value::v_precision tmps = m_storage[m_loc];
							m_storage[m_loc] = m_storage[m_location];
							m_keys[m_loc] = m_keys[m_location];
							m_storage[m_location] = tmps;
							m_keys[m_location] = thiskey;

							result = tmps;
							return true;
						}
						else{
							result = m_storage[m_loc];
							return true;
						}
					}
					m_loc++;
				}
			}
		}
		return false;
	}

	void IKernel::insertIntoCache(std::vector<unsigned int> &inds, std::vector<Value::v_precision> &values, int index, int index2){
		if(!m_cacheActivated)
			return;

		Value::v_precision cacheCheck;
		int add = 1;
		if(index2 != -1)
			add = 2;
		for(unsigned int j=0; j<inds.size(); j++){
			m_kernelEval += add;
			m_key = -1;
			m_location = -1;

			if(!isCached(inds[j],index,cacheCheck)){
				// Use LRU cache
				if (inds[j*add] > index) {
					m_key = (inds[j] + ((long) index * m_data->getNumTrainingInstances()));
				}
				else{
					m_key = (index + ((long) inds[j] * m_data->getNumTrainingInstances()));
				}
				m_location = (int) (m_key % m_cacheSize) * m_cacheSlots;
				m_loc = m_location;

				if(m_loc >= 0 && m_loc < m_keys.size()){
					// store result in cache
					if ( (m_key != -1) && (m_cacheSize != -1) && !(inds[j] == index)) {
						// move all cache slots forward one array index
						// to make room for the new entry
						for(unsigned int i=m_location+(m_cacheSlots-1); i>m_location; i--){
							m_keys[i] = m_keys[i-1];
							m_storage[i] = m_storage[i-1];
						}
						m_storage[m_location] = values[j*add];
						m_keys[m_location] = (m_key + 1);
					}
				}
			}

			if(index2 != -1 && index != index2){
				m_key = -1;
				m_location = -1;

				if(!isCached(inds[j],index2,cacheCheck)){
					// Use LRU cache
					if (inds[j] > index2) {
						m_key = (inds[j] + ((long) index2 * m_data->getNumTrainingInstances()));
					}
					else{
						m_key = (index2 + ((long) inds[j] * m_data->getNumTrainingInstances()));
					}
					m_location = (int) (m_key % m_cacheSize) * m_cacheSlots;
					m_loc = m_location;

					if(m_loc >= 0 && m_loc < m_keys.size()){
						// store result in cache
						if ( (m_key != -1) && (m_cacheSize != -1) && !(inds[j] == index2)) {
							// move all cache slots forward one array index
							// to make room for the new entry
							for(unsigned int i=m_location+(m_cacheSlots-1); i>m_location; i--){
								m_keys[i] = m_keys[i-1];
								m_storage[i] = m_storage[i-1];
							}
							m_storage[m_location] = values[(j*add)+1];
							m_keys[m_location] = (m_key + 1);
						}
					}
				}
			}
		}
	}

	Value::v_precision IKernel::eval(int i1, int i2, InstancePtr inst){
		if(i1 == i2)
			return evaluate(i1,i2,inst);

		if(i1 >= 0 && m_cacheActivated){
			Value::v_precision result;
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
				if(loc >= 0 && loc < m_keys.size()){
					for (int i = 0; i < m_cacheSlots; i++) {
						long thiskey = m_keys[loc];
						if (thiskey == 0)
							break; // empty slot, so break out of loop early
						if (thiskey == (key + 1)) {
							m_cacheHits++;
							// move entry to front of cache (LRU) by swapping
							// only if it's not already at the front of cache
							if (i > 0) {
								Value::v_precision tmps = m_storage[loc];
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
				}

				result = evaluate(i1, i2, inst);

				m_kernelEval++;

				if(loc >= 0 && loc < m_keys.size()){
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
			m_kernelMatrix.assign(m_data->getNumTrainingInstances(),std::vector<Value::v_precision>());
			for(unsigned int i = 0; i < m_data->getNumTrainingInstances(); i++) {
				m_kernelMatrix[i].reserve(i+1);
				for(unsigned int j = 0; j <= i; j++){
					m_kernelEval++;
					m_kernelMatrix[i].push_back(evaluate(i, j, m_data->getTrainingInstance(i)));
				}
			}
		}
	}

	Value::v_precision IKernel::dotProd(InstancePtr inst1, InstancePtr inst2){
		Value::v_precision result = 0;

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