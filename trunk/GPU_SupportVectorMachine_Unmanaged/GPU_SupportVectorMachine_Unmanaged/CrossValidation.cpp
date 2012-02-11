#include "stdafx.h"
#include "CrossValidation.h"
#include "GUIManager.h"

namespace SVM_Framework{
	CrossValidation::CrossValidation(unsigned int folds){
		m_folds = folds;
		m_currentFold = 0;
	}

	InstancePtr CrossValidation::getTrainingInstance(unsigned int index){
		return m_trainingInstances[index];
	}
	
	InstancePtr CrossValidation::getTestingInstance(unsigned int index){
		return m_testingInstances[index];
	}

	unsigned int CrossValidation::getNumTrainingInstances(){
		return m_trainingInstances.size();
	}

	unsigned int CrossValidation::getNumTestingInstances(){
		return m_testingInstances.size();
	}

	bool CrossValidation::advance(){
		if((m_currentFold+1) > m_folds)
			return false;
		m_currentFold++;

		m_testingInstances.clear();
		m_trainingInstances.clear();

		if(m_folds == m_currentFold){
			int foldStart = (m_cl1Instances.size()/m_folds)*(m_currentFold-1);
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart)
					m_testingInstances.push_back(m_cl1Instances[i]);
				else
					m_trainingInstances.push_back(m_cl1Instances[i]);
			}
			foldStart = (m_cl2Instances.size()/m_folds)*(m_currentFold-1);
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart)
					m_testingInstances.push_back(m_cl2Instances[i]);
				else
					m_trainingInstances.push_back(m_cl2Instances[i]);
			}
		}
		else{
			int foldStart = (m_cl1Instances.size()/m_folds)*(m_currentFold-1);
			int foldEnd = (m_cl1Instances.size()/m_folds)+foldStart;
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInstances.push_back(m_cl1Instances[i]);
				else
					m_trainingInstances.push_back(m_cl1Instances[i]);
			}
			foldStart = (m_cl2Instances.size()/m_folds)*(m_currentFold-1);
			foldEnd = (m_cl2Instances.size()/m_folds)+foldStart;
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInstances.push_back(m_cl2Instances[i]);
				else
					m_trainingInstances.push_back(m_cl2Instances[i]);
			}
		}

		std::wstringstream stream;
		stream << "Training instance: " << m_trainingInstances.size() << " Testing instances: " << m_testingInstances.size() << " for fold " << m_currentFold << "\r\n";
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

	void CrossValidation::setNumFolds(unsigned int folds){
		m_folds = folds;
	}

	void CrossValidation::setFold(unsigned int fold){
		m_currentFold = fold;
	}
}