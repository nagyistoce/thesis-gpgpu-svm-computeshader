#pragma once
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "FrameworkMessage.h"
#include "MessageHandler.h"

namespace SVM_Framework{
	class MainFramework{
	public:
		MainFramework(GraphicsManagerPtr gmgr);
		~MainFramework();

		void run();
		void postMessage(FrameworkMessagePtr message);
	private:
		void parseMessages();
		void addHandler(std::string messageId, MessageHandlerPtr handler);

		bool m_endParsing;
		std::list<FrameworkMessagePtr> m_messageStack;
		
		GraphicsManagerPtr m_gManager;
		ResourceManagerPtr m_rManager;

		std::map<std::string,MessageHandlerPtr> m_messageHandlers;

		MutexPtr m_messageStackMutex;
		ThreadPtr m_messageParser;
		ConditionPtr m_parseCondition;
	};
}

typedef boost::shared_ptr<SVM_Framework::MainFramework> MainFrameworkPtr;