#include "stdafx.h"
#include "GUIManager.h"
#include "MainFramework.h"
#include "DirectXManager.h"
#include <CommCtrl.h>

namespace SVM_Framework{
	GUIManager::GUIManager(HWND hwnd, HINSTANCE hInst){
		m_hwnd = hwnd;
		m_hinstance = hInst;

		// Add windows
		addWindow(IDC_BUTTON_RUN,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,10,5,105,25,L"Run");
		addWindow(IDC_BUTTON_STOP,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,120,5,105,25,L"Stop");
		addWindow(IDC_BUTTON_LOAD,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,345,5,105,25,L"Load");
		addWindow(IDC_BUTTON_GRIDSEARCH,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,1050,175,105,25,L"Grid search");
		addWindow(IDC_BUTTON_PERFSEARCH,L"BUTTON",0,WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,1050,145,105,25,L"Perf search");
		enableWindow(IDC_BUTTON_STOP,false);

		addWindow(IDC_PROGRESSBAR_PROGRESS,PROGRESS_CLASS,0,WS_VISIBLE|WS_CHILD|PBS_SMOOTH,230,5,110,15,L"");
		addWindow(IDC_PROGRESSBAR_PROGRESS2,PROGRESS_CLASS,0,WS_VISIBLE|WS_CHILD|PBS_SMOOTH,230,21,110,8,L"");

		addWindow(IDC_EDIT_PARAM2,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1105,110,50,21,L"0.01");
		addWindow(IDC_EDIT_PARAM3,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1160,110,50,21,L"1.0");
		addWindow(IDC_EDIT_FILEPATH,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,455,7,580,21,L"E:/Projekts/DSV/Current/Exjobb/Program/Training files/oselma_short.txt");
		addWindow(IDC_STATIC_INFOTEXT,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,10,35,505,740,L"");
		addWindow(IDC_STATIC_DEBUG,L"EDIT",0,ES_MULTILINE|WS_VSCROLL|WS_BORDER|WS_VISIBLE|WS_CHILD,530,35,505,740,L"");
		addWindow(IDC_EDIT_C,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1050,35,100,21,L"1.0");
		addWindow(IDC_EDIT_EVALPARAM,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1050,85,100,21,L"70");
		addWindow(IDC_EDIT_GRIDSTART,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1160,177,40,21,L"0");
		addWindow(IDC_EDIT_PERFSTART,L"EDIT",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD,1160,147,40,21,L"0");

		std::vector<std::wstring> items;
		addWindow(IDC_COMBO_KERNEL,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1050,110,50,21,L"");
		items.push_back(L"RBF");
		items.push_back(L"Puk");
		addItemsToWindow(IDC_COMBO_KERNEL,items);
		items.clear();

		addWindow(IDC_COMBO_KERNELCACHE,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1155,7,50,21,L"");
		items.push_back(L"true");
		items.push_back(L"false");
		addItemsToWindow(IDC_COMBO_KERNELCACHE,items);
		items.clear();

		addWindow(IDC_COMBO_KERNELCACHEFULL,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1210,7,50,21,L"");
		items.push_back(L"false");
		items.push_back(L"true");
		addItemsToWindow(IDC_COMBO_KERNELCACHEFULL,items);
		items.clear();

		addWindow(IDC_COMBO_EVALUATION,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1050,60,100,21,L"");
		items.push_back(L"PercentageSplit");
		items.push_back(L"CrossValidation");
		addItemsToWindow(IDC_COMBO_EVALUATION,items);
		items.clear();
		
		addWindow(IDC_COMBO_ALGO,L"COMBOBOX",WS_EX_CLIENTEDGE,WS_VISIBLE|WS_CHILD|LBS_STANDARD,1050,7,100,21,L"");
		items.push_back(L"OpenCLSVM");
		items.push_back(L"CUDASVM");
		items.push_back(L"DX11SVM");
		items.push_back(L"CPUSVM");
		addItemsToWindow(IDC_COMBO_ALGO,items);
	}

	void GUIManager::disableAllButStop(){
		enableWindow(IDC_BUTTON_RUN,false);
		enableWindow(IDC_EDIT_PARAM2,false);
		enableWindow(IDC_EDIT_PARAM3,false);
		enableWindow(IDC_EDIT_FILEPATH,false);
		enableWindow(IDC_EDIT_C,false);
		enableWindow(IDC_COMBO_KERNEL,false);
		enableWindow(IDC_EDIT_EVALPARAM,false);
		enableWindow(IDC_COMBO_EVALUATION,false);
		enableWindow(IDC_COMBO_ALGO,false);
		enableWindow(IDC_COMBO_KERNELCACHE,false);
		enableWindow(IDC_COMBO_KERNELCACHEFULL,false);
		enableWindow(IDC_BUTTON_LOAD,false);
		enableWindow(IDC_BUTTON_GRIDSEARCH,false);
		enableWindow(IDC_BUTTON_PERFSEARCH,false);
		enableWindow(IDC_EDIT_GRIDSTART,false);
		enableWindow(IDC_EDIT_PERFSTART,false);
		enableWindow(IDC_BUTTON_STOP,true);
		setProgressBar(IDC_PROGRESSBAR_PROGRESS,100,0);
		setProgressBar(IDC_PROGRESSBAR_PROGRESS2,100,0);
	}

	void GUIManager::enableAllButStop(){
		enableWindow(IDC_BUTTON_RUN,true);
		enableWindow(IDC_EDIT_PARAM2,true);
		enableWindow(IDC_EDIT_PARAM3,true);
		enableWindow(IDC_EDIT_FILEPATH,true);
		enableWindow(IDC_EDIT_C,true);
		enableWindow(IDC_COMBO_KERNEL,true);
		enableWindow(IDC_EDIT_EVALPARAM,true);
		enableWindow(IDC_COMBO_EVALUATION,true);
		enableWindow(IDC_COMBO_ALGO,true);
		enableWindow(IDC_COMBO_KERNELCACHE,true);
		enableWindow(IDC_COMBO_KERNELCACHEFULL,true);
		enableWindow(IDC_BUTTON_LOAD,true);
		enableWindow(IDC_BUTTON_GRIDSEARCH,true);
		enableWindow(IDC_BUTTON_PERFSEARCH,true);
		enableWindow(IDC_EDIT_GRIDSTART,true);
		enableWindow(IDC_EDIT_PERFSTART,true);
		enableWindow(IDC_BUTTON_STOP,false);
		setProgressBar(IDC_PROGRESSBAR_PROGRESS,100,0);
		setProgressBar(IDC_PROGRESSBAR_PROGRESS2,100,0);
	}

	void GUIManager::setText(int id, std::wstring message){
		HWND hwnd = m_windows[id]->getHWND();
		SetWindowText(hwnd,message.c_str());
	}

	void GUIManager::setProgressBar(int id, unsigned int max, unsigned int progress){
		HWND hwnd = m_windows[id]->getHWND();
		SendMessage(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, max));
		SendMessage(hwnd, PBM_SETPOS, (WPARAM)progress, 0);
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