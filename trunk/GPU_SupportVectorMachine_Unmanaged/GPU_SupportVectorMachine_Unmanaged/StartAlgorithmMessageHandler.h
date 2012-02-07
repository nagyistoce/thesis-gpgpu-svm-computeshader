#pragma once
#include "MessageHandler.h"
#include "AlgorithmDataPack.h"
#include "IAlgorithm.h"

namespace SVM_Framework{
	class StartAlgorithmMessageHandler : public MessageHandler{
	public:
		StartAlgorithmMessageHandler(GraphicsManagerPtr gfxMgr);

		void handle(IDataPackPtr dataPack);
	private:
		std::map<std::string,IAlgorithmPtr> m_algorithms;
	};
}