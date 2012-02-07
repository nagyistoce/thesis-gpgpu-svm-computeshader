#pragma once
#include "IDataPack.h"

namespace SVM_Framework{
	class MessageHandler{
	public:
		virtual void handle(IDataPackPtr dataPack) = 0;
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::MessageHandler> MessageHandlerPtr;