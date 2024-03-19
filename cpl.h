/*

    Windows Control Panel

    Copyright (c) 2024 Ricardo Hanke

    Released under the terms of the GNU General Public License.
    See the file 'COPYING' in the main directory for details.

*/


#define WM_CPL_LAUNCH   (WM_USER + 1000)
#define WM_CPL_LAUNCHED (WM_USER + 1001)

typedef LRESULT(CALLBACK *APPLET_PROC)(HWND hwnd, UINT msg, LPARAM param1, LPARAM param2);


typedef struct tagCPLINFO
{
    int  idIcon;
    int  idName;
    int  idInfo;
    LONG lData;
} CPLINFO, *PCPLINFO, FAR *LPCPLINFO;

typedef struct tagNEWCPLINFO
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHelpContext;
    LONG  lData;
    HICON hIcon;
    char  szName[32];
    char  szInfo[64];
    char  szHelpFile[128];
} NEWCPLINFO, *PNEWCPLINFO, FAR *LPNEWCPLINFO;


#define CPL_INIT        1
#define CPL_GETCOUNT    2
#define CPL_INQUIRE     3
#define CPL_SELECT      4
#define CPL_DBLCLK      5
#define CPL_STOP        6
#define CPL_EXIT        7
#define CPL_NEWINQUIRE  8

