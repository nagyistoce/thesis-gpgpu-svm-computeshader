#include "stdafx.h"
#include "MainFramework.h"
#include "StartAlgorithmMessageHandler.h"
#include "StopAlgorithmMessage.h"
#include "GUIManager.h"

namespace SVM_Framework{
	MainFramework::MainFramework(GraphicsManagerPtr gmgr):m_endParsing(false){
		m_rManager = ResourceManagerPtr(new ResourceManager);
		m_gManager = gmgr;
		m_messageStackMutex = MutexPtr(new boost::mutex);
		m_parseCondition = ConditionPtr(new boost::condition_variable);
	}

	MainFramework::~MainFramework(){
		m_messageHandlers.clear();
		m_endParsing = true;
		m_parseCondition->notify_all();
		m_messageParser->join();

		std::map<std::string,boost::shared_ptr<boost::thread>>::iterator itr = m_runningThreads.begin();
		while(itr != m_runningThreads.end()){
			itr->second->interrupt();
			itr->second->join();
			itr++;
		}
		m_runningThreads.clear();
	}

	void MainFramework::run(){
		if(!m_messageParser)
			m_messageParser = ThreadPtr(new boost::thread(&MainFramework::parseMessages,this));
	}

	void MainFramework::postMessage(FrameworkMessagePtr message){
		m_messageStackMutex->lock();
		m_messageStack.push_front(message);
		m_messageStackMutex->unlock();

		m_parseCondition->notify_one();
	}

	void MainFramework::initHandlers(){
		// Add messageHandlers
		m_messageHandlers["StartAlgorithm"] = MessageHandlerPtr(new StartAlgorithmMessageHandler(m_gManager));
	}

	void MainFramework::parseMessages(){
		m_gManager->initialize();
		initHandlers();
		boost::mutex mut;
		boost::unique_lock<boost::mutex> lock(mut);
		while(!m_endParsing){
			std::list<FrameworkMessagePtr> stack;
			m_messageStackMutex->lock();
			stack = m_messageStack;
			m_messageStack.clear();
			m_messageStackMutex->unlock();

			std::map<std::string,MessageHandlerPtr>::iterator handlerItr;
			while(!stack.empty()){
				if((handlerItr = m_messageHandlers.find(stack.front()->getMessage())) != m_messageHandlers.end()){
					if(m_runningThreads.find(stack.front()->getMessage()) == m_runningThreads.end()){
						IDataPackPtr dataPack = stack.front()->getDataPack();
						dataPack->m_gfxMgr = m_gManager;
						dataPack->m_recMgr = m_rManager;

						m_runningThreads[stack.front()->getMessage()] = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&MainFramework::runThread,this,stack.front()->getMessage(),dataPack)));
						if(SetThreadPriority(m_runningThreads[stack.front()->getMessage()]->native_handle(),THREAD_PRIORITY_BELOW_NORMAL) == 0){
							assert(0);
						}
					}
				}
				else if(stack.front()->getMessage().compare("StopAlgorithm") == 0){
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_BUTTON_STOP,false);
					m_runningThreads["StartAlgorithm"]->interrupt();
					m_runningThreads["StartAlgorithm"]->join();
					m_runningThreads.erase("StartAlgorithm");
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_BUTTON_RUN,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_EDIT_PARAM2,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_EDIT_PARAM3,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_EDIT_FILEPATH,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_EDIT_C,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_COMBO_KERNEL,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_EDIT_EVALPARAM,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_COMBO_EVALUATION,true);
					stack.front()->getDataPack()->m_gui->enableWindow(IDC_COMBO_ALGO,true);
					stack.front()->getDataPack()->m_gui->setText(IDC_STATIC_INFOTEXT,L"");
					stack.front()->getDataPack()->m_gui->setText(IDC_STATIC_DEBUG,L"");
				}
				else{
					TRACE_DEBUG("Message without registered handler recieved.");
					assert(0);
				}

				stack.pop_front();
			}

			m_parseCondition->wait(lock);
		}
	}

	void MainFramework::addHandler(std::string messageId, MessageHandlerPtr handler){
		m_messageHandlers[messageId] = handler;
	}

	void MainFramework::runThread(std::string message, IDataPackPtr dataPack){
		std::map<std::string,MessageHandlerPtr>::iterator handlerItr;
		handlerItr = m_messageHandlers.find(message);
		handlerItr->second->handle(dataPack);
		m_runningThreads.erase(message);
	}
}