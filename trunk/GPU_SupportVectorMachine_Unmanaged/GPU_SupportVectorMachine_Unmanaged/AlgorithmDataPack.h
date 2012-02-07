#pragma once
#include "IDataPack.h"

namespace SVM_Framework{
	class AlgorithmDataPack : public IDataPack{
	public:
		std::string m_algoName;
		std::string m_dataResource;
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::AlgorithmDataPack> AlgorithmDataPackPtr;