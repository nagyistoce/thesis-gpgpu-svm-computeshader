#include "stdafx.h"
#include "GUIManager.h"
#include "MainFramework.h"
#include "DirectXManager.h"
#include "StartAlgorithmMessage.h"
#include "AlgorithmDataPack.h"

namespace SVM_Framework{
	GUIManager::GUIManager(HWND hwnd, HINSTANCE hInst){
		m_hwnd = hwnd;
		m_hinstance = hInst;

		m_framework = MainFrameworkPtr(new MainFramework(DirectXManagerPtr(new DirectXManager())));
		m_framework->run();
		m_windowId = 0;

		// Add windows
		addWindow(L"BUTTON",10,5,105,20,L"Run");
		m_editWindow = addWindow(L"STATIC",10,35,505,700,L"");
	}

	void GUIManager::launchAlgo(){
		AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new AlgorithmDataPack);
		data->m_algoName = "CPUSVM";
		data->m_dataResource = "E:\\Projekts\\DSV\\Current\\Exjobb\\Program\\Training files\\oselma.txt";
		data->m_gui = this;

		m_framework->postMessage(StartAlgorithmMessagePtr(new StartAlgorithmMessage(data)));
	}

	void GUIManager::setText(int id, std::wstring message){
		HWND hwnd = m_windows[id]->getHWND();
		SetWindowText(hwnd,message.c_str());
	}

	unsigned int GUIManager::addWindow(std::wstring type, int x, int y, int width, int height, std::wstring text){
		unsigned int id = m_windowId++;
		m_windows[id] = GUIWindowPtr(new GUIWindow(type,m_hwnd,x,y,width,height,m_hinstance,text));
		return id;
	}

	void GUIManager::removeWindow(unsigned int id){
		m_windows.erase(id);
	}
}