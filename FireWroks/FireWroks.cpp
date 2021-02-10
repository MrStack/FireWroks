// FireWroks.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FireWroks.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define MAX_LOADSTRING 100

#define TIMER_CHANGE_RADIUS 0
#define TIMER_DRAW 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const int windowX{ 500 };
const int windowY{ 500 };
HWND ghWnd{};
std::shared_ptr<Drawer> spDrawer{};
std::shared_ptr<FireWorks> spFire{};

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    LowLevelMouseProc(int, WPARAM, LPARAM);
VOID    CALLBACK    TimerChangeRadius(HWND, UINT, UINT_PTR, DWORD);
VOID    CALLBACK    TimerDraw(HWND, UINT, UINT_PTR, DWORD);

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
    LoadStringW(hInstance, IDC_FIREWROKS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FIREWROKS));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIREWROKS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_FIREWROKS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


void WINAPI OnTimerEvent(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
    auto start = std::chrono::high_resolution_clock::now();
    SendMessage(ghWnd, WM_PAINT, 0, 0);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::string result{ "Time elapsed:" };
    OutputDebugStringA((result + std::to_string(diff.count()*1000) + "\n").c_str());
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT, szWindowClass, szTitle, WS_POPUP,
       CW_USEDEFAULT, CW_USEDEFAULT, windowX, windowY, NULL, NULL, hInstance, NULL);
   SetLayeredWindowAttributes(hWnd, RGB(0,0,0), (255 * 0) / 100, LWA_COLORKEY);

   POINT mousePoint{};
   GetCursorPos(&mousePoint);
   MoveWindow(hWnd, mousePoint.x - windowX / 2, mousePoint.y - windowY / 2, windowX, windowY, TRUE);

   ghWnd = hWnd;
   auto mouse_hook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandleW(0) , 0);
   auto hot_key = GlobalAddAtom(L"Exit");
   RegisterHotKey(hWnd, hot_key, MOD_ALT, 0x51);

   spDrawer = std::make_shared<Drawer>(hWnd);
   spFire = std::make_shared<FireWorks>(spDrawer);


   if (NULL == timeSetEvent(30, 1, OnTimerEvent, NULL, TIME_PERIODIC))
   {
       MessageBox(NULL, L"timeSetEvent Failed.\n", L"ERROR", 48);
   }

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

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
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_MOUSEMOVE:
        {            
            POINT mousePoint{};
            GetCursorPos(&mousePoint);
            MoveWindow(hWnd, mousePoint.x-windowX/2, mousePoint.y-windowY/2, windowX, windowY, TRUE);
        }        
        break;
    case WM_HOTKEY:
        {
            if (LOWORD(lParam) == MOD_ALT && HIWORD(lParam) == 0x51)
            {
                PostQuitMessage(0);
            }
        }
        break;
    case WM_TIMER:
        {
            
            //spFire->change_radius();
            //spFire->bloom(windowX / 2, windowY / 2);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            spDrawer->DrawParticles();
            //spFire->bloom();

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_CHANGE_RADIUS);
        KillTimer(hWnd, TIMER_DRAW);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (wParam == WM_MOUSEMOVE)
    {
        PostMessage(ghWnd, WM_MOUSEMOVE, wParam, lParam);
    }    

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
