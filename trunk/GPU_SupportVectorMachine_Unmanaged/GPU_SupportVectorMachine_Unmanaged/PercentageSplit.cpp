#include "stdafx.h"
#include "PercentageSplit.h"

namespace SVM_Framework{
	PercentageSplit::PercentageSplit(float splitPercentage):m_splitPercentage(splitPercentage){
		m_stage = 0;
		m_numStages = 1;
	}

	bool PercentageSplit::advance(){
		if(m_stage == 0){
			m_stage++;
			return true;
		}
		return false;
	}

	bool PercentageSplit::isFinalStage(){
		if(m_stage == 1)
			return true;
		return false;
	}

	void PercentageSplit::init(){
		for(unsigned int i=0; i<m_data->getNumInstances(); i++){
			if(m_data->getInstance(i)->classValue() == m_data->m_cl1Value)
				m_cl1Instances.push_back(m_data->getInstance(i));
			else
				m_cl2Instances.push_back(m_data->getInstance(i));
		}

		unsigned int numTrainClass1 = m_cl1Instances.size() * (m_splitPercentage*0.01);
		unsigned int numTrainClass2 = m_cl2Instances.size() * (m_splitPercentage*0.01);
		for(unsigned int i=0; i<m_cl1Instances.size(); i++){
			if(i < numTrainClass1)
				m_trainingInds.push_back(m_cl1Instances[i]->getIndex());
			else
				m_testingInds.push_back(m_cl1Instances[i]->getIndex());
		}
		for(unsigned int i=0; i<m_cl2Instances.size(); i++){
			if(i < numTrainClass2)
				m_trainingInds.push_back(m_cl2Instances[i]->getIndex());
			else
				m_testingInds.push_back(m_cl2Instances[i]->getIndex());
		}

		calculateCost(numTrainClass1,numTrainClass2);
	}
}