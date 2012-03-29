#pragma once
#include "FrameworkMessage.h"

namespace SVM_Framework{
	class StopAlgorithmMessage : public FrameworkMessage{
	public:
		StopAlgorithmMessage(IDataPackPtr datapack):FrameworkMessage("StopAlgorithm",datapack){}
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::StopAlgorithmMessage> StopAlgorithmMessagePtr;