#include "stdafx.h"
#include "StartAlgorithmMessageHandler.h"
#include "DX11SVM.h"
#include "CPUSVM.h"
#include "OpenCLSVM.h"
#include "CUDASVM.h"

namespace SVM_Framework{
	StartAlgorithmMessageHandler::StartAlgorithmMessageHandler(GraphicsManagerPtr gfxMgr){
		m_algorithms["DX11SVM"] = IAlgorithmPtr(new DX11SVM(gfxMgr));
		m_algorithms["CPUSVM"] = IAlgorithmPtr(new CPUSVM(gfxMgr));
		m_algorithms["CUDASVM"] = IAlgorithmPtr(new CUDASVM(gfxMgr));
		m_algorithms["OpenCLSVM"] = IAlgorithmPtr(new OpenCLSVM(gfxMgr));
	}

	void StartAlgorithmMessageHandler::handle(IDataPackPtr dataPack){
		AlgorithmDataPackPtr data = boost::static_pointer_cast<AlgorithmDataPack>(dataPack);
		std::map<std::string,IAlgorithmPtr>::iterator itr;
		if((itr = m_algorithms.find(data->m_algoName)) != m_algorithms.end()){
			itr->second->run(data);
		}
	}
}