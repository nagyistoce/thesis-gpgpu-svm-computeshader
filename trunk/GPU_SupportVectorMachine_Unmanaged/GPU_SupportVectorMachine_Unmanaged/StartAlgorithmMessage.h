#pragma once
#include "FrameworkMessage.h"

namespace SVM_Framework{
	class StartAlgorithmMessage : public FrameworkMessage{
	public:
		StartAlgorithmMessage(IDataPackPtr datapack):FrameworkMessage("StartAlgorithm",datapack){}
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::StartAlgorithmMessage> StartAlgorithmMessagePtr;