#include "stdafx.h"
#include "ISVM.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "CrossValidation.h"
#include "PercentageSplit.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	ISVM::ISVM():m_eps(1.0e-12),m_tol(1.0e-3),m_C(1.0),m_multiThreaded(false){
		m_Del = 1000 * DBL_MIN;
		m_sharedBuffer.push_back(SharedBuffer());
	}

	void ISVM::run(AlgorithmDataPackPtr data){
		m_data = data;
		m_document = m_data->m_recMgr->getDocumentResource(m_data->m_dataResource);
		m_outputStream = std::wstringstream();

		// Set C
		try{
			m_C = boost::lexical_cast<svm_precision>(m_data->m_gui->getEditText(IDC_EDIT_C));
		}
		catch(...){
			m_C = 1.0;
		}

		m_alphaTransferNeeded = true;
		beginExecute();
		execute();
		endExecute();

		m_data->m_gui->enableAllButStop();

		if(m_data->m_callBack){
			(*m_data->m_callBack)(shared_from_this());
		}
	}

	void ISVM::meassurements(std::vector<svm_precision> &results, int id){
		std::map<svm_precision,std::vector<int>> class1Sorted,class2Sorted;
		unsigned int class1 = 0;
		unsigned int class2 = 0;
		unsigned int cl1Correct = 0;
		unsigned int cl2Correct = 0;
		unsigned int cl1Wrong = 0;
		unsigned int cl2Wrong = 0;

		// Testing
		InstancePtr testInst;
		int classValue;
		for(unsigned int i=0; i<results.size(); i++){
			testInst = m_params[id].m_evaluation->getTestingInstance(i);
			classValue = testInst->classValue();
			if(classValue == m_document->m_cl1Value){
				class1++;
			}
			else{
				class2++;
			}

			if(results[i] < 0){
				class1Sorted[abs(results[i])].push_back(i);

				if(classValue == m_document->m_cl1Value)
					cl1Correct++;
				else
					cl1Wrong++;
			}
			else{
				class2Sorted[abs(results[i])].push_back(i);

				if(classValue == m_document->m_cl2Value)
					cl2Correct++;
				else
					cl2Wrong++;
			}
		}

		m_params[id].m_resultPack.cl1Correct = cl1Correct;
		m_params[id].m_resultPack.cl2Correct = cl2Correct;
		m_params[id].m_resultPack.cl1Wrong = cl1Wrong;
		m_params[id].m_resultPack.cl2Wrong = cl2Wrong;

		unsigned int topTenPercentCount = unsigned int(svm_precision(results.size())*0.1f);
		unsigned int correctClass = 0;
		unsigned int counter = 0;

		std::map<svm_precision,std::vector<int>>::reverse_iterator cl1Itr = class1Sorted.rbegin();
		std::map<svm_precision,std::vector<int>>::iterator cl2RevItr = class2Sorted.begin();
		while(cl1Itr != class1Sorted.rend()){
			for(unsigned int i=0; i<cl1Itr->second.size(); i++){
				if(m_params[id].m_evaluation->getTestingInstance(cl1Itr->second[i])->classValue() == m_document->m_cl1Value)
					correctClass++;
				counter++;
			}
			
			cl1Itr++;

			if(cl1Itr == class1Sorted.rend() && counter < topTenPercentCount){
				while(cl2RevItr != class2Sorted.end()){
					for(unsigned int i=0; i<cl2RevItr->second.size(); i++){
						if(m_params[id].m_evaluation->getTestingInstance(cl2RevItr->second[i])->classValue() == m_document->m_cl1Value)
							correctClass++;
						counter++;
					}

					cl2RevItr++;

					if(counter >= topTenPercentCount)
						break;
				}
			}

			if(counter >= topTenPercentCount)
				break;
		}
		m_params[id].m_resultPack.cl1EnrichmentFactor = double(double(correctClass)/double(topTenPercentCount))/(double(class1)/double(results.size()));

		counter = 0;
		correctClass = 0;
		std::map<svm_precision,std::vector<int>>::reverse_iterator cl2Itr = class2Sorted.rbegin();
		std::map<svm_precision,std::vector<int>>::iterator cl1RevItr = class1Sorted.begin();
		while(cl2Itr != class2Sorted.rend()){
			for(unsigned int i=0; i<cl2Itr->second.size(); i++){
				if(m_params[id].m_evaluation->getTestingInstance(cl2Itr->second[i])->classValue() == m_document->m_cl2Value)
					correctClass++;
				counter++;
			}
			
			cl2Itr++;

			if(cl2Itr == class2Sorted.rend() && counter < topTenPercentCount){
				while(cl1RevItr != class1Sorted.end()){
					for(unsigned int i=0; i<cl1RevItr->second.size(); i++){
						if(m_params[id].m_evaluation->getTestingInstance(cl1RevItr->second[i])->classValue() == m_document->m_cl2Value)
							correctClass++;
						counter++;
					}

					cl1RevItr++;

					if(counter >= topTenPercentCount)
						break;
				}
			}

			if(counter >= topTenPercentCount)
				break;
		}
		m_params[id].m_resultPack.cl2EnrichmentFactor = double(double(correctClass)/double(topTenPercentCount))/(double(class2)/double(results.size()));

		// Balanced error rate
		m_params[id].m_resultPack.balancedErrorRate = 0.5*((double(cl2Wrong)/double(cl1Correct+(cl2Wrong))) + (double(cl1Wrong)/double((cl1Wrong)+cl2Correct)));

		// Accuracy
		m_params[id].m_resultPack.accuracy = double(cl1Correct+cl2Correct)/double(class1+class2);
	}

	void ISVM::execute(){
		unsigned int timerId = ConfigManager::startTimer();
		m_params.clear();
		m_params.assign(1,AlgoParams());

		if(!m_multiThreaded){
			std::string eval = m_data->m_gui->getEditText(IDC_COMBO_EVALUATION);
			if(eval.compare("CrossValidation") == 0){
				unsigned int folds = 0;
				try{
					folds = boost::lexical_cast<unsigned int>(m_data->m_gui->getEditText(IDC_EDIT_EVALPARAM));
				}catch(...){
					folds = 10;
				}
				m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(folds));
			}
			else if(eval.compare("PercentageSplit") == 0){
				svm_precision percent = 0;
				try{
					percent = boost::lexical_cast<svm_precision>(m_data->m_gui->getEditText(IDC_EDIT_EVALPARAM));
				}catch(...){
					percent = 66;
				}
				m_params[0].m_evaluation = IEvaluationPtr(new PercentageSplit(percent));
			}
			else{
				m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(10));
			}
			m_params[0].m_evaluation->setData(m_document,m_data);
			for(unsigned int i=0; i<m_params[0].m_evaluation->getNumStages(); i++){
				m_params[0].m_evaluation->setStage(i);
				executeStage(0);
				if(m_stop)
					break;

				m_params[0].m_resultPack.supportVectors = m_params[0].m_supportVectors->numElements();

				m_params[0].m_resultPack.cacheHits = m_params[0].m_kernel->getCacheHits();
				m_params[0].m_resultPack.kernelEvals = m_params[0].m_kernel->getKernelEvals();
				m_params[0].m_kernel->resetCounters();

				if(m_params[0].m_evaluation->isFinalStage())
					m_params[0].m_finalResultPack.totalTime = ConfigManager::getTime(timerId);

				resultOutput(m_params[0].m_evaluation->isFinalStage(),0);
			}
		}
		else{
			// TODO: Implement multithreading
		}
		ConfigManager::removeTimer(timerId);
	}

	void ISVM::resultOutput(bool final, unsigned int id){
		if(final){
			m_params[id].m_finalResultPack.accuracy += m_params[id].m_resultPack.accuracy;
			m_params[id].m_finalResultPack.balancedErrorRate += m_params[id].m_resultPack.balancedErrorRate;
			m_params[id].m_finalResultPack.cl1Correct += m_params[id].m_resultPack.cl1Correct;
			m_params[id].m_finalResultPack.cl2Correct += m_params[id].m_resultPack.cl2Correct;
			m_params[id].m_finalResultPack.cl1Wrong += m_params[id].m_resultPack.cl1Wrong;
			m_params[id].m_finalResultPack.cl2Wrong += m_params[id].m_resultPack.cl2Wrong;
			m_params[id].m_finalResultPack.cl1EnrichmentFactor += m_params[id].m_resultPack.cl1EnrichmentFactor;
			m_params[id].m_finalResultPack.cl2EnrichmentFactor += m_params[id].m_resultPack.cl2EnrichmentFactor;
			m_params[id].m_finalResultPack.cacheHits += m_params[id].m_resultPack.cacheHits;
			m_params[id].m_finalResultPack.iterations += m_params[id].m_resultPack.iterations;
			m_params[id].m_finalResultPack.supportVectors += m_params[id].m_resultPack.supportVectors;
			m_params[id].m_finalResultPack.testingTime += m_params[id].m_resultPack.testingTime;
			m_params[id].m_finalResultPack.trainingTime += m_params[id].m_resultPack.trainingTime;

			m_params[id].m_finalResultPack.cacheHits = m_params[id].m_resultPack.cacheHits;
			m_params[id].m_finalResultPack.kernelEvals = m_params[id].m_resultPack.kernelEvals;
			m_params[id].m_finalResultPack.divisor++;

			m_params[id].m_resultPack = m_params[id].m_finalResultPack;
			m_params[id].m_resultPack.accuracy /= m_params[id].m_resultPack.divisor;
			m_params[id].m_resultPack.balancedErrorRate /= m_params[id].m_resultPack.divisor;
			m_params[id].m_resultPack.cl1EnrichmentFactor /= m_params[id].m_resultPack.divisor;
			m_params[id].m_resultPack.cl2EnrichmentFactor /= m_params[id].m_resultPack.divisor;

			m_outputResults = m_params[id].m_resultPack;
			m_outputResults.trainingInstances = m_params[id].m_evaluation->getNumTrainingInstances();
			m_outputResults.testingInstances = m_params[id].m_evaluation->getNumTestingInstances();
		}

		m_outputStream	<< "\r\n\r\n" << m_document->m_cl1Value << "	" << m_document->m_cl2Value 
						<< "\r\n" << m_params[id].m_resultPack.cl1Correct << "	" << m_params[id].m_resultPack.cl2Wrong << "	" << m_document->m_cl1Value 
						<< "\r\n" << m_params[id].m_resultPack.cl1Wrong << "	" << m_params[id].m_resultPack.cl2Correct << "	" << m_document->m_cl2Value << "\r\n";
		m_outputStream << "Accuracy: " << (m_params[id].m_resultPack.accuracy)*100 << "%\r\n";
		m_outputStream << "BER: " << m_params[id].m_resultPack.balancedErrorRate << "\r\n";
		m_outputStream << "Enrichment Factor Cl1: " << m_params[id].m_resultPack.cl1EnrichmentFactor << "\r\n";
		m_outputStream << "Enrichment Factor Cl2: " << m_params[id].m_resultPack.cl2EnrichmentFactor << "\r\n\r\n";

		m_outputStream << "Testing amount: " << m_params[id].m_resultPack.cl1Correct+m_params[id].m_resultPack.cl2Correct+m_params[id].m_resultPack.cl1Wrong+m_params[id].m_resultPack.cl2Wrong << "\r\n";
		m_outputStream << "Training amount: " << m_params[id].m_evaluation->getNumTrainingInstances() << "\r\n";
		m_outputStream << "Iteration count: " << m_params[id].m_resultPack.iterations << "\r\n";
		m_outputStream << "Support vectors: " << m_params[id].m_resultPack.supportVectors << "\r\n";
		m_outputStream << "Cachehits: " << m_params[id].m_resultPack.cacheHits << " (" << (double(m_params[id].m_resultPack.cacheHits)/double((m_params[id].m_resultPack.cacheHits)+(m_params[id].m_resultPack.kernelEvals+1)))*100 << "%)\r\n";
		m_outputStream << "Kernel evals: " << m_params[id].m_resultPack.kernelEvals;
		m_outputStream << "\r\n";
		
		if(!final){
			m_params[id].m_finalResultPack.accuracy += m_params[id].m_resultPack.accuracy;
			m_params[id].m_finalResultPack.balancedErrorRate += m_params[id].m_resultPack.balancedErrorRate;
			m_params[id].m_finalResultPack.cl1Correct += m_params[id].m_resultPack.cl1Correct;
			m_params[id].m_finalResultPack.cl2Correct += m_params[id].m_resultPack.cl2Correct;
			m_params[id].m_finalResultPack.cl1Wrong += m_params[id].m_resultPack.cl1Wrong;
			m_params[id].m_finalResultPack.cl2Wrong += m_params[id].m_resultPack.cl2Wrong;
			m_params[id].m_finalResultPack.cl1EnrichmentFactor += m_params[id].m_resultPack.cl1EnrichmentFactor;
			m_params[id].m_finalResultPack.cl2EnrichmentFactor += m_params[id].m_resultPack.cl2EnrichmentFactor;
			m_params[id].m_finalResultPack.cacheHits += m_params[id].m_resultPack.cacheHits;
			m_params[id].m_finalResultPack.iterations += m_params[id].m_resultPack.iterations;
			m_params[id].m_finalResultPack.supportVectors += m_params[id].m_resultPack.supportVectors;
			m_params[id].m_finalResultPack.testingTime += m_params[id].m_resultPack.testingTime;
			m_params[id].m_finalResultPack.trainingTime += m_params[id].m_resultPack.trainingTime;

			m_params[id].m_finalResultPack.cacheHits = m_params[id].m_resultPack.cacheHits;
			m_params[id].m_finalResultPack.kernelEvals = m_params[id].m_resultPack.kernelEvals;
			m_params[id].m_finalResultPack.divisor++;
		}
		else{
			m_outputStream << "\r\n\r\nTotal time: " << m_params[id].m_finalResultPack.totalTime;
			m_outputStream << "\r\nTotal training time: " << m_params[id].m_finalResultPack.trainingTime;
			m_outputStream << "\r\nTotal testing time: " << m_params[id].m_finalResultPack.testingTime;
			m_outputStream << "\r\n";
		}

		m_params[id].m_resultPack = ResultsPack();
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());
	}

	int ISVM::examineExample(int i2, int id){
		svm_precision y2, F2;
		int i1 = -1;
    
		y2 = m_params[id].m_class[i2];
		if (m_params[id].m_I0->contains(i2)) {
			F2 = m_params[id].m_errors[i2];
		} 
		else{
			F2 = SVMOutput(i2, m_params[id].m_evaluation->getTrainingInstance(i2),id) + m_params[id].m_b - y2;
			updateErrorCache(F2,i2,id);
		    
			// Update thresholds
			if ((m_params[id].m_I1->contains(i2) || m_params[id].m_I2->contains(i2)) && (F2 < m_params[id].m_bUp)) {
				m_params[id].m_bUp = F2; 
				m_params[id].m_iUp = i2;
			} 
			else if ((m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) && (F2 > m_params[id].m_bLow)) {
				m_params[id].m_bLow = F2; 
				m_params[id].m_iLow = i2;
			}
		}

		// Check optimality using current bLow and bUp and, if
		// violated, find an index i1 to do joint optimization
		// with i2...
		boolean optimal = true;
		if (m_params[id].m_I0->contains(i2) || m_params[id].m_I1->contains(i2) || m_params[id].m_I2->contains(i2)) {
			if (m_params[id].m_bLow - F2 > 2 * m_tol) {
				optimal = false; 
				i1 = m_params[id].m_iLow;
			}
		}
		if (m_params[id].m_I0->contains(i2) || m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) {
			if (F2 - m_params[id].m_bUp > 2 * m_tol) {
				optimal = false; 
				i1 = m_params[id].m_iUp;
			}
		}
		if(optimal){
			return 0;
		}

		// For i2 unbound choose the better i1...
		if (m_params[id].m_I0->contains(i2)) {
			if (m_params[id].m_bLow - F2 > F2 - m_params[id].m_bUp) {
				i1 = m_params[id].m_iLow;
			} 
			else{
				i1 = m_params[id].m_iUp;
			}
		}
		
		if (i1 == -1) {
			// Should never happen
			assert(0);
		}
		return takeStep(i1, i2, F2,id);
	}

	int ISVM::takeStep(int i1, int i2, svm_precision F2, int id){
		svm_precision	alph1, alph2, y1, y2, F1, s, L, H, k11, k12, k22, eta,
				a1, a2, f1, f2, v1, v2, Lobj, Hobj;
		svm_precision C1 = m_C * (m_params[id].m_evaluation->getTrainingInstance(i1)->classValue() == m_document->m_cl1Value ? m_params[id].m_evaluation->getCl1C():m_params[id].m_evaluation->getCl2C());
		svm_precision C2 = m_C * (m_params[id].m_evaluation->getTrainingInstance(i2)->classValue() == m_document->m_cl1Value ? m_params[id].m_evaluation->getCl1C():m_params[id].m_evaluation->getCl2C());

		// Don't do anything if the two instances are the same
		if(i1 == i2){
			return 0;
		}

		// Initialize variables
		alph1 = m_params[id].m_alpha[i1];
		alph2 = m_params[id].m_alpha[i2];
		
		y1 = m_params[id].m_class[i1]; 
		y2 = m_params[id].m_class[i2];
		
		F1 = m_params[id].m_errors[i1];
		s = y1 * y2;

		// Find the constraints on a2
		if (y1 != y2) {
			L = max(0, alph2 - alph1); 
			H = min(C2, C1 + alph2 - alph1);
		} 
		else{
			L = max(0, alph1 + alph2 - C1);
			H = min(C2, alph1 + alph2);
		}
		
		if (L >= H){
			return 0;
		}

		std::vector<int> inds;
		inds.push_back(i1);
		inds.push_back(i2);
		std::vector<svm_precision> results;
		kernelEvaluations(inds,results,id);

		// Compute second derivative of objective function
		k11 = m_params[id].m_kernel->eval(i1, i1, m_params[id].m_evaluation->getTrainingInstance(i1));
		k12 = results[0];
		k22 = m_params[id].m_kernel->eval(i2, i2, m_params[id].m_evaluation->getTrainingInstance(i2));
		eta = 2 * k12 - k11 - k22;

		// Check if second derivative is negative
		if(eta < 0){
			// Compute unconstrained maximum
			a2 = alph2 - y2 * (F1 - F2) / eta;

			// Compute constrained maximum
			if(a2 < L){
				a2 = L;
			} 
			else if(a2 > H){
				a2 = H;
			}
		} 
		else{
			// Look at endpoints of diagonal
			f1 = SVMOutput(i1, m_params[id].m_evaluation->getTrainingInstance(i1),id);
			f2 = SVMOutput(i2, m_params[id].m_evaluation->getTrainingInstance(i2),id);
			v1 = f1 + m_params[id].m_b - y1 * alph1 * k11 - y2 * alph2 * k12; 
			v2 = f2 + m_params[id].m_b - y1 * alph1 * k12 - y2 * alph2 * k22; 
			svm_precision gamma = alph1 + s * alph2;
			Lobj = (gamma - s * L) + L - 0.5 * k11 * (gamma - s * L) * (gamma - s * L) - 
					0.5 * k22 * L * L - s * k12 * (gamma - s * L) * L - 
					y1 * (gamma - s * L) * v1 - y2 * L * v2;
			Hobj = (gamma - s * H) + H - 0.5 * k11 * (gamma - s * H) * (gamma - s * H) - 
					0.5 * k22 * H * H - s * k12 * (gamma - s * H) * H - 
					y1 * (gamma - s * H) * v1 - y2 * H * v2;
			if(Lobj > Hobj + m_eps){
				a2 = L;
			} 
			else if (Lobj < Hobj - m_eps) {
				a2 = H;
			} 
			else{
				a2 = alph2;
			}
		}
		if (abs(a2 - alph2) < (m_eps * (a2 + alph2 + m_eps))) {
			return 0;
		}
		    
		// To prevent precision problems
		if (a2 > (C2 - m_Del * C2)){
			a2 = C2;
		} 
		else if (a2 <= (m_Del * C2)){
			a2 = 0;
		}
		    
		// Recompute a1
		a1 = alph1 + s * (alph2 - a2);
		    
		// To prevent precision problems
		if (a1 > C1 - m_Del * C1) {
			a1 = C1;
		} 
		else if (a1 <= m_Del * C1) {
			a1 = 0;
		}
		    
		// Update sets
		if (a1 > 0)
			m_params[id].m_supportVectors->insert(i1);
		else
			m_params[id].m_supportVectors->remove(i1);
		if ((a1 > 0) && (a1 < C1))
			m_params[id].m_I0->insert(i1);
		else
			m_params[id].m_I0->remove(i1);
		if ((y1 == 1) && (a1 == 0))
			m_params[id].m_I1->insert(i1);
		else
			m_params[id].m_I1->remove(i1);
		if ((y1 == -1) && (a1 == C1))
			m_params[id].m_I2->insert(i1);
		else
			m_params[id].m_I2->remove(i1);
		if ((y1 == 1) && (a1 == C1))
			m_params[id].m_I3->insert(i1);
		else
			m_params[id].m_I3->remove(i1);
		if ((y1 == -1) && (a1 == 0))
			m_params[id].m_I4->insert(i1);
		else
			m_params[id].m_I4->remove(i1);

		if (a2 > 0)
			m_params[id].m_supportVectors->insert(i2);
		else
			m_params[id].m_supportVectors->remove(i2);
		if ((a2 > 0) && (a2 < C2))
			m_params[id].m_I0->insert(i2);
		else
			m_params[id].m_I0->remove(i2);
		if ((y2 == 1) && (a2 == 0))
			m_params[id].m_I1->insert(i2);
		else
			m_params[id].m_I1->remove(i2);
		if ((y2 == -1) && (a2 == C2))
			m_params[id].m_I2->insert(i2);
		else
			m_params[id].m_I2->remove(i2);
		if ((y2 == 1) && (a2 == C2))
			m_params[id].m_I3->insert(i2);
		else
			m_params[id].m_I3->remove(i2);
		if ((y2 == -1) && (a2 == 0))
			m_params[id].m_I4->insert(i2);
		else
			m_params[id].m_I4->remove(i2);
		
		svm_precision preComp1 = y1 * (a1 - alph1);
		svm_precision preComp2 = y2 * (a2 - alph2);

		// Update array with Lagrange multipliers
		m_params[id].m_alpha[i1] = a1;
		m_params[id].m_alpha[i2] = a2;
		//m_alphaTransferNeeded = true;

		lagrangeThresholdUpdate(preComp1, preComp2, id, i1, i2);

		if (!m_params[id].m_I0->contains(i1)) {
			if (m_params[id].m_I3->contains(i1) || m_params[id].m_I4->contains(i1)) {
				if (m_params[id].m_errors[i1] > m_params[id].m_bLow) {
					m_params[id].m_bLow = m_params[id].m_errors[i1]; 
					m_params[id].m_iLow = i1;
				} 
			} 
			else {
				if (m_params[id].m_errors[i1] < m_params[id].m_bUp) {
					m_params[id].m_bUp = m_params[id].m_errors[i1]; 
					m_params[id].m_iUp = i1;
				}
			}
		}
		if (!m_params[id].m_I0->contains(i2)) {
			if (m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) {
				if (m_params[id].m_errors[i2] > m_params[id].m_bLow) {
					m_params[id].m_bLow = m_params[id].m_errors[i2]; 
					m_params[id].m_iLow = i2;
				}
			} 
			else {
				if (m_params[id].m_errors[i2] < m_params[id].m_bUp) {
					m_params[id].m_bUp = m_params[id].m_errors[i2]; 
					m_params[id].m_iUp = i2;
				}
			}
		}
		if ((m_params[id].m_iLow == -1) || (m_params[id].m_iUp == -1)) {
			// Should never happen
			assert(0);
		}

		// Made some progress.
		return 1;
	}

	void ISVM::initialize(unsigned int id){
		m_params[id].m_bUp = -1; m_params[id].m_bLow = 1; m_params[id].m_b = 0;
		
		unsigned int numInstances = m_params[id].m_evaluation->getNumTrainingInstances();

		m_params[id].m_I0 = SMOSetPtr(new SMOset(numInstances));
		m_params[id].m_I1 = SMOSetPtr(new SMOset(numInstances));
		m_params[id].m_I2 = SMOSetPtr(new SMOset(numInstances));
		m_params[id].m_I3 = SMOSetPtr(new SMOset(numInstances));
		m_params[id].m_I4 = SMOSetPtr(new SMOset(numInstances));
		m_params[id].m_supportVectors = SMOSetPtr(new SMOset(numInstances));

		m_params[id].m_errors.clear();
		m_params[id].m_errors.assign(numInstances,0);

		m_params[id].m_class.clear();
		m_params[id].m_class.assign(numInstances,0);
		m_params[id].m_alpha.clear();
		m_params[id].m_alpha.assign(numInstances,0);

		// Set class values
		m_params[id].m_iUp = -1; m_params[id].m_iLow = -1;
		for (unsigned int i = 0; i < m_params[id].m_class.size(); i++) {
			if(m_params[id].m_evaluation->getTrainingInstance(i)->classValue() == m_document->m_cl1Value) {
				m_params[id].m_class[i] = -1; m_params[id].m_iLow = i;
			} 
			else if (m_params[id].m_evaluation->getTrainingInstance(i)->classValue() == m_document->m_cl2Value) {
				m_params[id].m_class[i] = 1; m_params[id].m_iUp = i;
			} 
			else{
				// Should never happen
				assert(0);
			}
		}
		m_params[id].m_errors[m_params[id].m_iLow] = 1; m_params[id].m_errors[m_params[id].m_iUp] = -1;

		for (unsigned int i = 0; i < m_params[id].m_class.size(); i++ ) {
			if (m_params[id].m_class[i] == 1) {
				m_params[id].m_I1->insert(i);
			} 
			else {
				m_params[id].m_I4->insert(i);
			}
		}

		bool cache = false;
		bool cacheFull = false;
		if(m_data->m_gui->getEditText(IDC_COMBO_KERNELCACHE).compare("true") == 0){
			cache = true;
		}
		if(m_data->m_gui->getEditText(IDC_COMBO_KERNELCACHEFULL).compare("true") == 0){
			cacheFull = true;
		}

		ConfigManager::initialize();
		std::string kernelName = m_data->m_gui->getEditText(IDC_COMBO_KERNEL);
		if(kernelName.compare("Puk") == 0)
			m_params[id].m_kernel = IKernelPtr(new PuKKernel(cache,cacheFull));
		else if(kernelName.compare("RBF") == 0)
			m_params[id].m_kernel = IKernelPtr(new RBFKernel(cache,cacheFull));
		else
			m_params[id].m_kernel = IKernelPtr(new PuKKernel(cache,cacheFull));
		m_params[id].m_kernel->initKernel(m_params[id].m_evaluation);
		
		std::stringstream ss;
		svm_precision p1,p2;
		ss << m_data->m_gui->getEditText(IDC_EDIT_PARAM2);
		ss >> p1;
		if(ss.fail())
			p1 = 1.0;
		ss = std::stringstream();
		ss << m_data->m_gui->getEditText(IDC_EDIT_PARAM3);
		ss >> p2;
		if(ss.fail())
			p2 = 1.0;

		m_params[id].m_kernel->setParameters(p1,p2);
	}

	void ISVM::executeStage(unsigned int id){
		m_params[id].m_evaluation->advance();

		initialize(id);
		initializeFold(id);
		
		unsigned int foldTimer = ConfigManager::startTimer();
		m_outputStream << "-------------------------------------------------------\r\nTraining stage " << m_params[id].m_evaluation->getStage() << ": ";
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());
		// Loop to find all the support vectors
		int numChanged = 0;
		boolean examineAll = true;
		while(((numChanged > 0) || examineAll) && !m_stop) {
			numChanged = 0;
			if (examineAll) {
				m_data->m_gui->setProgressBar(IDC_PROGRESSBAR_PROGRESS2,m_params[id].m_alpha.size(),0);
				for (unsigned int i = 0; i < m_params[id].m_alpha.size(); i++) {
					if (examineExample(i,id)) {
						numChanged++;
					}
				}
				m_data->m_gui->setProgressBar(IDC_PROGRESSBAR_PROGRESS,m_params[id].m_alpha.size(),m_params[id].m_alpha.size()-numChanged);
			}
			else {
				// This code implements Modification 1 from Keerthi et al.'s paper
				for (unsigned int i = 0; i < m_params[id].m_alpha.size(); i++) {
					if ((m_params[id].m_alpha[i] > 0) && (m_params[id].m_alpha[i] < m_C * m_params[id].m_evaluation->getTrainingInstance(i)->weight())) {
						if (examineExample(i,id)) {
							numChanged++;
						}
	      
						// Is optimality on unbound vectors obtained?
						if (m_params[id].m_bUp > m_params[id].m_bLow - 2 * m_tol) {
							numChanged = 0;
							break;
						}
					}
				}
				m_data->m_gui->setProgressBar(IDC_PROGRESSBAR_PROGRESS2,m_params[id].m_alpha.size(),m_params[id].m_alpha.size()-numChanged);
			}

			m_params[id].m_resultPack.iterations++;
			std::wstringstream s;
			s << "Iteration: " << m_params[id].m_resultPack.iterations;
			m_data->m_gui->setText(IDC_STATIC_DEBUG,s.str());
	
			if (examineAll) {
				examineAll = false;
			} 
			else if (numChanged == 0) {
				examineAll = true;
			}

			boost::xtime xt;
			boost::xtime_get(&xt, boost::TIME_UTC);
			boost::thread::sleep(xt);
		}
		double trainingTime = ConfigManager::getTime(foldTimer);
		m_outputStream << trainingTime << " -- ";
		m_params[id].m_resultPack.trainingTime = trainingTime;
		ConfigManager::resetTimer(foldTimer);

		// Set threshold
		if(!m_stop){
			m_params[id].m_b = (m_params[id].m_bLow + m_params[id].m_bUp) / 2.0;
			m_outputStream << "Testing stage " << m_params[id].m_evaluation->getStage() << ": ";
			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());

			finalResult.clear();
			testInstances(finalResult,id);

			double testTime = ConfigManager::getTime(foldTimer);
			m_params[id].m_resultPack.testingTime = testTime;
			m_outputStream << testTime << "\r\n";
			ConfigManager::removeTimer(foldTimer);
			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());

			meassurements(finalResult,id);
		}
		endFold(id);
	}
}