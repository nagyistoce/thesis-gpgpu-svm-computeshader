#pragma once
#include "DataDocument.h"
#include "IDataPack.h"

namespace SVM_Framework{
	class IEvaluation{
	public:
		virtual InstancePtr getTrainingInstance(unsigned int index) = 0;
		virtual InstancePtr getTestingInstance(unsigned int index) = 0;

		virtual unsigned int getNumTrainingInstances() = 0;
		virtual unsigned int getNumTestingInstances() = 0;

		virtual bool advance() = 0;
		virtual void init() = 0;

		void setData(DataDocumentPtr data, IDataPackPtr dataPack){	m_dataPack = dataPack; m_data = data; init(); }
	protected:
		IDataPackPtr m_dataPack;
		DataDocumentPtr m_data;
	};
}

typedef boost::shared_ptr<SVM_Framework::IEvaluation> IEvaluationPtr;