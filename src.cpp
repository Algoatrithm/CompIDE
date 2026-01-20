#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <cmath>
#include <strsafe.h>
#include <vector>

#define BUFSIZE 65535
#define SHIFTED 0x8000

#define ID_EDITCHILD 100

#define NEWFILE 0x3E9
#define OPENFILE 0x3EA
#define THEME 0x3EB
#define MAINMENU 1
#define EDITOR 2

#define LIGHT_MODE 1
#define DARK_MODE 2
#define DARK_MODE_DARKER 3
#define NERD_MODE 4

int WindowState;           // records the current state of the hwnd window
HDC hdc;                   // handle to device context
TEXTMETRIC tm;             // structure for text metrics
static DWORD dwCharX;      // average width of characters
static DWORD dwCharY;      // height of characters
static DWORD dwClientX;    // width of client area
static DWORD dwClientY;    // height of client area
static DWORD dwLineLen;    // line length
static DWORD dwLines;      // text lines in client area
static int nCaretPosX = 0; // horizontal position of caret
static int nCaretPosY = 0; // vertical position of caret
static int nCharWidth = 0; // width of a character
static int cch = 0;        // characters in buffer
static int nCurChar = 0;   // index of current character
static PTCHAR pchInputBuf; // input buffer
static int *lineWidth;     // Buffer to store every line's summation of character width
static int lineCount;      // Number of lines
int i, j;                  // loop counters
int cCR = 0;               // count of carriage returns
int nCRIndex = 0;          // index of last carriage return
int nVirtKey;              // virtual-key code
TCHAR szBuf[128];          // temporary buffer
TCHAR ch;                  // current character
PAINTSTRUCT ps;            // required by BeginPaint
SIZE sz;                   // string dimensions
COLORREF crPrevText;       // previous text color
COLORREF crPrevBk;         // previous background color
size_t *pcch;
HRESULT hResult;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EditorWindow(HDC hdc, PAINTSTRUCT &ps);

void MainMenu(HDC hdc, PAINTSTRUCT &ps);
void debug(LPCWSTR text);

LPRECT WindowRect;

RECT EditorRect;
RECT _WindowRect;
RECT NewFileRect;
RECT EditorHeaderRect;
RECT HeaderMenu;
RECT SavedFilesPaintRect;

HWND hwndNewFileButton;
HWND hwndEditorNewFileButton;
HWND hwndEditorRecentFileButton;
HWND hwndEditorThemeButton;
HWND hwndEditorTextFile;
HWND hwnd;
HWND hwndEdit;

HANDLE hFile;

struct FileContent
{
    std::string strTitle;
    std::string strText;
};

TCHAR Content[] = TEXT("Hello World!");

