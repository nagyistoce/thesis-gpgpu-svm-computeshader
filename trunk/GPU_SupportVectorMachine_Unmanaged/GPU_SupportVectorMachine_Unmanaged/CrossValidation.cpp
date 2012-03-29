#include "stdafx.h"
#include "CrossValidation.h"
#include "GUIManager.h"

namespace SVM_Framework{
	CrossValidation::CrossValidation(unsigned int folds){
		m_numStages = folds+1;
		m_stage = 0;
	}

	bool CrossValidation::advance(){
		if((m_stage+1) > m_numStages)
			return false;
		m_stage++;

		m_testingInds.clear();
		m_trainingInds.clear();

		int cl1 = 0, cl2 = 0;
		if(m_numStages == m_stage){
			for(unsigned int i=0; i<m_validationSet.size(); i++){
				m_testingInds.push_back(m_validationSet[i]->getIndex());
			}
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			}
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			}

			calculateCost(m_cl1Instances.size(),m_cl2Instances.size());
		}
		else if(m_numStages-1 == m_stage){
			int foldStart = float(float(m_cl1Instances.size())/float(m_numStages-1))*float(m_stage-1);
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else{
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
					cl1++;
				}
			}
			foldStart = float(float(m_cl2Instances.size())/float(m_numStages-1))*float(m_stage-1);
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else{
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
					cl2++;
				}
			}
			calculateCost(cl1,cl2);
		}
		else{
			int foldStart = float(float(m_cl1Instances.size())/float(m_numStages-1))*float(m_stage-1);
			int foldEnd = float(float(m_cl1Instances.size())/float(m_numStages-1))+float(foldStart);
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else{
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
					cl1++;
				}
			}
			foldStart = float(float(m_cl2Instances.size())/float(m_numStages-1))*float(m_stage-1);
			foldEnd = float(float(m_cl2Instances.size())/float(m_numStages-1))+float(foldStart);
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else{
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
					cl2++;
				}
			}
			calculateCost(cl1,cl2);
		}

		/*std::wstringstream stream;
		stream << "Training instance: " << m_trainingInds.size() << " Testing instances: " << m_testingInds.size() << " for fold " << m_stage << "\r\n";
		m_dataPack->m_gui->postDebugMessage(stream.str());*/

		return true;
	}

	bool CrossValidation::isFinalStage(){
		if(m_numStages-1 == m_stage)
			return true;
		return false;
	}

	void CrossValidation::init(){
		for(unsigned int i=0; i<m_data->getNumInstances(); i++){
			if(m_data->getInstance(i)->classValue() == m_data->m_cl1Value)
				m_cl1Instances.push_back(m_data->getInstance(i));
			else
				m_cl2Instances.push_back(m_data->getInstance(i));
		}

		// Validation set 30% of the data
		int num = int(float(m_cl1Instances.size())*0.3);
		for(unsigned int i=0; i<num; i++){
			m_validationSet.push_back(m_cl1Instances.back());
			m_cl1Instances.pop_back();
		}
		num = int(float(m_cl2Instances.size())*0.3);
		for(unsigned int i=0; i<num; i++){
			m_validationSet.push_back(m_cl2Instances.back());
			m_cl2Instances.pop_back();
		}
	}
}