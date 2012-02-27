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
		std::wstringstream outputStream;;
		unsigned int timerId = ConfigManager::startTimer();

		long	cl1Correct = 0,
				cl1Wrong = 0,
				cl2Correct = 0,
				cl2Wrong = 0,
				iterations = 0;
		long	cacheHits = 0,
				kernelEvals = 0,
				supportVectors = 0;

		bool multithread = false;

		if(multithread){
			outputStream << L"Working on batch 1...\r\n";
			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,outputStream.str());
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
				cl1Correct += m_params[i].cl1Correct;
				cl2Correct += m_params[i].cl2Correct;
				cl1Wrong += m_params[i].cl1Wrong;
				cl2Wrong += m_params[i].cl2Wrong;
				iterations += m_params[i].iterations;
				supportVectors += m_params[i].m_supportVectors->numElements();

				cacheHits += m_params[i].m_kernel->getCacheHits();
				kernelEvals += m_params[i].m_kernel->getKernelEvals();
				m_params[i].m_kernel->resetCounters();
			}
			m_threads.clear();

			outputStream << "Intermediate time: " << ConfigManager::getTime(timerId);

			outputStream	<< "\r\n\r\n" << m_document->m_cl1Value << "	" << m_document->m_cl2Value 
							<< "\r\n" << cl1Correct << "	" << cl1Wrong << "	" << m_document->m_cl1Value 
							<< "\r\n" << cl2Wrong << "	" << cl2Correct << "	" << m_document->m_cl2Value << "\r\n";
			outputStream << "Accuracy: " << (float(cl1Correct+cl2Correct)/float(cl1Correct+cl2Correct+cl1Wrong+cl2Wrong))*100 << "%\r\n\r\n";
			outputStream << "Total number tested: " << cl1Correct+cl2Correct+cl1Wrong+cl2Wrong << "\r\n";
			outputStream << "Iteration count: " << iterations << "\r\n";
			outputStream << "Cachehits: " << cacheHits << " (" << (double(cacheHits)/double(cacheHits+kernelEvals))*100 << "%)\r\n";
			outputStream <<	"Kernel evals: " << kernelEvals << "\r\n\r\n";
			outputStream << "Working on batch 2...\r\n";

			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,outputStream.str());

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
				cl1Correct += m_params[i].cl1Correct;
				cl2Correct += m_params[i].cl2Correct;
				cl1Wrong += m_params[i].cl1Wrong;
				cl2Wrong += m_params[i].cl2Wrong;
				iterations += m_params[i].iterations;
				supportVectors += m_params[i].m_supportVectors->numElements();

				cacheHits += m_params[i].m_kernel->getCacheHits();
				kernelEvals += m_params[i].m_kernel->getKernelEvals();
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
				stream << "Working on fold " << i+1 << "...\r\n";
				m_data->m_gui->setText(IDC_STATIC_INFOTEXT,stream.str());
				boost::static_pointer_cast<CrossValidation>(m_params[0].m_evaluation)->setFold(i);
				executeFold(0);
				cl1Correct += m_params[0].cl1Correct;
				cl2Correct += m_params[0].cl2Correct;
				cl1Wrong += m_params[0].cl1Wrong;
				cl2Wrong += m_params[0].cl2Wrong;
				iterations += m_params[0].iterations;
				supportVectors += m_params[0].m_supportVectors->numElements();

				cacheHits += m_params[0].m_kernel->getCacheHits();
				kernelEvals += m_params[0].m_kernel->getKernelEvals();
				m_params[0].m_kernel->resetCounters();
			}
		}
		
		outputStream << "Total time: " << ConfigManager::getTime(timerId);

		outputStream	<< "\r\n\r\n" << m_document->m_cl1Value << "	" << m_document->m_cl2Value 
						<< "\r\n" << cl1Correct << "	" << cl1Wrong << "	" << m_document->m_cl1Value 
						<< "\r\n" << cl2Wrong << "	" << cl2Correct << "	" << m_document->m_cl2Value << "\r\n";
		outputStream << "Accuracy: " << (float(cl1Correct+cl2Correct)/float(cl1Correct+cl2Correct+cl1Wrong+cl2Wrong))*100 << "%\r\n\r\n";
		outputStream << "Total number tested: " << cl1Correct+cl2Correct+cl1Wrong+cl2Wrong << "\r\n";
		outputStream << "Iteration count: " << iterations/10 << "\r\n";
		outputStream << "Support vectors: " << supportVectors/10 << "\r\n";
		outputStream << "Cachehits: " << cacheHits/10 << " (" << (double(cacheHits/10)/double((cacheHits/10)+(kernelEvals/10)))*100 << "%)\r\n";
		outputStream <<	"Kernel evals: " << kernelEvals/10;

		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,outputStream.str());
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

	void CPUSVM::initializeFold(int id){
		
	}

	void CPUSVM::endFold(int id){
		
	}
}