#pragma once
#include "MessageHandlerDataPack.h"

namespace SVM_Framework{
	class FrameworkMessage{
	public:
		FrameworkMessage(std::string message, MessageHandlerDataPackPtr dataPack):m_message(message),m_dataPack(dataPack){
		}

		std::string& getMessage() { return m_message; }
		MessageHandlerDataPackPtr getDataPack() { return m_dataPack; }
	private:
		MessageHandlerDataPackPtr m_dataPack;
		std::string m_message;
	};
}

typedef boost::shared_ptr<SVM_Framework::FrameworkMessage> FrameworkMessagePtr;