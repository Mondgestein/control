/*

    Windows Control Panel

    Copyright (c) 2024 Ricardo Hanke

    Released under the terms of the GNU General Public License.
    See the file 'COPYING' in the main directory for details.

*/

#include "control.h"
#include "cpl.h"


char szClassName    [] = "CtlPanelClass";
char szConfigSection[] = "mmcpl";
char szConfigFile   [] = "control.ini";
char szHelpFile     [] = "control.hlp";

char szAppName      [256];
char szError        [256];
char szNoApplets    [256];

HWND hwndMainWindow;
HWND hwndAppletList;

typedef struct tagSETTINGS
{
    int x;
    int y;
    int w;
    int h;
} SETTINGS;

SETTINGS Settings;


typedef struct tagAPPLETINFO
{
    HINSTANCE   hInstance;
    APPLET_PROC lpAppletProc;
    LONG        lIndex;
    LONG        lData;
    char        szName[32];
    char        szInfo[64];
} APPLETINFO;

typedef APPLETINFO FAR *LPAPPLETINFO;




int LoadAppletFromFile(LPSTR szFile)
{
    HINSTANCE    hinstApplet;
    APPLET_PROC  lpfnApplet;
    BOOL         bResult;
    LONG         count;
    NEWCPLINFO   CplAppletInfo;
    DWORD        i;
    HGLOBAL      hAppletData;
    LPAPPLETINFO lpAppletData;
    int          index;
    int          numcount = 0;
  


    if ((hinstApplet = LoadLibrary(szFile)) <= HINSTANCE_ERROR)
    {
        return 0;
    }

    if ((lpfnApplet = (APPLET_PROC)GetProcAddress(hinstApplet, "CPlApplet")) == NULL)
    {
        FreeLibrary(hinstApplet);
        return 0;
    }

    if ((bResult = (lpfnApplet)(hwndMainWindow, CPL_INIT, 0, 0)) == FALSE)
    {
        FreeLibrary(hinstApplet);
        return 0;
    }

    if ((count = (lpfnApplet)(hwndMainWindow, CPL_GETCOUNT, 0, 0)) == 0)
    {
        (lpfnApplet)(hwndMainWindow, CPL_EXIT, 0, 0);
        FreeLibrary(hinstApplet);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        CplAppletInfo.dwSize = 0;

        (*lpfnApplet)(hwndMainWindow, CPL_NEWINQUIRE, i, (LPARAM)&CplAppletInfo);
        if (CplAppletInfo.dwSize == sizeof(NEWCPLINFO))
        {
            index = SendMessage(hwndAppletList, LB_ADDSTRING, 0, (LPARAM)CplAppletInfo.szName);

            hAppletData  = GlobalAlloc(GMEM_MOVEABLE, sizeof(APPLETINFO));
            lpAppletData = (void FAR *)GlobalLock(hAppletData);

            _fstrcpy(lpAppletData->szName, CplAppletInfo.szName);
            _fstrcpy(lpAppletData->szInfo, CplAppletInfo.szInfo);

            lpAppletData->hInstance    = hinstApplet;
            lpAppletData->lpAppletProc = lpfnApplet;
            lpAppletData->lIndex       = i;
            lpAppletData->lData        = CplAppletInfo.lData;

            GlobalUnlock(hAppletData);
            SendMessage(hwndAppletList, LB_SETITEMDATA, index, MAKELONG(hAppletData, 0));

            numcount = numcount + 1;
        }
        else MessageBox(0, "CPL_NEWINQUIRE size mismatch.", szFile, MB_OK);
    }

    return numcount;
}



void UpdateSettingsMenu(HMENU hMenu, UINT uPosition)
{
    int count;
    int i;
    int  sel;
    LPAPPLETINFO lpData;
    HGLOBAL hData;
    char buffer[35];


    if ((count = SendMessage(hwndAppletList, LB_GETCOUNT, 0, 0)) == LB_ERR)
        return;


    for (i = 0; i < count; i++)
    {
        hData = SendMessage(hwndAppletList, LB_GETITEMDATA, i, 0);

        lpData = (void FAR *)GlobalLock(hData);

        _fstrcpy(buffer, lpData->szName);
        _fstrcat(buffer, "...");
        InsertMenu(hMenu, uPosition, MF_BYCOMMAND | MF_STRING, IDM_APPLET_FIRST + i, buffer);

        GlobalUnlock(hData);
    }

    if (count > 0)
        InsertMenu(hMenu, uPosition, MF_BYCOMMAND | MF_SEPARATOR, NULL, NULL);

    return;
}



void UnloadApplets(HWND hwndListbox)
{
    LPAPPLETINFO lpData;
    DWORD        count;
    DWORD        i;
    HGLOBAL      hData;


    count = SendMessage(hwndListbox, LB_GETCOUNT, 0, 0);
    
    for (i = count; i > 0; i--)
    {
        hData  = SendMessage(hwndListbox, LB_GETITEMDATA, i - 1, 0);
        lpData = (void FAR *)GlobalLock(hData);

        (lpData->lpAppletProc)(hwndMainWindow, CPL_STOP, i - 1, lpData->lData);

        if (lpData->lIndex == 0)
        {
            (lpData->lpAppletProc)(hwndMainWindow, CPL_EXIT, 0, 0);
            FreeLibrary(lpData->hInstance);
        }

        GlobalUnlock(hData);
        GlobalFree(hData);
        SendMessage(hwndListbox, LB_DELETESTRING, i - 1, 0);
    }

    return;
}



