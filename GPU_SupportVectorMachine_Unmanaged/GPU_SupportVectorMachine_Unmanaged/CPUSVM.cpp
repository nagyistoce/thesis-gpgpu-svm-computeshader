#include "stdafx.h"
#include "CPUSVM.h"
#include "ConfigManager.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "GUIManager.h"
#include "CrossValidation.h"

namespace SVM_Framework{
	CPUSVM::CPUSVM(GraphicsManagerPtr dxMgr):m_eps(1.0e-12),m_tol(1.0e-3),m_C(1.0),m_KernelIsLinear(false){
		m_Del = 1000 * DBL_MIN;
	}
	
	CPUSVM::~CPUSVM(){
		
	}

	void CPUSVM::execute(){
		std::wstringstream outputStream;;
		unsigned int timerId = ConfigManager::startTimer();
		m_data->m_gui->setText(m_data->m_gui->getEditWindowId(),L"Working...");

		unsigned int	cl1Correct = 0,
						cl1Wrong = 0,
						cl2Correct = 0,
						cl2Wrong = 0;

		m_params.assign(5,AlgoParams());
		for(unsigned int i=0; i<5; i++){
			m_params[i].m_evaluation = IEvaluationPtr(new CrossValidation(10));
			m_params[i].m_evaluation->setData(m_document);
			boost::static_pointer_cast<CrossValidation>(m_params[i].m_evaluation)->setFold(i);

			m_threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(
				boost::bind(&CPUSVM::executeFold,this,i)
			)));
		}

		for(unsigned int i=0; i<5; i++){
			m_threads[i]->join();
			cl1Correct += m_params[i].cl1Correct;
			cl2Correct += m_params[i].cl2Correct;
			cl1Wrong += m_params[i].cl1Wrong;
			cl2Wrong += m_params[i].cl2Wrong;
		}
		m_threads.clear();

		for(unsigned int i=0; i<5; i++){
			m_params[i].m_evaluation = IEvaluationPtr(new CrossValidation(10));
			m_params[i].m_evaluation->setData(m_document);
			boost::static_pointer_cast<CrossValidation>(m_params[i].m_evaluation)->setFold(i+5);

			m_threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(
				boost::bind(&CPUSVM::executeFold,this,i)
			)));
		}

		for(unsigned int i=0; i<5; i++){
			m_threads[i]->join();
			cl1Correct += m_params[i].cl1Correct;
			cl2Correct += m_params[i].cl2Correct;
			cl1Wrong += m_params[i].cl1Wrong;
			cl2Wrong += m_params[i].cl2Wrong;
		}
		
		outputStream << "Total time: " << ConfigManager::getTime(timerId);

		outputStream << "\n\na	b\n" << cl1Correct << "	" << cl1Wrong << "\n" << cl2Wrong << "	" << cl2Correct << "\n";
		outputStream << "Accuracy: " << (float(cl1Correct+cl2Correct)/float(cl1Correct+cl2Correct+cl1Wrong+cl2Wrong))*100 << "%\n\n";

		m_data->m_gui->setText(m_data->m_gui->getEditWindowId(),outputStream.str());
		ConfigManager::removeTimer(timerId);

		m_threads.clear();
		m_params.clear();
	}

	void CPUSVM::initialize(unsigned int id){
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
		int cl1 = 1, cl2 = 0;
		m_params[id].m_iUp = -1; m_params[id].m_iLow = -1;
		for (unsigned int i = 0; i < m_params[id].m_class.size(); i++) {
			if(m_params[id].m_evaluation->getTrainingInstance(i)->classValue() == cl1) {
				m_params[id].m_class[i] = -1; m_params[id].m_iLow = i;
			} 
			else if (m_params[id].m_evaluation->getTrainingInstance(i)->classValue() == cl2) {
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

		ConfigManager::initialize();
		std::string kernelName = ConfigManager::getSetting("kernel");
		if(kernelName.compare("puk") == 0)
			m_params[id].m_kernel = IKernelPtr(new PuKKernel);
		else if(kernelName.compare("rbf") == 0)
			m_params[id].m_kernel = IKernelPtr(new RBFKernel);
		m_params[id].m_kernel->initKernel(m_params[id].m_evaluation);
		
		std::stringstream ss;
		double p1,p2,p3;
		ss << ConfigManager::getSetting("kernelParam1");
		ss >> p1;
		ss = std::stringstream();
		ss << ConfigManager::getSetting("kernelParam2");
		ss >> p2;
		ss = std::stringstream();
		ss << ConfigManager::getSetting("kernelParam3");
		ss >> p3;

		m_params[id].m_kernel->setParameters(p1,p2,p3);
	}
	
	int CPUSVM::examineExample(int i2, unsigned int id){
		double y2, F2;
		int i1 = -1;
    
		y2 = m_params[id].m_class[i2];
		if (m_params[id].m_I0->contains(i2)) {
			F2 = m_params[id].m_errors[i2];
		} 
		else{
			F2 = SVMOutput(i2, m_params[id].m_evaluation->getTrainingInstance(i2),id) + m_params[id].m_b - y2;
			m_params[id].m_errors[i2] = F2;
		    
			// Update thresholds
			if ((m_params[id].m_I1->contains(i2) || m_params[id].m_I2->contains(i2)) && (F2 < m_params[id].m_bUp)) {
				m_params[id].m_bUp = F2; m_params[id].m_iUp = i2;
			} 
			else if ((m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) && (F2 > m_params[id].m_bLow)) {
				m_params[id].m_bLow = F2; m_params[id].m_iLow = i2;
			}
		}

		// Check optimality using current bLow and bUp and, if
		// violated, find an index i1 to do joint optimization
		// with i2...
		boolean optimal = true;
		if (m_params[id].m_I0->contains(i2) || m_params[id].m_I1->contains(i2) || m_params[id].m_I2->contains(i2)) {
			if (m_params[id].m_bLow - F2 > 2 * m_tol) {
				optimal = false; i1 = m_params[id].m_iLow;
			}
		}
		if (m_params[id].m_I0->contains(i2) || m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) {
			if (F2 - m_params[id].m_bUp > 2 * m_tol) {
				optimal = false; i1 = m_params[id].m_iUp;
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

	int CPUSVM::takeStep(int i1, int i2, double F2, unsigned int id){
		double	alph1, alph2, y1, y2, F1, s, L, H, k11, k12, k22, eta,
				a1, a2, f1, f2, v1, v2, Lobj, Hobj;
		double C1 = m_C * m_params[id].m_evaluation->getTrainingInstance(i1)->weight();
		double C2 = m_C * m_params[id].m_evaluation->getTrainingInstance(i2)->weight();

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

		// Compute second derivative of objective function
		k11 = m_params[id].m_kernel->eval(i1, i1, m_params[id].m_evaluation->getTrainingInstance(i1));
		k12 = m_params[id].m_kernel->eval(i1, i2, m_params[id].m_evaluation->getTrainingInstance(i1));
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
			double gamma = alph1 + s * alph2;
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
		if (a1 > 0) {
			m_params[id].m_supportVectors->insert(i1);
		}
		else{
			m_params[id].m_supportVectors->remove(i1);
		}
		if ((a1 > 0) && (a1 < C1)) {
			m_params[id].m_I0->insert(i1);
		} 
		else{
			m_params[id].m_I0->remove(i1);
		}
		if ((y1 == 1) && (a1 == 0)) {
			m_params[id].m_I1->insert(i1);
		} 
		else {
			m_params[id].m_I1->remove(i1);
		}
		if ((y1 == -1) && (a1 == C1)) {
			m_params[id].m_I2->insert(i1);
		} 
		else {
			m_params[id].m_I2->remove(i1);
		}
		if ((y1 == 1) && (a1 == C1)) {
			m_params[id].m_I3->insert(i1);
		} 
		else {
			m_params[id].m_I3->remove(i1);
		}
		if ((y1 == -1) && (a1 == 0)) {
			m_params[id].m_I4->insert(i1);
		} 
		else {
			m_params[id].m_I4->remove(i1);
		}
		if (a2 > 0) {
			m_params[id].m_supportVectors->insert(i2);
		}
		else {
			m_params[id].m_supportVectors->remove(i2);
		}
		if ((a2 > 0) && (a2 < C2)) {
			m_params[id].m_I0->insert(i2);
		} 
		else {
			m_params[id].m_I0->remove(i2);
		}
		if ((y2 == 1) && (a2 == 0)) {
			m_params[id].m_I1->insert(i2);
		} 
		else {
			m_params[id].m_I1->remove(i2);
		}
		if ((y2 == -1) && (a2 == C2)) {
			m_params[id].m_I2->insert(i2);
		} 
		else {
			m_params[id].m_I2->remove(i2);
		}
		if ((y2 == 1) && (a2 == C2)) {
			m_params[id].m_I3->insert(i2);
		} 
		else {
			m_params[id].m_I3->remove(i2);
		}
		if ((y2 == -1) && (a2 == 0)) {
			m_params[id].m_I4->insert(i2);
		} 
		else {
			m_params[id].m_I4->remove(i2);
		}
		    
		// Update weight vector to reflect change a1 and a2, if linear SVM
		if (m_KernelIsLinear) {
			assert(0);
			/*InstancePtr inst1 = m_evaluation->getTrainingInstance(i1);
			for (int p1 = 0; p1 < inst1->numValues(); p1++) {
				if (inst1.index(p1) != m_document->m_classAttributeId) {
					m_weights[inst1.index(p1)] += y1 * (a1 - alph1) * inst1.valueSparse(p1);
				}
			}
			InstancePtr inst2 = m_document->getInstance(i2);
			for (int p2 = 0; p2 < inst2->numValues(); p2++) {
				if (inst2.index(p2) != m_document->m_classAttributeId) {
					m_weights[inst2.index(p2)] += y2 * (a2 - alph2) * inst2.valueSparse(p2);
				}
			}*/
		}
		    
		// Update error cache using new Lagrange multipliers
		for (int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)) {
			if ((j != i1) && (j != i2)) {
				m_params[id].m_errors[j] +=	y1 * (a1 - alph1) * m_params[id].m_kernel->eval(i1, j, m_params[id].m_evaluation->getTrainingInstance(i1)) + 
											y2 * (a2 - alph2) * m_params[id].m_kernel->eval(i2, j, m_params[id].m_evaluation->getTrainingInstance(i2));
			}
		}
		    
		// Update error cache for i1 and i2
		m_params[id].m_errors[i1] += y1 * (a1 - alph1) * k11 + y2 * (a2 - alph2) * k12;
		m_params[id].m_errors[i2] += y1 * (a1 - alph1) * k12 + y2 * (a2 - alph2) * k22;
		    
		// Update array with Lagrange multipliers
		m_params[id].m_alpha[i1] = a1;
		m_params[id].m_alpha[i2] = a2;
		    
		// Update thresholds
		m_params[id].m_bLow = -DBL_MAX; m_params[id].m_bUp = DBL_MAX;
		m_params[id].m_iLow = -1; m_params[id].m_iUp = -1;
		for (int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)) {
			if (m_params[id].m_errors[j] < m_params[id].m_bUp) {
				m_params[id].m_bUp = m_params[id].m_errors[j]; m_params[id].m_iUp = j;
			}
			if(m_params[id].m_errors[j] > m_params[id].m_bLow) {
				m_params[id].m_bLow = m_params[id].m_errors[j]; m_params[id].m_iLow = j;
			}
		}
		if (!m_params[id].m_I0->contains(i1)) {
			if (m_params[id].m_I3->contains(i1) || m_params[id].m_I4->contains(i1)) {
				if (m_params[id].m_errors[i1] > m_params[id].m_bLow) {
					m_params[id].m_bLow = m_params[id].m_errors[i1]; m_params[id].m_iLow = i1;
				} 
			} 
			else {
				if (m_params[id].m_errors[i1] < m_params[id].m_bUp) {
					m_params[id].m_bUp = m_params[id].m_errors[i1]; m_params[id].m_iUp = i1;
				}
			}
		}
		if (!m_params[id].m_I0->contains(i2)) {
			if (m_params[id].m_I3->contains(i2) || m_params[id].m_I4->contains(i2)) {
				if (m_params[id].m_errors[i2] > m_params[id].m_bLow) {
					m_params[id].m_bLow = m_params[id].m_errors[i2]; m_params[id].m_iLow = i2;
				}
			} 
			else {
				if (m_params[id].m_errors[i2] < m_params[id].m_bUp) {
					m_params[id].m_bUp = m_params[id].m_errors[i2]; m_params[id].m_iUp = i2;
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

	double CPUSVM::SVMOutput(int index, InstancePtr inst, unsigned int id){
		double result = 0;
		
		// Is the machine linear?
		if (m_KernelIsLinear) {
			assert(0);
			//// Is weight vector stored in sparse format?
			//if (m_sparseWeights == null) {
			//	int n1 = inst.numValues(); 
			//	for (int p = 0; p < n1; p++) {
			//		if (inst.index(p) != m_classIndex) {
			//			result += m_weights[inst.index(p)] * inst.valueSparse(p);
			//		}
			//	}
			//} 
			//else {
			//	int n1 = inst.numValues(); int n2 = m_sparseWeights.length;
			//	for (int p1 = 0, p2 = 0; p1 < n1 && p2 < n2;) {
			//		int ind1 = inst.index(p1); 
			//		int ind2 = m_sparseIndices[p2];
			//		if (ind1 == ind2) {
			//			if (ind1 != m_classIndex) {
			//				result += inst.valueSparse(p1) * m_sparseWeights[p2];
			//			}
			//			p1++; p2++;
			//		} 
			//		else if (ind1 > ind2) {
			//			p2++;
			//		} 
			//		else { 
			//			p1++;
			//		}
			//	}
			//}
		} 
		else {
			for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
				result += m_params[id].m_class[i] * m_params[id].m_alpha[i] * m_params[id].m_kernel->eval(index, i, inst);
			}
		}
		result -= m_params[id].m_b;
		
		return result;
	}

	void CPUSVM::executeFold(unsigned int id){
		m_params[id].m_evaluation->advance();
		m_params[id].cl1Correct = 0;
		m_params[id].cl2Correct = 0;
		m_params[id].cl1Wrong = 0;
		m_params[id].cl2Wrong = 0;

		initialize(id);

		// Loop to find all the support vectors
		int numChanged = 0;
		boolean examineAll = true;
		while ((numChanged > 0) || examineAll) {
			numChanged = 0;
			if (examineAll) {
				for (unsigned int i = 0; i < m_params[id].m_alpha.size(); i++) {
					if (examineExample(i,id)) {
						numChanged++;
					}
				}
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
	  
				//This is the code for Modification 2 from Keerthi et al.'s paper
				/*BOOL innerLoopSuccess = true;
				numChanged = 0;
				while ((m_bUp < (m_bLow - 2 * m_tol)) && (innerLoopSuccess == TRUE)) {
					innerLoopSuccess = takeStep(m_iUp, m_iLow, m_errors[m_iLow]);
				}*/
			}
	
			if (examineAll) {
				examineAll = false;
			} 
			else if (numChanged == 0) {
				examineAll = true;
			}
		}

		// Set threshold
		m_params[id].m_b = (m_params[id].m_bLow + m_params[id].m_bUp) / 2.0;

		// Testing
		InstancePtr testInst;
		int classValue;
		for(unsigned int i=0; i<m_params[id].m_evaluation->getNumTestingInstances(); i++){
			testInst = m_params[id].m_evaluation->getTestingInstance(i);
			double output = SVMOutput(-1,testInst,id);
			classValue = testInst->classValue();
			if(output < 0){
				if(testInst->classValue() == 1)
					m_params[id].cl1Correct++;
				else
					m_params[id].cl1Wrong++;
			}
			else{
				if(testInst->classValue() == 0)
					m_params[id].cl2Correct++;
				else
					m_params[id].cl2Wrong++;
			}
		}
	}
}