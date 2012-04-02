#include "stdafx.h"
#include "PerformanceSearch.h"

namespace SVM_Framework{
	PerformanceSearch::PerformanceSearch(MainFrameworkPtr framework, Value::v_precision stepSize):SearchScheme(framework){
		m_stepSize = stepSize;
		m_step = m_stepSize;
		m_measureCount = 3;
		m_currentStage = 0;

		try{
			m_currentStage = boost::lexical_cast<int>(m_framework->getGuiPtr()->getEditText(IDC_EDIT_PERFSTART));
		}
		catch(...){
			m_currentStage = 0;
		}

		if(m_currentStage*m_step > 100 || m_currentStage < 0)
			m_currentStage = 0;
	}

	bool PerformanceSearch::callbackProcessing(IAlgorithmPtr algorithm){
		bool print = false;

		m_output << algorithm->getoutputStream().str();
		m_output << "\n------------------------------------------------------------------------\n";
		
		if(m_currentStage < (m_measureCount-1))
			m_currentStage++;
		else{
			m_step += m_stepSize;
			m_currentStage = 0;
			print = true;
		}

		if(m_step >= 100)
			return false;
		return true;
	}

	std::string PerformanceSearch::printProcess(){
		if(m_currentStage == 0){
			resultCompilation compile;
			std::wstringstream	total,
								training,
								testing;

			total << "	(";
			training << "	(";
			testing << "	(";

			std::wstring plus;
			for(unsigned int i=0; i<m_resPacks.size(); i++){
				compile.m_total += m_resPacks[i].totalTime;
				compile.m_training += m_resPacks[i].trainingTime;
				compile.m_testing += m_resPacks[i].testingTime;
				compile.m_acc += m_resPacks[i].accuracy;
				compile.m_cl1Enrich += m_resPacks[i].cl1EnrichmentFactor;
				compile.m_cl2Enrich += m_resPacks[i].cl2EnrichmentFactor;
				compile.m_supportVectors += m_resPacks[i].supportVectors;

				if(i < m_resPacks.size()-1)
					plus = L"+";
				else
					plus = L"";

				total << m_resPacks[i].totalTime << plus;
				training << m_resPacks[i].trainingTime << plus;
				testing << m_resPacks[i].testingTime << plus;
			}
			total << ") / " << m_measureCount;
			training << ") / " << m_measureCount;
			testing << ") / " << m_measureCount;

			
			compile.m_step = m_step-m_stepSize;
			compile.m_total /= double(m_measureCount);
			compile.m_training /= double(m_measureCount);
			compile.m_testing /= double(m_measureCount);
			compile.m_acc /= double(m_measureCount);
			compile.m_cl1Enrich /= double(m_measureCount);
			compile.m_cl2Enrich /= double(m_measureCount);
			compile.m_supportVectors /= m_measureCount;
			m_finalCompilation.push_back(compile);

			m_output << "Step: " << m_step-m_stepSize << 
						"%\n--------------------------------------\n";
			m_output << "Total time: " << compile.m_total << total.str() <<
						"\n	Training time:" << compile.m_training << training.str() <<
						"\n	Testing time: " << compile.m_testing << testing.str() << "\n";
			m_output << "Total instances: " << m_resPacks[0].testingInstances + m_resPacks[0].trainingInstances << 
						"\n	Training instances: " << m_resPacks[0].trainingInstances << 
						"\n	Testing instances: " << m_resPacks[0].testingInstances << "\n";

			if(m_step >= 100){
				m_output << "\n\n\nFinal result compilation:\n---------------------------------------\n";
				for(unsigned int i=0; i<m_finalCompilation.size(); i++){
					m_output << "Step: " << m_finalCompilation[i].m_step << "%\n" <<
								"	Total: " << m_finalCompilation[i].m_total << "s\n" <<
								"	Training: " << m_finalCompilation[i].m_training << "s\n" <<
								"	Testing: " << m_finalCompilation[i].m_testing << "s\n";
				}
				for(unsigned int i=0; i<m_finalCompilation.size(); i++){
					m_output << "\n" << m_finalCompilation[i].m_step << "%	" << m_finalCompilation[i].m_total << "	" << 
								m_finalCompilation[i].m_training << "	" << m_finalCompilation[i].m_testing;
				}
				m_output << "\n\n";
				for(unsigned int i=0; i<m_finalCompilation.size(); i++){
					m_output << "\n" << m_finalCompilation[i].m_acc*100 << "%	" << m_finalCompilation[i].m_cl1Enrich << "	" << 
						m_finalCompilation[i].m_cl2Enrich << "	" << m_finalCompilation[i].m_supportVectors;
				}
			}

			m_resPacks.clear();
		}

		return "perfSearch_";
	}
	
	void PerformanceSearch::runProcess(){
		m_framework->getGuiPtr()->disableAllButStop();

		std::wstringstream sstream;
		sstream << m_step;
		m_framework->getGuiPtr()->setText(IDC_EDIT_EVALPARAM,sstream.str());
	}
}