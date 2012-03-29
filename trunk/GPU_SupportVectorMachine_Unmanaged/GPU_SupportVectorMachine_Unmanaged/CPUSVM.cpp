#include "stdafx.h"
#include "CPUSVM.h"
#include "ConfigManager.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "GUIManager.h"
#include "CrossValidation.h"
#include "PercentageSplit.h"

namespace SVM_Framework{
	CPUSVM::CPUSVM(GraphicsManagerPtr dxMgr){
	}
	
	CPUSVM::~CPUSVM(){
	}

	void CPUSVM::beginExecute(){
		
	}

	void CPUSVM::endExecute(){
		
	}

	void CPUSVM::lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2){
		// Update error cache using new Lagrange multipliers
		// Update thresholds
		m_params[id].m_bLow = -DBL_MAX; m_params[id].m_bUp = DBL_MAX;
		m_params[id].m_iLow = -1; m_params[id].m_iUp = -1;
		for (int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)) {
			m_params[id].m_errors[j] +=	p1 * m_params[id].m_kernel->eval(i1, j, m_params[id].m_evaluation->getTrainingInstance(i1)) + 
										p2 * m_params[id].m_kernel->eval(i2, j, m_params[id].m_evaluation->getTrainingInstance(i2));

			if (m_params[id].m_errors[j] < m_params[id].m_bUp) {
				m_params[id].m_bUp = m_params[id].m_errors[j];
				m_params[id].m_iUp = j;
			}
			if(m_params[id].m_errors[j] > m_params[id].m_bLow) {
				m_params[id].m_bLow = m_params[id].m_errors[j];
				m_params[id].m_iLow = j;
			}
		}
	}

	void CPUSVM::updateErrorCache(svm_precision f, int i, int id){
		m_params[id].m_errors[i] = f;
	}

	ISVM::svm_precision CPUSVM::SVMOutput(int index, InstancePtr inst, int id){
		svm_precision result = 0;

		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			result += m_params[id].m_class[i] * m_params[id].m_alpha[i] * m_params[id].m_kernel->eval(index, i, inst);
		}
		result -= m_params[id].m_b;
		
		return result;
	}

	void CPUSVM::testInstances(std::vector<svm_precision> &finalResult, int id){
		// Testing
		InstancePtr testInst;
		int classValue;
		for(unsigned int i=0; i<m_params[id].m_evaluation->getNumTestingInstances(); i++){
			testInst = m_params[id].m_evaluation->getTestingInstance(i);
			finalResult.push_back(SVMOutput(-1,testInst,id));
		}
	}

	void CPUSVM::kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id){
		if(inds.size() < 2)
			return;

		for(unsigned int i=0; i<inds.size(); i+=2){
			result.push_back(m_params[id].m_kernel->eval(inds[i], inds[i+1], m_params[id].m_evaluation->getTrainingInstance(inds[i])));
		}
	}

	void CPUSVM::initializeFold(int id){
		
	}

	void CPUSVM::endFold(int id){
		
	}
}