struct FileContent fileContent;

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

    GetClientRect(hwnd, &_WindowRect);

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
    case WM_CREATE:

        /*          RECORDING NECESSARY INFORMATION FOR THE CURRENT FONT          */

        // Get a handler/reference to the current device's grahics information
        // aka get the metrics/information inside the device.
        hdc = GetDC(hwnd);
        // Specifically extract the metrics regarding texts
        GetTextMetrics(hdc, &tm);
        // Free up the handler
        ReleaseDC(hwnd, hdc);

        // Save the average character width and height.
        // For its length
        dwCharX = tm.tmAveCharWidth;
        // For its height
        dwCharY = tm.tmHeight;

        // Allocate a buffer to store keyboard input.
        // aka an array to store the individual characters
        pchInputBuf = (LPTSTR)GlobalAlloc(GPTR,
                                          BUFSIZE * sizeof(TCHAR));
        return 0;

    case WM_SIZE:

        /*          TRACKING THE ROW AND COLUMNS OF CHARACTERS          */

        // Save the new width and height of the client area.
        // The literal area of the screen where a character
        // would be displayed
        dwClientX = LOWORD(lParam);
        dwClientY = HIWORD(lParam);

        // Calculate the maximum width of a line and the
        // maximum number of lines in the client area.
        // Keeping track of how long has the client area has taken
        dwLineLen = dwClientX - dwCharX;
        // Keeping track of the vertical area taken
        dwLines = dwClientY / dwCharY;
        break;

    case WM_SETFOCUS:
        /*          PREPARING THE CARET (CARETS ARE WHERE EACH CHARACTER WILL BE DISPLAYED)          */

        // Create, position, and display the caret when the
        // window receives the keyboard focus.
        CreateCaret(hwnd, (HBITMAP)1, 0, dwCharY);
        SetCaretPos(nCaretPosX, nCaretPosY * dwCharY + HeaderMenu.bottom);
        ShowCaret(hwnd);
        break;

    case WM_KILLFOCUS:
        /*          KILLING THE CARET'S FOCUS ESSENTIALLY HALTING THE DISPLAY OF ANY TEXT INPUT          */

        // Hide and destroy the caret when the window loses the
        // keyboard focus.
        HideCaret(hwnd);
        DestroyCaret();
        break;

    case WM_CHAR:
        /*                           TEXT INPUT                                             */

        // check if current location is close enough to the
        // end of the buffer that a buffer overflow may
        // occur. If so, add null and display contents.
        // I don't know why -5
        if (cch > BUFSIZE - 5)
        {
            // Set the first character to 0
            pchInputBuf[cch] = 0x00;
            // This one is to repaint the window
            // SendMessage(hwnd, WM_PAINT, 0, 0);
        }
        switch (wParam)
        {
        case 0x08: // backspace

            ShowCaret(hwnd);

            // Get the current char
            ch = pchInputBuf[--nCurChar];
            hdc = GetDC(hwnd);
            // Get that current char's width
            GetCharWidth32(hdc, (UINT)ch, (UINT)ch,
                           &nCharWidth);
            // Subtract the width to the carets x-axis position
            nCaretPosX -= nCharWidth;
            // Release the character in the buffer.
            cch--;
            ch = (TCHAR)0x20;
            TextOut(hdc, nCaretPosX, (nCaretPosY * dwCharY) + HeaderMenu.bottom,
                    &ch, 2);
            ReleaseDC(hwnd, hdc);
            
            // If at the far left
            if ((DWORD)nCaretPosX <= 0)
            {
                // Get the current char
                nCaretPosX = pchInputBuf[--nCurChar];
                cch--;
                pchInputBuf[cch] = 0x0D;
                --nCaretPosY;
            }

            nCurChar = cch;
            SetCaretPos(nCaretPosX, nCaretPosY / dwCharY + HeaderMenu.bottom);
        case 0x0A: // linefeed
        case 0x1B: // escape
            MessageBeep((UINT)-1);
            return 0;

        case 0x09: // tab

            // Convert tabs to four consecutive spaces.
            for (i = 0; i < 4; i++)
                // 0x20 so that it goes to the default case which is also
                // a displayable character but is a white-space
                SendMessage(hwnd, WM_CHAR, 0x20, 0);
            return 0;

        case 0x0D: // carriage return

            // Record the carriage return and position the
            // caret at the beginning of the new line.
            pchInputBuf[cch++] = 0x0D; // adds the character code for a carriage return
            nCaretPosX = 0;            // reset x position
            nCaretPosY += 1;           // iterate the y position
            // Record the previous line's length

            break;

        default: // displayable character

            // wParam stores all displayable character codes
            ch = (TCHAR)wParam;
            // Temporarily hide the caret so that it doesn't annoy you
            HideCaret(hwnd);
            // Retrieve the character's width and output
            // the character.

            // Notice these operation has happened before, I guess this is to save
            // resource
            hdc = GetDC(hwnd);
            GetCharWidth32(hdc, (UINT)wParam, (UINT)wParam,
                           &nCharWidth);
            TextOut(hdc, nCaretPosX, (nCaretPosY * dwCharY) + HeaderMenu.bottom,
                    &ch, 1);
            ReleaseDC(hwnd, hdc);

            // Store the character in the buffer.
            pchInputBuf[cch++] = ch;

            // Calculate the new horizontal position of the
            // caret. If the position exceeds the maximum,
            // insert a carriage return and move the caret
            // to the beginning of the next line.
            nCaretPosX += nCharWidth;
            if ((DWORD)nCaretPosX > dwLineLen)
            {
                nCaretPosX = 0;
                pchInputBuf[cch++] = 0x0D;
                ++nCaretPosY;
            }
            nCurChar = cch;
            ShowCaret(hwnd);
            break;
        }
        SetCaretPos(nCaretPosX, nCaretPosY * dwCharY + HeaderMenu.bottom);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT: // LEFT ARROW

            // The caret can move only to the beginning of
            // the current line.

            if (nCaretPosX > 0)
            {
                HideCaret(hwnd);

                // Retrieve the character to the left of
                // the caret, calculate the character's
                // width, then subtract the width from the
                // current horizontal position of the caret
                // to obtain the new position.
                ch = pchInputBuf[--nCurChar];

                hdc = GetDC(hwnd);
                GetCharWidth32(hdc, ch, ch, &nCharWidth);
                ReleaseDC(hwnd, hdc);
                nCaretPosX = std::max(nCaretPosX - nCharWidth,
                                      0);
                ShowCaret(hwnd);
            }
            break;

        case VK_RIGHT: // RIGHT ARROW

            // Caret moves to the right or, when a carriage
            // return is encountered, to the beginning of
            // the next line.

            if (nCurChar < cch)
            {
                HideCaret(hwnd);

                // Retrieve the character to the right of
                // the caret. If it's a carriage return,
                // position the caret at the beginning of
                // the next line.

                ch = pchInputBuf[nCurChar];

                if (ch == 0x0D)
                {
                    nCaretPosX = 0;
                    nCaretPosY++;
                }

                // If the character isn't a carriage
                // return, check to see whether the SHIFT
                // key is down. If it is, invert the text
                // colors and output the character.

                else
                {
                    hdc = GetDC(hwnd);
                    nVirtKey = GetKeyState(VK_SHIFT);
                    if (nVirtKey & SHIFTED)
                    {
                        crPrevText = SetTextColor(hdc,
                                                  RGB(255, 255, 255));
                        crPrevBk = SetBkColor(hdc,
                                              RGB(0, 0, 0));
                        TextOut(hdc, nCaretPosX,
                                nCaretPosY * dwCharY,
                                &ch, 1);
                        SetTextColor(hdc, crPrevText);
                        SetBkColor(hdc, crPrevBk);
                    }

                    // Get the width of the character and
                    // calculate the new horizontal
                    // position of the caret.

                    GetCharWidth32(hdc, ch, ch, &nCharWidth);
                    ReleaseDC(hwnd, hdc);
                    nCaretPosX = nCaretPosX + nCharWidth;
                }
                nCurChar++;
                ShowCaret(hwnd);
                break;
            }
            break;

        case VK_UP:   // UP ARROW
        case VK_DOWN: // DOWN ARROW
            MessageBeep((UINT)-1);
            return 0;

        case VK_HOME: // HOME

            // Set the caret's position to the upper left
            // corner of the client area.
            nCaretPosX = nCaretPosY = 0;
            nCurChar = 0;
            break;

        case VK_END: // END

            // Move the caret to the end of the text.
            for (i = 0; i < cch; i++)
            {
                // Count the carriage returns and save the
                // index of the last one.
                if (pchInputBuf[i] == 0x0D)
                {
                    cCR++;
                    nCRIndex = i + 1;
                }
            }
            nCaretPosY = cCR;

            // Copy all text between the last carriage
            // return and the end of the keyboard input
            // buffer to a temporary buffer.
            for (i = nCRIndex, j = 0; i < cch; i++, j++)
                szBuf[j] = pchInputBuf[i];
            szBuf[j] = TEXT('\0');

            // Retrieve the text extent and use it
            // to set the horizontal position of the
            // caret.
            hdc = GetDC(hwnd);
            hResult = StringCchLength(szBuf, 128, pcch);
            if (FAILED(hResult))
            {
                // TODO: write error handler
            }
            GetTextExtentPoint32(hdc, szBuf, *pcch,
                                 &sz);
            nCaretPosX = sz.cx;
            ReleaseDC(hwnd, hdc);
            nCurChar = cch;
            break;

        default:
            break;
        }
        SetCaretPos(nCaretPosX, nCaretPosY * dwCharY);
        break;

    case WM_PAINT:

        hdc = BeginPaint(hwnd, &ps);
        HideCaret(hwnd);

        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(Theme));

        // State machine
        switch (WindowState)
        {
        case MAINMENU:
            MainMenu(hdc, ps);
            break;
        case EDITOR:
            EditorWindow(hdc, ps);
            break;
        }
        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);

        // Free the input buffer.
        GlobalFree((HGLOBAL)pchInputBuf);
        UnregisterHotKey(hwnd, 0xAAAA);
        break;

    case WM_COMMAND:

        // New File
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case NEWFILE:
                InvalidateRect(hwnd, WindowRect, true);
                ShowWindow(hwndNewFileButton, SW_HIDE);
                WindowState = EDITOR;
                break;
            case OPENFILE:

                break;
            case THEME:
                Theme++;
                if (Theme > NERD_MODE)
                    Theme = LIGHT_MODE;
                WindowState = EDITOR;
                InvalidateRect(hwnd, WindowRect, true);
                //  UpdateWindow(hwnd);
                break;
            }
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void EditorWindow(HDC hdc, PAINTSTRUCT &ps)
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

    // Display the characters
    // Set the clipping rectangle, and then draw the text
    // into it.
    // SetRect(&EditorRect, 0, 300, dwLineLen, dwClientY);
    // DrawText(hdc, pchInputBuf, -1, &EditorRect, DT_LEFT);
    // ShowCaret(hwnd);

    // Open a handle to the file
    hFile = CreateFile(
        L"E:\\Users\\penan\\Downloads\\ATextFile.txt", // Filename
        GENERIC_WRITE,                                 // Desired access
        FILE_SHARE_READ,                               // Share mode
        NULL,                                          // Security attributes
        CREATE_NEW,                                    // Creates a new file, only if it doesn't already exist
        FILE_ATTRIBUTE_NORMAL,                         // Flags and attributes
        NULL);                                         // Template file handle
    if (hFile == INVALID_HANDLE_VALUE)
    {
        WindowState = EDITOR;
        InvalidateRect(hwnd, WindowRect, true);
        ShowWindow(hwndNewFileButton, SW_HIDE);
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
    ShowWindow(hwndEdit, SW_SHOW);

    FillRect(hdc, &HeaderMenu, (HBRUSH)(COLOR_WINDOW + 2));

    hwndEditorTextFile = CreateWindow(
        L"TextEditor",                                                            // Predefined class; Unicode assumed
        L"New file",                                                              // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_SIZEBOX | WS_VSCROLL | WS_BORDER, // Styles
        EditorHeaderRect.right,                                                   // x position
        EditorHeaderRect.top,                                                     // y position
        EditorHeaderRect.left,                                                    // Button width
        EditorHeaderRect.bottom - EditorHeaderRect.top,                           // Button height
        hwnd,                                                                     // Parent window
        (HMENU)NEWFILE,                                                           // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL); // Pointer not needed.

    DWORD bytesWritten;

    WriteFile(
        hFile,                       // Handle to the file
        fileContent.strText.c_str(), // Buffer to write
        fileContent.strText.size(),  // Buffer size
        &bytesWritten,               // Bytes written
        nullptr);                    // Overlapped

    CloseHandle(hFile);
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

void debug(LPCWSTR text)
{
    MessageBox(
        NULL,
        text,
        L"Alert",
        MB_ICONEXCLAMATION);
}