#include "stdafx.h"
#include "GUIWindow.h"

namespace SVM_Framework{
	GUIWindow::GUIWindow(int id, std::wstring type, HWND hwnd, DWORD style1, DWORD style2, int x, int y, int width, int height, HINSTANCE instance, std::wstring text){
		m_hwnd = CreateWindowEx(	style1,	
									type.c_str(),
									text.c_str(),
									style2,
									x,
									y,
									width,
									height,
									hwnd,
									(HMENU)id,
									instance,
									NULL);
		HFONT defaultFont;
		defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(m_hwnd, WM_SETFONT, WPARAM (defaultFont), TRUE);
	}

	GUIWindow::~GUIWindow(){
		DestroyWindow(m_hwnd);
	}
}