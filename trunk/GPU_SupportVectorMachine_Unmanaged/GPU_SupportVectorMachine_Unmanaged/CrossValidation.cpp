#include "stdafx.h"
#include "CrossValidation.h"

namespace SVM_Framework{
	CrossValidation::CrossValidation(unsigned int folds){
		m_folds = folds;
		m_currentFold = 0;
	}

	InstancePtr CrossValidation::getTrainingInstance(unsigned int index){
		unsigned int ind = index;

		if((index < m_foldEnd) && (index >= m_foldStart)){
			ind += m_numTestingIns;
		}

		return m_data->getInstance(ind);
	}
	
	InstancePtr CrossValidation::getTestingInstance(unsigned int index){
		if(((index+m_foldStart) > m_foldEnd)){
			return InstancePtr();
		}

		return m_data->getInstance(index+m_foldStart);
	}

	unsigned int CrossValidation::getNumTrainingInstances(){
		return m_numTrainingIns;
	}

	unsigned int CrossValidation::getNumTestingInstances(){
		return m_numTestingIns;
	}

	bool CrossValidation::advance(){
		if((m_currentFold+1) > m_folds)
			return false;
		m_currentFold++;

		if(m_folds == m_currentFold){
			int add = ((m_numTestingIns+m_numTrainingIns)-(m_numTestingIns*m_folds));
			m_foldStart = m_numTestingIns*(m_currentFold-1);
			m_foldEnd = m_numTestingIns+m_numTrainingIns-1;

			m_numTestingIns += add;
			m_numTrainingIns -= add;
		}
		else{
			m_foldStart = m_numTestingIns*(m_currentFold-1);
			m_foldEnd = m_foldStart + m_numTestingIns;
		}
		return true;
	}

	void CrossValidation::init(){
		if(m_folds < 2){
			m_numTrainingIns = m_data->getNumInstances();
			m_numTestingIns = 0;
		}
		else{
			m_numTestingIns = floor(double(m_data->getNumInstances())/double(m_folds));
			m_numTrainingIns = ceil((double(m_data->getNumInstances())/double(m_folds))*double(m_folds-1));
			assert((m_numTrainingIns+m_numTestingIns) == m_data->getNumInstances());
		}
	}

	void CrossValidation::setNumFolds(unsigned int folds){
		m_folds = folds;
	}

	void CrossValidation::setFold(unsigned int fold){
		m_currentFold = fold;
	}
}