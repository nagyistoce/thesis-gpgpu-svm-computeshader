// GPU_SupportVectorMachine_Unmanaged.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GPU_SupportVectorMachine_Unmanaged.h"
#include "GUIManager.h"
#include "DirectXManager.h"
#include "StartAlgorithmMessage.h"
#include "StopAlgorithmMessage.h"
#include "AlgorithmDataPack.h"
#include "MainFramework.h"
#include "GridSearch.h"
#include "PerformanceSearch.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
MainFrameworkPtr mainFramework;
boost::shared_ptr<boost::thread> loadThread;
boost::shared_ptr<SVM_Framework::GridSearch> gridSearch;
boost::shared_ptr<SVM_Framework::PerformanceSearch> perfSearch;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GPU_SUPPORTVECTORMACHINE_UNMANAGED, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GPU_SUPPORTVECTORMACHINE_UNMANAGED));

	HWND hwnd = FindWindow(szWindowClass,szTitle);
	// Create and initialize app framework
	mainFramework = MainFrameworkPtr(new SVM_Framework::MainFramework(DirectXManagerPtr(new SVM_Framework::DirectXManager()),GUIManagerPtr(new SVM_Framework::GUIManager(hwnd,hInstance))));
	mainFramework->run();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	mainFramework.reset();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GPU_SUPPORTVECTORMACHINE_UNMANAGED));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GPU_SUPPORTVECTORMACHINE_UNMANAGED);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	HWND hWnd;
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd){
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void loadFile(){
	GUIManagerPtr guiPtr = mainFramework->getGuiPtr();
	std::stringstream ss;
	ss << guiPtr->getEditText(IDC_EDIT_FILEPATH);
	DataDocumentPtr doc = mainFramework->getResourcePtr()->getDocumentResource(ss.str());

	std::wstringstream wss;
	wss << ss.str().c_str() << "\r\nAttributes: " << doc->getNumAttributes() << "\r\nInstances: " << doc->getNumInstances() << "\r\n" 
		<< "CL1 - " << doc->m_cl1Value << ":" << doc->m_numCl1 << "\r\n" << "CL2 - " << doc->m_cl2Value << ":" << doc->m_numCl2 << "\r\n";
	guiPtr->postDebugMessage(wss.str());

	guiPtr->enableAllButStop();
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDC_BUTTON_GRIDSEARCH:{
			gridSearch = boost::shared_ptr<SVM_Framework::GridSearch>(new SVM_Framework::GridSearch(mainFramework));
			gridSearch->run();
			break;
		}
		case IDC_BUTTON_PERFSEARCH:{
			perfSearch = boost::shared_ptr<SVM_Framework::PerformanceSearch>(new SVM_Framework::PerformanceSearch(mainFramework,5));
			perfSearch->run();
			break;
		}
		case IDC_BUTTON_RUN:{
			GUIManagerPtr guiPtr = mainFramework->getGuiPtr();
			guiPtr->disableAllButStop();

			AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new SVM_Framework::AlgorithmDataPack);
			data->m_algoName = guiPtr->getEditText(IDC_COMBO_ALGO);
			std::stringstream ss;
			ss << guiPtr->getEditText(IDC_EDIT_FILEPATH);
			data->m_dataResource = ss.str();

			mainFramework->postMessage(StartAlgorithmMessagePtr(new SVM_Framework::StartAlgorithmMessage(data)));
			break;
		}
		case IDC_BUTTON_STOP:{
			AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new SVM_Framework::AlgorithmDataPack);
			mainFramework->postMessage(StopAlgorithmMessagePtr(new SVM_Framework::StopAlgorithmMessage(data)));
			break;
		}
		case IDC_BUTTON_LOAD:{
			GUIManagerPtr guiPtr = mainFramework->getGuiPtr();
			guiPtr->disableAllButStop();

			if(loadThread)
				loadThread->join();

			loadThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&loadFile)));
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
