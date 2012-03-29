#include "stdafx.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	void IEvaluation::setData(DataDocumentPtr data, IDataPackPtr dataPack){	
		m_dataPack = dataPack; 
		m_data = data; 
		init();
	}

	void IEvaluation::calculateCost(int cl1,int cl2){
		if(cl2 < cl1){
			m_C_cl1 = double(double(cl2)/double(cl1));
			m_C_cl2 = 1.0;
		}
		else{
			m_C_cl2 = double(double(cl1)/double(cl2));
			m_C_cl1 = 1.0;
		}
		/*m_C_cl2 = 1.0;
		m_C_cl1 = 1.0;*/
	}
}