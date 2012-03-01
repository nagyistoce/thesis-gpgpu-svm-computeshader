#pragma once
#include "DataDocument.h"
#include "IDataPack.h"

namespace SVM_Framework{
	class IEvaluation{
	public:
		InstancePtr getTrainingInstance(unsigned int index) { return m_data->getInstance(m_trainingInds[index]); }
		InstancePtr getTestingInstance(unsigned int index) { return m_data->getInstance(m_testingInds[index]); }

		unsigned int getNumTrainingInstances() { return m_trainingInds.size(); }
		unsigned int getNumTestingInstances() { return m_testingInds.size(); }

		virtual bool advance() = 0;
		virtual void init() = 0;

		void setStage(unsigned int stage) { m_stage = stage; }
		unsigned int getStage() { return m_stage; }
		unsigned int getNumStages() { return m_numStages; }

		void setData(DataDocumentPtr data, IDataPackPtr dataPack){	m_dataPack = dataPack; m_data = data; init(); }
		std::vector<unsigned int>& getTrainingInds() { return m_trainingInds; }
		std::vector<unsigned int>& getTestingInds() { return m_testingInds; }
	protected:
		IDataPackPtr m_dataPack;
		DataDocumentPtr m_data;

		std::vector<unsigned int>	m_trainingInds,
									m_testingInds;

		std::vector<InstancePtr>	m_cl1Instances,
									m_cl2Instances;

		unsigned int m_stage;
		unsigned int m_numStages;
	};
}

typedef boost::shared_ptr<SVM_Framework::IEvaluation> IEvaluationPtr;