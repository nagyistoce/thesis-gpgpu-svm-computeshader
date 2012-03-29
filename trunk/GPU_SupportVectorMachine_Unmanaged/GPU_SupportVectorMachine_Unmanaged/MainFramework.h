#pragma once
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "FrameworkMessage.h"
#include "MessageHandler.h"
#include "IAlgorithm.h"
#include "GUIManager.h"

namespace SVM_Framework{
	class MainFramework{
	public:
		MainFramework(GraphicsManagerPtr gmgr, GUIManagerPtr guimgr);
		~MainFramework();

		void run();
		void postMessage(FrameworkMessagePtr message);

		GUIManagerPtr getGuiPtr() {return m_guiManager;}
		ResourceManagerPtr getResourcePtr() {return m_rManager;}
		GraphicsManagerPtr getGraphicsPtr() {return m_gManager;}
	private:
		void initHandlers();
		void parseMessages();
		void addHandler(std::string messageId, MessageHandlerPtr handler);
		void runThread(std::string message, IDataPackPtr dataPack);

		bool m_endParsing;
		std::list<FrameworkMessagePtr> m_messageStack;
		
		GraphicsManagerPtr m_gManager;
		ResourceManagerPtr m_rManager;
		GUIManagerPtr m_guiManager;

		std::map<std::string,MessageHandlerPtr> m_messageHandlers;

		MutexPtr m_messageStackMutex;
		ThreadPtr m_messageParser;
		ConditionPtr m_parseCondition;

		std::map<std::string,boost::shared_ptr<boost::thread>> m_runningThreads;
	};
}

typedef boost::shared_ptr<SVM_Framework::MainFramework> MainFrameworkPtr;