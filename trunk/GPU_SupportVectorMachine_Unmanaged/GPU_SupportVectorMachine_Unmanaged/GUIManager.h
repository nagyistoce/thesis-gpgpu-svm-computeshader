#pragma once
#include "MainFramework.h"
#include "GUIWindow.h"

#define IDC_BUTTON_RUN 9823
#define IDC_BUTTON_STOP 8932
#define IDC_STATIC_INFOTEXT 7398
#define IDC_STATIC_DEBUG 2892
#define IDC_EDIT_PARAM2 9375
#define IDC_EDIT_PARAM3 9475
#define IDC_EDIT_FILEPATH 9234
#define IDC_EDIT_C 1283
#define IDC_EDIT_EVALPARAM 8423

#define IDC_COMBO_ALGO 9753
#define IDC_COMBO_EVALUATION 4208
#define IDC_COMBO_KERNEL 9275

namespace SVM_Framework{
	class GUIManager{
	public:
		GUIManager(HWND hwnd, HINSTANCE hInst);

		void launchAlgo();
		void stopAlgo();
		void setText(int id, std::wstring message);
		std::string getEditText(int id);

		void postDebugMessage(std::wstring message);

		unsigned int addWindow(int id, std::wstring type, DWORD style1, DWORD style2, int x, int y, int width, int height, std::wstring text);
		void addItemsToWindow(int id, std::vector<std::wstring> &items);
		void removeWindow(unsigned int id);
		void enableWindow(int id, bool enable);
	private:
		MainFrameworkPtr m_framework;
		std::map<unsigned int, GUIWindowPtr> m_windows;

		HWND m_hwnd;
		HINSTANCE m_hinstance;

		std::wstring m_debugString;
	};
}

typedef boost::shared_ptr<SVM_Framework::GUIManager> GUIManagerPtr;