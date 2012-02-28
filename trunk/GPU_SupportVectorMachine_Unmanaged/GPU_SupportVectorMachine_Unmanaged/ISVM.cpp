#include "stdafx.h"
#include "ISVM.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "CrossValidation.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	ISVM::ISVM():m_eps(1.0e-12),m_tol(1.0e-3),m_C(1.0),m_KernelIsLinear(false){
		m_Del = 1000 * DBL_MIN;
	}

	void ISVM::run(AlgorithmDataPackPtr data){
		m_data = data;
		m_document = m_data->m_recMgr->getDocumentResource(m_data->m_dataResource);
		m_outputStream = std::wstringstream();

		execute();
		resultOutput();
	}

	void ISVM::resultOutput(){
		m_outputStream << "\r\n\r\nTotal time: " << m_resultPack.totalTime;

		m_outputStream	<< "\r\n\r\n" << m_document->m_cl1Value << "	" << m_document->m_cl2Value 
						<< "\r\n" << m_resultPack.cl1Correct << "	" << m_resultPack.cl1Wrong << "	" << m_document->m_cl1Value 
						<< "\r\n" << m_resultPack.cl2Wrong << "	" << m_resultPack.cl2Correct << "	" << m_document->m_cl2Value << "\r\n";
		m_outputStream << "Accuracy: " << (float(m_resultPack.cl1Correct+m_resultPack.cl2Correct)/float(m_resultPack.cl1Correct+m_resultPack.cl2Correct+m_resultPack.cl1Wrong+m_resultPack.cl2Wrong))*100 << "%\r\n\r\n";
		m_outputStream << "Total number tested: " << m_resultPack.cl1Correct+m_resultPack.cl2Correct+m_resultPack.cl1Wrong+m_resultPack.cl2Wrong << "\r\n";
		m_outputStream << "Iteration count: " << m_resultPack.iterations << "\r\n";
		m_outputStream << "Support vectors: " << m_resultPack.supportVectors << "\r\n";
		m_outputStream << "Cachehits: " << m_resultPack.cacheHits << " (" << (double(m_resultPack.cacheHits)/double((m_resultPack.cacheHits)+(m_resultPack.kernelEvals+1)))*100 << "%)\r\n";
		m_outputStream <<	"Kernel evals: " << m_resultPack.kernelEvals;
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());

		m_resultPack = ResultsPack();
	}

	int ISVM::examineExample(int i2, int id){
		double y2, F2;
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

	int ISVM::takeStep(int i1, int i2, double F2, int id){
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

		std::vector<int> inds;
		inds.push_back(i1);
		inds.push_back(i2);
		std::vector<float> results;
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
		
		double preComp1 = y1 * (a1 - alph1);
		double preComp2 = y2 * (a2 - alph2);

		// Update array with Lagrange multipliers
		m_params[id].m_alpha[i1] = a1;
		m_params[id].m_alpha[i2] = a2;

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

		ConfigManager::initialize();
		std::string kernelName = m_data->m_gui->getEditText(IDC_EDIT_PARAM1);
		if(kernelName.compare("puk") == 0)
			m_params[id].m_kernel = IKernelPtr(new PuKKernel);
		else if(kernelName.compare("rbf") == 0)
			m_params[id].m_kernel = IKernelPtr(new RBFKernel);
		else
			m_params[id].m_kernel = IKernelPtr(new PuKKernel);
		m_params[id].m_kernel->initKernel(m_params[id].m_evaluation);
		
		std::stringstream ss;
		double p1,p2;
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

	void ISVM::executeFold(unsigned int id){
		m_params[id].m_evaluation->advance();
		m_params[id].cl1Correct = 0;
		m_params[id].cl2Correct = 0;
		m_params[id].cl1Wrong = 0;
		m_params[id].cl2Wrong = 0;
		m_params[id].iterations = 0;

		initialize(id);
		initializeFold(id);
		
		unsigned int foldTimer = ConfigManager::startTimer();
		m_outputStream << "Training fold " << m_params[id].m_evaluation->getStage() << ": ";
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());
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
			}

			m_params[id].iterations++;
	
			if (examineAll) {
				examineAll = false;
			} 
			else if (numChanged == 0) {
				examineAll = true;
			}
		}
		m_outputStream << ConfigManager::getTime(foldTimer) << " -- ";
		ConfigManager::resetTimer(foldTimer);

		// Set threshold
		m_params[id].m_b = (m_params[id].m_bLow + m_params[id].m_bUp) / 2.0;
		m_outputStream << "Testing fold " << m_params[id].m_evaluation->getStage() << ": ";
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());
		testInstances(id);

		m_outputStream << ConfigManager::getTime(foldTimer) << "\r\n";
		ConfigManager::removeTimer(foldTimer);
		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,m_outputStream.str());

		endFold(id);
	}
}