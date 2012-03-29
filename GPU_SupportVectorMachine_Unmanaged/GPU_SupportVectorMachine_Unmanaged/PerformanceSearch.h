#pragma once
#include "SearchScheme.h"

namespace SVM_Framework{
	class PerformanceSearch : public SearchScheme{
	public:
		PerformanceSearch(MainFrameworkPtr framework, Value::v_precision stepSize);
	protected:
		struct resultCompilation{
			resultCompilation():
				m_total(0.0),
				m_training(0.0),
				m_testing(0.0),
				m_step(0.0),
				m_acc(0.0),
				m_cl1Enrich(0.0),
				m_cl2Enrich(0.0),
				m_trainingDev(0.0),
				m_testingDev(0.0)
			{}

			double	m_total,
					m_training,
					m_testing,
					m_step,
					m_acc,
					m_cl1Enrich,
					m_cl2Enrich,
					
					m_trainingDev,
					m_testingDev;
		};

		bool callbackProcessing(IAlgorithmPtr algorithm);
		std::string printProcess();
		void runProcess();

		Value::v_precision	m_step,
							m_stepSize;

		unsigned int	m_measureCount,
						m_currentStage;

		std::vector<resultCompilation> m_finalCompilation;
	};
}