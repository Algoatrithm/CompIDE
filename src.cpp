#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#define NEWFILE 0x3E9
#define OPENFILE 0x3EA
#define THEME 0x3EB
#define MAINMENU 1
#define EDITOR 2

#define LIGHT_MODE 1
#define DARK_MODE 2
#define DARK_MODE_DARKER 3
#define NERD_MODE 4

int WindowState;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Editor(HDC hdc, PAINTSTRUCT &ps);

void MainMenu(HDC hdc, PAINTSTRUCT &ps);
void debug();

LPRECT WindowRect;

RECT NewFileRect;
RECT EditorHeaderRect;
RECT HeaderMenu;
RECT SavedFilesPaintRect;

HWND hwndNewFileButton;
HWND hwndEditorNewFileButton;
HWND hwndEditorRecentFileButton;
HWND hwndEditorThemeButton;
HWND hwnd;

int Theme = LIGHT_MODE;

struct WinConst
{
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpCmdLine;
    int nCmdShow;
};
const wchar_t CLASS_NAME[] = L"MainMenu";

struct WinConst _WinConst;

void RunApplication();

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class.

    WNDCLASS MainMenu = {};
    MainMenu.lpfnWndProc = WindowProc;
    MainMenu.hInstance = hInstance;
    MainMenu.lpszClassName = CLASS_NAME;
    RegisterClass(&MainMenu);

    _WinConst.hInstance = hInstance;
    _WinConst.hPrevInstance = hPrevInstance;
    _WinConst.lpCmdLine = lpCmdLine;
    _WinConst.nCmdShow = nCmdShow;

    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    // Create the window.
    hwnd = CreateWindowEx(
        0,          // Optional window styles.
        CLASS_NAME, // Window class
        L"CompIDE", // Window text
        style,      // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,                // Parent window
        NULL,                // Menu
        _WinConst.hInstance, // Instance handle
        NULL                 // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);

    WindowState = MAINMENU;

    // Run the message loop.

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// Main Window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_COMMAND:
        // New File
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case NEWFILE:
                WindowState = EDITOR;
                InvalidateRect(hwnd, WindowRect, true);
                ShowWindow(hwndNewFileButton, SW_HIDE);
                break;
            case OPENFILE:

                break;
            case THEME:
                Theme++;
                if (Theme > NERD_MODE)
                    Theme = LIGHT_MODE;
                InvalidateRect(hwnd, WindowRect, true);
                break;
            }
        }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(Theme));

        // State machine
        switch (WindowState)
        {
        case MAINMENU:
            MainMenu(hdc, ps);
            break;
        case EDITOR:
            Editor(hdc, ps);
            break;
        }

        EndPaint(hwnd, &ps);
    }
    break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Editor(HDC hdc, PAINTSTRUCT &ps)
{
    EditorHeaderRect.bottom = 18;
    EditorHeaderRect.left = 50;
    EditorHeaderRect.right = 2;
    EditorHeaderRect.top = 2;

    HeaderMenu.left = 1500; // x-upper left
    /*
        #----------# <--- x
        |          |
        |          |
        |          |
        |          |
        #----------#

    */
    HeaderMenu.right = 0; // x-lower right
    /*
        #----------#
        |          |
        |          |
        |          |
        |          |
 x ---> #----------#

    */
    HeaderMenu.top = 0; // y-upper left
    /*
        #----------# <--- y
        |          |
        |          |
        |          |
        |          |
        #----------#

    */
    HeaderMenu.bottom = 20; // y-lower right
                            /*
                                #----------#
                                |          |
                                |          |
                                |          |
                                |          |
                         y ---> #----------#
                        
                            */

    // Open a handle to the file
    HANDLE hFile = CreateFile(
        L"C:\\NewFile.txt",    // Filename
        GENERIC_WRITE,         // Desired access
        FILE_SHARE_READ,       // Share mode
        NULL,                  // Security attributes
        CREATE_NEW,            // Creates a new file, only if it doesn't already exist
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes
        NULL);                 // Template file handle
    if (hFile == INVALID_HANDLE_VALUE)
    {
        // Failed to open/create file
    }

    HFONT hFontHeaderButton = CreateFontW(
        12,                  // height (pixels, negative = character height)
        0,                   // width (0 = auto)
        0, 0,                // escapement, orientation
        FW_NORMAL,           // weight
        FALSE, FALSE, FALSE, // italic, underline, strikeout
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        L"Segoe UI");

    hwndEditorNewFileButton = CreateWindow(
        L"BUTTON",                                          // Predefined class; Unicode assumed
        L"New file",                                        // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        EditorHeaderRect.right,                             // x position
        EditorHeaderRect.top,                               // y position
        EditorHeaderRect.left,                              // Button width
        EditorHeaderRect.bottom,                            // Button height
        hwnd,                                               // Parent window
        (HMENU)NEWFILE,                                     // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL); // Pointer not needed.
    hwndEditorRecentFileButton = CreateWindow(
        L"BUTTON",                                          // Predefined class; Unicode assumed
        L"Open File",                                       // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        EditorHeaderRect.right + EditorHeaderRect.left * 1, // x position
        EditorHeaderRect.top,                               // y position
        EditorHeaderRect.left,                              // Button width
        EditorHeaderRect.bottom,                            // Button height
        hwnd,                                               // Parent window
        (HMENU)OPENFILE,                                    // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL); // Pointer not needed.
    hwndEditorThemeButton = CreateWindow(
        L"BUTTON",                                          // Predefined class; Unicode assumed
        L"Theme",                                           // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        EditorHeaderRect.right + EditorHeaderRect.left * 2, // x position
        EditorHeaderRect.top,                               // y position
        EditorHeaderRect.left,                              // Button width
        EditorHeaderRect.bottom,                            // Button height
        hwnd,                                               // Parent window
        (HMENU)THEME,                                       // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL); // Pointer not needed.

    // Change the font size
    SendMessage(hwndEditorNewFileButton, WM_SETFONT, (WPARAM)hFontHeaderButton, TRUE);
    SendMessage(hwndEditorRecentFileButton, WM_SETFONT, (WPARAM)hFontHeaderButton, TRUE);
    SendMessage(hwndEditorThemeButton, WM_SETFONT, (WPARAM)hFontHeaderButton, TRUE);

    ShowWindow(hwndEditorNewFileButton, SW_SHOW);
    ShowWindow(hwndEditorRecentFileButton, SW_SHOW);

    FillRect(hdc, &HeaderMenu, (HBRUSH)(COLOR_WINDOW + 2));
}

void MainMenu(HDC hdc, PAINTSTRUCT &ps)
{
    // Show the saved files
    SavedFilesPaintRect;
    SavedFilesPaintRect.left = 1100; // x-upper left
    /*
        #----------# <--- x
        |          |
        |          |
        |          |
        |          |
        #----------#

    */
    SavedFilesPaintRect.right = 600; // x-lower right
    /*
        #----------#
        |          |
        |          |
        |          |
        |          |
 x ---> #----------#

    */
    SavedFilesPaintRect.top = 50; // y-upper left
    /*
        #----------# <--- y
        |          |
        |          |
        |          |
        |          |
        #----------#

    */
    SavedFilesPaintRect.bottom = 500; // y-lower right
    /*
        #----------#
        |          |
        |          |
        |          |
        |          |
 y ---> #----------#

    */
    FillRect(hdc, &SavedFilesPaintRect, (HBRUSH)(COLOR_WINDOW + 2));

    NewFileRect.bottom = 50;
    NewFileRect.left = 150;
    NewFileRect.right = 300;
    NewFileRect.top = 250;

    hwndNewFileButton = CreateWindow(
        L"BUTTON",                                          // Predefined class; Unicode assumed
        L"New file",                                        // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        NewFileRect.right,                                  // x position
        NewFileRect.top,                                    // y position
        NewFileRect.left,                                   // Button width
        NewFileRect.bottom,                                 // Button height
        hwnd,                                               // Parent window
        (HMENU)NEWFILE,                                     // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL); // Pointer not needed.
    ShowWindow(hwndNewFileButton, SW_SHOW);

    // Show the New File button
    ShowWindow(hwndNewFileButton, SW_SHOW);
}

void debug()
{
    MessageBox(
        NULL,
        L"Works!",
        L"Got it!",
        MB_ICONEXCLAMATION);
}