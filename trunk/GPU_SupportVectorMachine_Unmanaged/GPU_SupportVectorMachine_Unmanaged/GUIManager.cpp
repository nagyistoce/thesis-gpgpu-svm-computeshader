#include "stdafx.h"
#include "GUIManager.h"
#include "MainFramework.h"
#include "DirectXManager.h"
#include "StartAlgorithmMessage.h"
#include "StopAlgorithmMessage.h"
#include "AlgorithmDataPack.h"

namespace SVM_Framework{
	GUIManager::GUIManager(HWND hwnd, HINSTANCE hInst){
		m_hwnd = hwnd;
		m_hinstance = hInst;

		m_framework = MainFrameworkPtr(new MainFramework(DirectXManagerPtr(new DirectXManager())));
		m_framework->run();

		// Add windows
		addWindow(IDC_BUTTON_RUN,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,10,5,105,25,L"Run");
		addWindow(IDC_BUTTON_STOP,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,120,5,105,25,L"Stop");
		enableWindow(IDC_BUTTON_STOP,false);

		addWindow(IDC_EDIT_PARAM2,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,345,7,50,21,L"1.0");
		addWindow(IDC_EDIT_PARAM3,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,400,7,50,21,L"1.0");
		addWindow(IDC_EDIT_FILEPATH,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,455,7,580,21,L"E:/Projekts/DSV/Current/Exjobb/Program/Training files/oselma_short.txt");
		addWindow(IDC_STATIC_INFOTEXT,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,10,35,505,740,L"");
		addWindow(IDC_STATIC_DEBUG,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,530,35,505,740,L"");
		addWindow(IDC_EDIT_C,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1050,35,100,21,L"1.0");
		addWindow(IDC_EDIT_EVALPARAM,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1050,85,100,21,L"10");

		std::vector<std::wstring> items;
		addWindow(IDC_COMBO_KERNEL,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,290,7,50,21,L"");
		items.push_back(L"Puk");
		items.push_back(L"RBF");
		addItemsToWindow(IDC_COMBO_KERNEL,items);
		items.clear();

		addWindow(IDC_COMBO_EVALUATION,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1050,60,100,21,L"");
		items.push_back(L"CrossValidation");
		items.push_back(L"PercentageSplit");
		addItemsToWindow(IDC_COMBO_EVALUATION,items);
		items.clear();
		
		addWindow(IDC_COMBO_ALGO,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1050,7,100,21,L"");
		items.push_back(L"DX11SVM");
		items.push_back(L"CPUSVM");
		items.push_back(L"CUDASVM");
		items.push_back(L"OpenCLSVM");
		addItemsToWindow(IDC_COMBO_ALGO,items);
	}

	void GUIManager::launchAlgo(){
		enableWindow(IDC_BUTTON_RUN,false);
		enableWindow(IDC_EDIT_PARAM2,false);
		enableWindow(IDC_EDIT_PARAM3,false);
		enableWindow(IDC_EDIT_FILEPATH,false);
		enableWindow(IDC_EDIT_C,false);
		enableWindow(IDC_COMBO_KERNEL,false);
		enableWindow(IDC_EDIT_EVALPARAM,false);
		enableWindow(IDC_COMBO_EVALUATION,false);
		enableWindow(IDC_COMBO_ALGO,false);
		enableWindow(IDC_BUTTON_STOP,true);

		AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new AlgorithmDataPack);
		data->m_algoName = getEditText(IDC_COMBO_ALGO);
		std::stringstream ss;
		ss << getEditText(IDC_EDIT_FILEPATH);
		data->m_dataResource = ss.str();
		data->m_gui = this;

		m_framework->postMessage(StartAlgorithmMessagePtr(new StartAlgorithmMessage(data)));
	}

	void GUIManager::stopAlgo(){
		AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new AlgorithmDataPack);
		data->m_gui = this;
		m_framework->postMessage(StopAlgorithmMessagePtr(new StopAlgorithmMessage(data)));
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

	void GUIManager::addItemsToWindow(int id, std::vector<std::wstring> &items){
		for(unsigned int i=0; i<items.size(); i++)
			SendMessage(m_windows[id]->getHWND(), CB_ADDSTRING, 0, (LPARAM)items[i].c_str());
		SendMessage(m_windows[id]->getHWND(), CB_SETCURSEL, 0, 0);
	}

	void GUIManager::removeWindow(unsigned int id){
		m_windows.erase(id);
	}

	void GUIManager::enableWindow(int id, bool enable){
		EnableWindow(m_windows[id]->getHWND(),enable);
	}
}