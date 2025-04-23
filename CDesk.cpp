// CDesk.cpp : Defines the entry point for the application.
//


#include "CDesk.h"
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <Unknwn.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <WebView2.h>
#define MAX_LOADSTRING 100
#include <wrl.h>
#include <wil/com.h>
#include <wrl/client.h>
#include <filesystem>
#include <thread>
#include <iostream>


using namespace Microsoft::WRL;
wil::com_ptr<ICoreWebView2> g_webview;
wil::com_ptr<ICoreWebView2Controller> g_webviewController;
const wchar_t* webviewRuntimePath = L"Microsoft.WebView2.FixedVersionRuntime.134.0.3124.93.x64";
const wchar_t* htmlFilePath;

void EnsureFolderExists(const std::wstring& folderPath) {
    if (!std::filesystem::exists(folderPath)) {
        std::filesystem::create_directories(folderPath); // Creates all intermediate directories if they don't exist
    }
}

std::wstring GetExecutableDir() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::filesystem::path exePath(path);
    return exePath.parent_path().wstring();
}

std::wstring GetHtmlFilePath() {
    std::wstring exeDir = GetExecutableDir();
    std::filesystem::path htmlPath = std::filesystem::path(exeDir) / L"CantariApp" / L"index.html";
    return L"file:///" + htmlPath.wstring();
}

std::wstring GetSiteFilePath(const std::wstring& filename) {
    return (std::filesystem::path(GetExecutableDir()) / L"CantariApp" / filename).wstring();
}


std::vector<std::wstring> GetMissingFiles() {
    std::vector<std::wstring> requiredFiles = {
        L"index.html",
        L"favicon.ico",
        L"script.js",
        L"styles.css"
    };

    std::vector<std::wstring> missingFiles;

    for (const auto& file : requiredFiles) {
        if (!std::filesystem::exists(GetSiteFilePath(file))) {
            missingFiles.push_back(file);
        }
    }

    return missingFiles;
}

void RunCommandInBackground(const std::wstring& command) {
    // Convert the wstring to a narrow string (std::string)
    std::string commandStr(command.begin(), command.end());

    // Prepare the process startup info
    STARTUPINFO si = {0};
    si.cb = sizeof(si);  // Initialize the size of the structure
    si.dwFlags = STARTF_USESHOWWINDOW;  // Control window visibility
    si.wShowWindow = SW_HIDE;  // Hide the window (use SW_MINIMIZE for minimized)

    // Prepare the process information
    PROCESS_INFORMATION pi;
    std::vector<wchar_t> cmdLine(command.begin(), command.end());
    cmdLine.push_back(0);
	// Null-terminate the command line
    // Create the process with hidden window
    if (!CreateProcessW(
        NULL,
        cmdLine.data(),     // LPWSTR
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,   // no console window
        NULL,
        NULL,
        &si,
        &pi))
    {
        std::wcerr << L"CreateProcessW failed: " << GetLastError() << L"\n";
    }
    else {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void DownloadMissingFilesFromGitHub() {
    const std::wstring token = L"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";
    const std::wstring baseUrl = L"https://api.github.com/repos/Badpythondev/CWApp/contents/CantariApp/";

    // Get the missing files
    std::vector<std::wstring> missingFiles = GetMissingFiles();

    // Ensure that the target directory exists before downloading files
    std::wstring targetDir = (std::filesystem::path(GetExecutableDir()) / L"CantariApp").wstring();
    EnsureFolderExists(targetDir);

    // If there are missing files, download them
    for (const auto& file : missingFiles) {
        std::wstring fullPath = GetSiteFilePath(file);
        std::wstring command = L"curl.exe -L -H \"Authorization: token " + token + L"\" "
            L"-H \"Accept: application/vnd.github.v3.raw\" "
            + baseUrl + file + L" -o \"" + fullPath + L"\"";
        std::string commandStr(command.begin(), command.end());
        std::system(commandStr.c_str());
    }
}
void dwnl_thread() {
	DownloadMissingFilesFromGitHub();
}
void InitWebView(HWND hwnd) {
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        nullptr,
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            wil::com_ptr<ICoreWebView2> webview;
                            controller->get_CoreWebView2(&webview);
                            g_webview = webview;
							g_webviewController = controller;

                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            controller->put_Bounds(bounds);

                            // Load the local HTML file
                            if (!GetMissingFiles().empty()) {
								std::thread downloadThread(dwnl_thread);
								downloadThread.detach();
							    MessageBox(hwnd, L"Missing Files, Downolading", L"Info", MB_OK);
                                //DownloadMissingFilesFromGitHub();
                            }
                            std::wstring htmlFilePath = GetHtmlFilePath();
                            g_webview->Navigate(htmlFilePath.c_str());
							//webview->Navigate(L"https://google.com");
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HINSTANCE hInst;
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];
    hInst = hInstance; // Store instance handle in our global variable

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CDESK, szWindowClass, MAX_LOADSTRING);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    InitWebView(hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CDESK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CDESK));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CDESK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CDESK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
/*BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}*/

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (g_webviewController)
        {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            g_webviewController->put_Bounds(bounds);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
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
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