int LoadControlPanelApplets(void)
{
    struct find_t fileblock;
    char szBuffer[160];
    char szSysDir[160];
    int done;
    int index;
    int size;
    int count = 0;


    size = GetSystemDirectory(szSysDir, sizeof(szBuffer));

    if (szSysDir[size] != '\\')
        _fstrcat(szSysDir, "\\");

    sprintf(szBuffer, "%s%s", szSysDir, "*.CPL");

    done = _dos_findfirst(szBuffer, _A_RDONLY + _A_ARCH + _A_HIDDEN + _A_SYSTEM, &fileblock);

    while (!done)
    {
        char temp[160];

        sprintf(temp, "%s%s", szSysDir, fileblock.name);
        count = count + LoadAppletFromFile(temp);
        done = _dos_findnext(&fileblock);
    }

    _dos_findclose(&fileblock);

    return count;
}



void ExecuteApplet(int index)
{
    LPAPPLETINFO lpData;
    HGLOBAL      hData;


    hData  = SendMessage(hwndAppletList, LB_GETITEMDATA, index, 0);
    lpData = (void FAR *)GlobalLock(hData);

    (lpData->lpAppletProc)(hwndMainWindow, CPL_DBLCLK, lpData->lIndex, lpData->lData);

    GlobalUnlock(hData);

    return;
}



LONG __export FAR PASCAL CplWndProc(HWND hwnd, unsigned msg, UINT wparam, LONG lparam)
{
    switch (msg)
    {
        case WM_COMMAND:
        {
            if (HIWORD(lparam) == LBN_DBLCLK)
            {
                ExecuteApplet(SendMessage(hwndAppletList, LB_GETCURSEL, 0, 0));
                return 0;
            }
            else if (LOWORD(wparam) >= IDM_APPLET_FIRST)
            {
                ExecuteApplet(wparam - IDM_APPLET_FIRST);
                return 0;
            }

            switch (LOWORD(wparam))
            {
                case IDM_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0L);
                    break;

                case IDM_HELP_INDEX:
                    if (!WinHelp(hwnd, szHelpFile, HELP_INDEX, 0))
                        MessageBox(hwnd, szError, szAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
                    break;

                case IDM_HELP_SEARCH:
                    if (!WinHelp(hwnd, szHelpFile, HELP_PARTIALKEY, (DWORD)(LPSTR)""))
                        MessageBox(hwnd, szError, szAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
                    break;

                case IDM_HELP_HELPONHELP:
                    if (!WinHelp(hwnd, szHelpFile, HELP_HELPONHELP, 0))
                        MessageBox(hwnd, szError, szAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
                    break;

                case IDM_HELP_ABOUT:
                    ShellAbout(hwnd, szAppName, NULL, NULL);
                    break;
            }
            break;
        }

        case WM_SETFOCUS:
            SetFocus(hwndAppletList);
            break;

        case WM_DESTROY:
            UnloadApplets(hwndAppletList);
            WinHelp(hwnd, szHelpFile, HELP_QUIT, 0);
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            MoveWindow(hwndAppletList, 0, 0, LOWORD(lparam), HIWORD(lparam), TRUE);
            break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}



int PASCAL WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    RECT     rc;
    MSG      msg;


    if (hinstPrevious)
        return 0;

    LoadString(hinstCurrent, IDS_APPNAME,   szAppName,   sizeof(szAppName));
    LoadString(hinstCurrent, IDS_ERROR,     szError,     sizeof(szError));
    LoadString(hinstCurrent, IDS_NOAPPLETS, szNoApplets, sizeof(szNoApplets));


    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = CplWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinstCurrent;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = MAKEINTRESOURCE(MAINMENU);
    wc.lpszClassName = szClassName;

    if (!RegisterClass(&wc))
        return 0;


    Settings.x = GetPrivateProfileInt(szConfigSection, "x", CW_USEDEFAULT, szConfigFile);
    Settings.y = GetPrivateProfileInt(szConfigSection, "y", CW_USEDEFAULT, szConfigFile);
    Settings.w = GetPrivateProfileInt(szConfigSection, "w", 430, szConfigFile);
    Settings.h = GetPrivateProfileInt(szConfigSection, "h", 240, szConfigFile);


    hwndMainWindow = CreateWindow(szClassName,
                                  szAppName,
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX,
                                  Settings.x,
                                  Settings.y,
                                  Settings.w,
                                  Settings.h,
                                  0, 0,
                                  hinstCurrent,
                                  NULL
                                  );

    if (!hwndMainWindow)
        return 0;


    GetClientRect(hwndMainWindow, &rc);
    hwndAppletList = CreateWindow("LISTBOX",
                                  NULL, 
                                  WS_CHILD | WS_VISIBLE | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | LBS_NOTIFY,
                                  0, 0,
                                  rc.right,
                                  rc.bottom,
                                  hwndMainWindow,
                                  NULL,
                                  hinstCurrent,
                                  NULL
                                  );

    if (!hwndAppletList)
        return 0;


    if (LoadControlPanelApplets() == 0)
    {
        MessageBox(hwndMainWindow, szNoApplets, szAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
        return 0;
    }

    UpdateSettingsMenu(GetSubMenu(GetMenu(hwndMainWindow), 0), IDM_FILE_EXIT);


    ShowWindow(hwndMainWindow, nCmdShow);
    UpdateWindow(hwndMainWindow);


    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

