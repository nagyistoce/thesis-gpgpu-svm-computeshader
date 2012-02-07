#pragma once
#include "MainFramework.h"
#include "GUIWindow.h"

namespace SVM_Framework{
	class GUIManager{
	public:
		GUIManager(HWND hwnd, HINSTANCE hInst);

		void launchAlgo();
		unsigned int getEditWindowId() { return m_editWindow; }
		void setText(int id, std::wstring message);

		unsigned int addWindow(std::wstring type, int x, int y, int width, int height, std::wstring text);
		void removeWindow(unsigned int id);
	private:
		MainFrameworkPtr m_framework;
		std::map<unsigned int, GUIWindowPtr> m_windows;

		HWND m_hwnd;
		HINSTANCE m_hinstance;
		unsigned int m_windowId;

		unsigned int m_editWindow;
	};
}

typedef boost::shared_ptr<SVM_Framework::GUIManager> GUIManagerPtr;