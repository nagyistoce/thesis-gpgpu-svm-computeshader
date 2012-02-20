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

		// Add windows
		addWindow(IDC_BUTTON_RUN,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,10,5,105,25,L"Run");
		addWindow(IDC_EDIT_PARAM1,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,140,7,100,21,L"puk");
		addWindow(IDC_EDIT_PARAM2,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,245,7,100,21,L"1.0");
		addWindow(IDC_EDIT_PARAM3,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,350,7,100,21,L"1.0");

		addWindow(IDC_EDIT_FILEPATH,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,455,7,580,21,L"E:/Projekts/DSV/Current/Exjobb/Program/Training files/oselma_short.txt");

		addWindow(IDC_STATIC_INFOTEXT,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,10,35,505,740,L"");
		addWindow(IDC_STATIC_DEBUG,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,530,35,505,740,L"");
	}

	void GUIManager::launchAlgo(){
		AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new AlgorithmDataPack);
		data->m_algoName = "DX11SVM";
		std::stringstream ss;
		ss << getEditText(IDC_EDIT_FILEPATH);
		data->m_dataResource = ss.str();
		data->m_gui = this;

		m_framework->postMessage(StartAlgorithmMessagePtr(new StartAlgorithmMessage(data)));
	}

	void GUIManager::setText(int id, std::wstring message){
		HWND hwnd = m_windows[id]->getHWND();
		SetWindowText(hwnd,message.c_str());
	}

	std::string GUIManager::getEditText(int id){
		std::string result;
		HWND hwnd = m_windows[id]->getHWND();
		CHAR buff[1024];
		GetWindowTextA(hwnd, buff, 1024);
		result = buff;
		return result;
	}

	void GUIManager::postDebugMessage(std::wstring message){
		m_debugString += message;
		HWND hwnd = m_windows[IDC_STATIC_DEBUG]->getHWND();
		SetWindowText(hwnd,m_debugString.c_str());
	}

	unsigned int GUIManager::addWindow(int id, std::wstring type, DWORD style1, DWORD style2, int x, int y, int width, int height, std::wstring text){
		m_windows[id] = GUIWindowPtr(new GUIWindow(id,type,m_hwnd,style1,style2,x,y,width,height,m_hinstance,text));
		return id;
	}

	void GUIManager::removeWindow(unsigned int id){
		m_windows.erase(id);
	}
}