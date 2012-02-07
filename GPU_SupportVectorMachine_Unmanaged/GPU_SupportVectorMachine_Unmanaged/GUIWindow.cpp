#include "stdafx.h"
#include "GUIWindow.h"

namespace SVM_Framework{
	GUIWindow::GUIWindow(std::wstring type, HWND hwnd, int x, int y, int width, int height, HINSTANCE instance, std::wstring text){
		m_hwnd = CreateWindow(	type.c_str(),
								text.c_str(),
								WS_VISIBLE | WS_CHILD,
								x,
								y,
								width,
								height,
								hwnd,
								NULL,
								instance,
								NULL);
	}

	GUIWindow::~GUIWindow(){
		DestroyWindow(m_hwnd);
	}
}