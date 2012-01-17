#pragma once
#include "MessageHandlerDataPack.h"

namespace SVM_Framework{
	class MessageHandler{
	public:
		virtual void handle(MessageHandlerDataPackPtr dataPack) = 0;
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::MessageHandler> MessageHandlerPtr;