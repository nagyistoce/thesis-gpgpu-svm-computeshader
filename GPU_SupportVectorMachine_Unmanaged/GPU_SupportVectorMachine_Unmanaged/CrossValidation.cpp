#include "stdafx.h"
#include "CrossValidation.h"
#include "GUIManager.h"

namespace SVM_Framework{
	CrossValidation::CrossValidation(unsigned int folds){
		m_folds = folds;
		m_currentFold = 0;
	}

	InstancePtr CrossValidation::getTrainingInstance(unsigned int index){
		return m_data->getInstance(m_trainingInds[index]);
	}
	
	InstancePtr CrossValidation::getTestingInstance(unsigned int index){
		return m_data->getInstance(m_testingInds[index]);
	}

	unsigned int CrossValidation::getNumTrainingInstances(){
		return m_trainingInds.size();
	}

	unsigned int CrossValidation::getNumTestingInstances(){
		return m_testingInds.size();
	}

	bool CrossValidation::advance(){
		if((m_currentFold+1) > m_folds)
			return false;
		m_currentFold++;

		m_testingInds.clear();
		m_trainingInds.clear();

		if(m_folds == m_currentFold){
			int foldStart = (m_cl1Instances.size()/m_folds)*(m_currentFold-1);
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			}
			foldStart = (m_cl2Instances.size()/m_folds)*(m_currentFold-1);
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			}
		}
		else{
			int foldStart = (m_cl1Instances.size()/m_folds)*(m_currentFold-1);
			int foldEnd = (m_cl1Instances.size()/m_folds)+foldStart;
			for(unsigned int i=0; i<m_cl1Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl1Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			}
			foldStart = (m_cl2Instances.size()/m_folds)*(m_currentFold-1);
			foldEnd = (m_cl2Instances.size()/m_folds)+foldStart;
			for(unsigned int i=0; i<m_cl2Instances.size(); i++){
				if(i >= foldStart && i < foldEnd)
					m_testingInds.push_back(m_cl2Instances[i]->getIndex());
				else
					m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			}
		}

		std::wstringstream stream;
		stream << "Training instance: " << m_trainingInds.size() << " Testing instances: " << m_testingInds.size() << " for fold " << m_currentFold << "\r\n";
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