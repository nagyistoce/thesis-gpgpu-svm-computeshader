#include "stdafx.h"
#include "CrossValidation.h"
#include "GUIManager.h"

namespace SVM_Framework{
	CrossValidation::CrossValidation(unsigned int folds){
		m_numStages = folds;
		m_stage = 0;
	}

	bool CrossValidation::advance(){
		if((m_stage+1) > m_numStages)
			return false;
		m_stage++;

		m_testingInds.clear();
		m_trainingInds.clear();

		if(m_numStages == m_stage){
			int foldStart = (m_cl1Instances.size()/m_numStages)*(m_stage-1);
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			}
			foldStart = (m_cl2Instances.size()/m_numStages)*(m_stage-1);
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			}
		}
		else{
			int foldStart = (m_cl1Instances.size()/m_numStages)*(m_stage-1);
			int foldEnd = (m_cl1Instances.size()/m_numStages)+foldStart;
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			}
			foldStart = (m_cl2Instances.size()/m_numStages)*(m_stage-1);
			foldEnd = (m_cl2Instances.size()/m_numStages)+foldStart;
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			}
		}

		std::wstringstream stream;
		stream << "Training instance: " << m_trainingInds.size() << " Testing instances: " << m_testingInds.size() << " for fold " << m_stage << "\r\n";
		m_dataPack->m_gui->postDebugMessage(stream.str());

		return true;
	}

	void CrossValidation::init(){
		for(unsigned int i=0; i<m_data->getNumInstances(); i++){
			if(m_data->getInstance(i)->classValue() == m_data->m_cl1Value)
				m_cl1Instances.push_back(m_data->getInstance(i));
			else
				m_cl2Instances.push_back(m_data->getInstance(i));
		}
	}
}