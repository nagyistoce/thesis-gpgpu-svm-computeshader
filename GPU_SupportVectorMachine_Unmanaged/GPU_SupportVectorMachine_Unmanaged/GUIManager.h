#pragma once
#include "GUIWindow.h"

#define IDC_BUTTON_RUN 3000
#define IDC_BUTTON_STOP 3001
#define IDC_BUTTON_LOAD 3002
#define IDC_BUTTON_GRIDSEARCH 3003
#define IDC_BUTTON_PERFSEARCH 3004

#define IDC_STATIC_INFOTEXT 4001
#define IDC_STATIC_DEBUG 4002

#define IDC_EDIT_PARAM2 5001
#define IDC_EDIT_PARAM3 5002
#define IDC_EDIT_FILEPATH 5003
#define IDC_EDIT_C 5004
#define IDC_EDIT_EVALPARAM 5005
#define IDC_EDIT_GRIDSTART 5006
#define IDC_EDIT_PERFSTART 5007

#define IDC_COMBO_ALGO 6001
#define IDC_COMBO_EVALUATION 6002
#define IDC_COMBO_KERNEL 6003
#define IDC_COMBO_KERNELCACHE 6004
#define IDC_COMBO_KERNELCACHEFULL 6005

#define IDC_PROGRESSBAR_PROGRESS 7001
#define IDC_PROGRESSBAR_PROGRESS2 7002

namespace SVM_Framework{
	class GUIManager{
	public:
		GUIManager(HWND hwnd, HINSTANCE hInst);

		void setText(int id, std::wstring message);
		std::string getEditText(int id);
		void setProgressBar(int id, unsigned int max, unsigned int progress);

		void postDebugMessage(std::wstring message);

		unsigned int addWindow(int id, std::wstring type, DWORD style1, DWORD style2, int x, int y, int width, int height, std::wstring text);
		void addItemsToWindow(int id, std::vector<std::wstring> &items);
		void removeWindow(unsigned int id);
		void enableWindow(int id, bool enable);

		void disableAllButStop();
		void enableAllButStop();
	private:
		std::map<unsigned int, GUIWindowPtr> m_windows;

		HWND m_hwnd;
		HINSTANCE m_hinstance;

		std::wstring m_debugString;
	};
}

typedef boost::shared_ptr<SVM_Framework::GUIManager> GUIManagerPtr;