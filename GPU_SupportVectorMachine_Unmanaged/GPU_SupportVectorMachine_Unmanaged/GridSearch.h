#pragma once
#include "SearchScheme.h"

namespace SVM_Framework{
	class GridSearch : public SearchScheme{
	public:
		GridSearch(MainFrameworkPtr framework);
	protected:
		bool callbackProcessing(IAlgorithmPtr algorithm);
		std::string printProcess();
		void runProcess();

		std::vector<std::pair<Value::v_precision,Value::v_precision>> m_grid;
		std::vector<std::pair<Value::v_precision,Value::v_precision>>::iterator m_gridStage;
	};
}