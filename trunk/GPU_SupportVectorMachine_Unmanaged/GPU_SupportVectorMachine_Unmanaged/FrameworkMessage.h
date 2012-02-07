#pragma once
#include "IDataPack.h"

namespace SVM_Framework{
	class FrameworkMessage{
	public:
		FrameworkMessage(std::string message, IDataPackPtr dataPack):m_message(message),m_dataPack(dataPack){
		}

		std::string& getMessage() { return m_message; }
		IDataPackPtr getDataPack() { return m_dataPack; }
	private:
		IDataPackPtr m_dataPack;
		std::string m_message;
	};
}

typedef boost::shared_ptr<SVM_Framework::FrameworkMessage> FrameworkMessagePtr;