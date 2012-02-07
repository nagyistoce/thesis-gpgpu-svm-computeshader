#pragma once
#include "DataDocument.h"

namespace SVM_Framework{
	class IEvaluation{
	public:
		virtual InstancePtr getTrainingInstance(unsigned int index) = 0;
		virtual InstancePtr getTestingInstance(unsigned int index) = 0;

		virtual unsigned int getNumTrainingInstances() = 0;
		virtual unsigned int getNumTestingInstances() = 0;

		virtual bool advance() = 0;
		virtual void init() = 0;

		void setData(DataDocumentPtr data){ m_data = data; init(); }
	protected:
		DataDocumentPtr m_data;
		unsigned int	m_numTrainingIns,
						m_numTestingIns;
	};
}

typedef boost::shared_ptr<SVM_Framework::IEvaluation> IEvaluationPtr;