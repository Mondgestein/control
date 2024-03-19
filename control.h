/*

    Windows Control Panel

    Copyright (c) 2024 Ricardo Hanke

    Released under the terms of the GNU General Public License.
    See the file 'COPYING' in the main directory for details.

*/

#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>


#define MAINMENU             1

void WINAPI ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

#define IDM_APPLET_FIRST     1000


#define IDS_APPNAME          1
#define IDS_ERROR            2
#define IDS_NOAPPLETS        3

#define IDM_FILE_EXIT        100
#define IDM_HELP_INDEX       101
#define IDM_HELP_ABOUT       102
#define IDM_HELP_SEARCH      103
#define IDM_HELP_HELPONHELP  104

