#include "stdafx.h"
#include "CPUSVM.h"
#include "ConfigManager.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "GUIManager.h"
#include "CrossValidation.h"

namespace SVM_Framework{
	CPUSVM::CPUSVM(GraphicsManagerPtr dxMgr){
	}
	
	CPUSVM::~CPUSVM(){
		for(unsigned int i=0; i<m_threads.size(); i++){
			m_threads[i]->interrupt();
		}
		for(unsigned int i=0; i<m_threads.size(); i++){
			m_threads[i]->join();
		}
	}

	void CPUSVM::execute(){
		unsigned int timerId = ConfigManager::startTimer();

		bool multithread = false;

		if(multithread){
			m_params.assign(5,AlgoParams());
			for(unsigned int i=0; i<5; i++){
				m_params[i].m_evaluation = IEvaluationPtr(new CrossValidation(10));
				m_params[i].m_evaluation->setData(m_document,m_data);
				boost::static_pointer_cast<CrossValidation>(m_params[i].m_evaluation)->setFold(i);

				m_threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(
					boost::bind(&CPUSVM::executeFold,this,i)
				)));
				if(SetThreadPriority(m_threads.back()->native_handle(),THREAD_PRIORITY_BELOW_NORMAL) == 0){
					assert(0);
				}
			}

			for(unsigned int i=0; i<5; i++){
				m_threads[i]->join();
				m_resultPack.cl1Correct += m_params[i].cl1Correct;
				m_resultPack.cl2Correct += m_params[i].cl2Correct;
				m_resultPack.cl1Wrong += m_params[i].cl1Wrong;
				m_resultPack.cl2Wrong += m_params[i].cl2Wrong;
				m_resultPack.iterations += m_params[i].iterations;
				m_resultPack.supportVectors += m_params[i].m_supportVectors->numElements();

				m_resultPack.cacheHits += m_params[i].m_kernel->getCacheHits();
				m_resultPack.kernelEvals += m_params[i].m_kernel->getKernelEvals();
				m_params[i].m_kernel->resetCounters();
			}
			m_threads.clear();

			for(unsigned int i=0; i<5; i++){
				m_params[i].m_evaluation = IEvaluationPtr(new CrossValidation(10));
				m_params[i].m_evaluation->setData(m_document,m_data);
				boost::static_pointer_cast<CrossValidation>(m_params[i].m_evaluation)->setFold(i+5);

				m_threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(
					boost::bind(&CPUSVM::executeFold,this,i)
				)));
				if(SetThreadPriority(m_threads.back()->native_handle(),THREAD_PRIORITY_BELOW_NORMAL) == 0){
					assert(0);
				}
			}

			for(unsigned int i=0; i<5; i++){
				m_threads[i]->join();
				m_resultPack.cl1Correct += m_params[i].cl1Correct;
				m_resultPack.cl2Correct += m_params[i].cl2Correct;
				m_resultPack.cl1Wrong += m_params[i].cl1Wrong;
				m_resultPack.cl2Wrong += m_params[i].cl2Wrong;
				m_resultPack.iterations += m_params[i].iterations;
				m_resultPack.supportVectors += m_params[i].m_supportVectors->numElements();

				m_resultPack.cacheHits += m_params[i].m_kernel->getCacheHits();
				m_resultPack.kernelEvals += m_params[i].m_kernel->getKernelEvals();
				m_params[i].m_kernel->resetCounters();
			}
			m_threads.clear();
		}
		else{
			m_params.assign(1,AlgoParams());
			m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(10));
			m_params[0].m_evaluation->setData(m_document,m_data);
			std::wstringstream stream;
			for(unsigned int i=0; i<10; i++){
				boost::static_pointer_cast<CrossValidation>(m_params[0].m_evaluation)->setFold(i);
				executeFold(0);
				m_resultPack.cl1Correct += m_params[0].cl1Correct;
				m_resultPack.cl2Correct += m_params[0].cl2Correct;
				m_resultPack.cl1Wrong += m_params[0].cl1Wrong;
				m_resultPack.cl2Wrong += m_params[0].cl2Wrong;
				m_resultPack.iterations += m_params[0].iterations;
				m_resultPack.supportVectors += m_params[0].m_supportVectors->numElements();

				m_resultPack.cacheHits += m_params[0].m_kernel->getCacheHits();
				m_resultPack.kernelEvals += m_params[0].m_kernel->getKernelEvals();
				m_params[0].m_kernel->resetCounters();
			}
		}

		m_resultPack.totalTime = ConfigManager::getTime(timerId);
		ConfigManager::removeTimer(timerId);

		m_threads.clear();
		m_params.clear();
	}

	void CPUSVM::lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2){
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

	void CPUSVM::updateErrorCache(float f, int i, int id){
		m_params[id].m_errors[i] = f;
	}

	double CPUSVM::SVMOutput(int index, InstancePtr inst, int id){
		double result = 0;

		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			result += m_params[id].m_class[i] * m_params[id].m_alpha[i] * m_params[id].m_kernel->eval(index, i, inst);
		}
		result -= m_params[id].m_b;
		
		return result;
	}

	void CPUSVM::testInstances(int id){
		// Testing
		InstancePtr testInst;
		int classValue;
		for(unsigned int i=0; i<m_params[id].m_evaluation->getNumTestingInstances(); i++){
			testInst = m_params[id].m_evaluation->getTestingInstance(i);
			double output = SVMOutput(-1,testInst,id);
			classValue = testInst->classValue();
			if(output < 0){
				if(testInst->classValue() == m_document->m_cl1Value)
					m_params[id].cl1Correct++;
				else
					m_params[id].cl1Wrong++;
			}
			else{
				if(testInst->classValue() == m_document->m_cl2Value)
					m_params[id].cl2Correct++;
				else
					m_params[id].cl2Wrong++;
			}
		}
	}

	void CPUSVM::kernelEvaluations(std::vector<int> &inds, std::vector<float> &result, int id){
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