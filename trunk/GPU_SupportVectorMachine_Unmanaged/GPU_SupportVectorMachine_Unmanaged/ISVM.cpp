#include "stdafx.h"
#include "ISVM.h"
#include "PuKKernel.h"
#include "RBFKernel.h"
#include "CrossValidation.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	ISVM::ISVM(){
	}

	void ISVM::run(AlgorithmDataPackPtr data){
		m_data = data;
		m_document = m_data->m_recMgr->getDocumentResource(m_data->m_dataResource);
		
		execute();
	}
}