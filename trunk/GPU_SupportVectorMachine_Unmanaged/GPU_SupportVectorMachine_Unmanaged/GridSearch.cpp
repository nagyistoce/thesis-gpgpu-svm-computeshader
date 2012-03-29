#include "stdafx.h"
#include "GridSearch.h"

namespace SVM_Framework{
	GridSearch::GridSearch(MainFrameworkPtr framework):SearchScheme(framework){
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.001953125)); 
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.00390625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.0625));  
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(1.0, 0.125));

		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.001953125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.00390625)); 
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.0625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(3.1622776601683795, 0.125));
		
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.001953125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.00390625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.0625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(10.0, 0.125));

		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.001953125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.00390625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.0625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(31.622776601683793, 0.125));
		
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.001953125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.00390625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.0625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(100.0, 0.125));
		
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 9.765625E-4));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.001953125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.00390625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.0078125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.015625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.03125));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.0625));
		m_grid.push_back(std::pair<Value::v_precision,Value::v_precision>(316.2277660168379, 0.125));

		m_gridStage = m_grid.begin();

		int stageStart = 0;
		try{
			stageStart = boost::lexical_cast<int>(m_framework->getGuiPtr()->getEditText(IDC_EDIT_GRIDSTART));
		}
		catch(...){
			stageStart = 0;
		}

		if(stageStart < m_grid.size() && stageStart > 0){
			for(unsigned int i=0; i<stageStart; i++){
				m_gridStage++;
			}
		}
	}

	bool GridSearch::callbackProcessing(IAlgorithmPtr algorithm){
		m_output << "c: " << m_gridStage->first << " gamma: " << m_gridStage->second << "\n";
		m_output << algorithm->getoutputStream().str();
		m_output << "\n------------------------------------------------------------------------\n";

		m_gridStage++;

		if(m_gridStage != m_grid.end())
			return true;
		return false;
	}

	std::string GridSearch::printProcess(){
		unsigned int bestAcc,bestBER,bestEF;
		double acc = 0, BER = 1, EF = 0;
		for(unsigned int i=0; i<m_resPacks.size(); i++){
			if(acc < m_resPacks[i].accuracy){
				acc = m_resPacks[i].accuracy;
				bestAcc = i;
			}
			if(BER > m_resPacks[i].balancedErrorRate){
				BER = m_resPacks[i].balancedErrorRate;
				bestBER = i;
			}
			if(EF < m_resPacks[i].cl1EnrichmentFactor || EF < m_resPacks[i].cl2EnrichmentFactor){
				if(m_resPacks[i].cl1EnrichmentFactor > m_resPacks[i].cl2EnrichmentFactor)
					bestEF = m_resPacks[i].cl1EnrichmentFactor;
				else
					bestEF = m_resPacks[i].cl2EnrichmentFactor;
				EF = i;
			}
		}

		m_output << "Best accuracy: " << acc*100 << "%\r\n	c: " << m_grid[bestAcc].first << " gamma: " << m_grid[bestAcc].second << "\n";
		m_output << "Best balanced error rate: " << BER << "\r\n	c: " << m_grid[bestBER].first << " gamma: " << m_grid[bestBER].second << "\n";
		m_output << "Best enrichment factor: " << EF << "\r\n	c: " << m_grid[bestEF].first << " gamma: " << m_grid[bestEF].second << "\n";

		return "gridSearch_";
	}

	void GridSearch::runProcess(){
		m_framework->getGuiPtr()->disableAllButStop();

		std::wstringstream sstream;
		sstream << m_gridStage->first;
		m_framework->getGuiPtr()->setText(IDC_EDIT_C,sstream.str());

		sstream = std::wstringstream();
		sstream << m_gridStage->second;
		m_framework->getGuiPtr()->setText(IDC_EDIT_PARAM2,sstream.str());
	}
}