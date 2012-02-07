#pragma once

namespace SVM_Framework{
	class GUIWindow{
	public:
		GUIWindow(std::wstring type, HWND hwnd, int x, int y, int width, int height, HINSTANCE instance, std::wstring text);
		~GUIWindow();

		HWND getHWND() {return m_hwnd;}
	private:
		HWND m_hwnd;
	};
}

typedef boost::shared_ptr<SVM_Framework::GUIWindow> GUIWindowPtr;