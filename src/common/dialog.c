/******************************Module*Header*******************************\
* Module Name: dialog.c
*
* Dialog helper functions
*
* Copyright (c) 1995 Microsoft Corporation
*
* Settings persistence: Originally used WritePrivateProfileString / GetPrivateProfileInt
* with "%WINDIR%\control.ini", which silently fails on Windows Vista+ without
* administrator rights.  Replaced with proper Windows Registry (HKCU) storage.
*
\**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "sscommon.h"

#define BUF_SIZE 64
static TCHAR  szSectName[BUF_SIZE];  // e.g. "Screen Saver.3DPipes"
static TCHAR  szItemName[BUF_SIZE];
static HINSTANCE hInstance = 0;

// Registry base path; szSectName is appended as the subkey name.
#define REGISTRY_BASE TEXT("Software\\Microsoft\\ScreenSavers\\")

/******************************Public*Routine******************************\
* OpenRegKey
*
* Opens (or creates for writing) the screensaver settings key in HKCU.
* Caller must close the returned handle with RegCloseKey().
* Returns NULL on failure.
\**************************************************************************/

static HKEY OpenRegKey( BOOL forWrite )
{
    TCHAR keyPath[128];
    HKEY hKey = NULL;
    LONG res;

    wsprintf( keyPath, TEXT("%s%s"), REGISTRY_BASE, szSectName );

    if( forWrite ) {
        DWORD disp;
        res = RegCreateKeyEx( HKEY_CURRENT_USER, keyPath, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                              NULL, &hKey, &disp );
    } else {
        res = RegOpenKeyEx( HKEY_CURRENT_USER, keyPath, 0,
                            KEY_QUERY_VALUE, &hKey );
    }

    return (res == ERROR_SUCCESS) ? hKey : NULL;
}

/******************************Public*Routine******************************\
* ss_RegistrySetup
*
* Setup for registry access — loads the section name from resources.
\**************************************************************************/

BOOL ss_RegistrySetup( HINSTANCE hinst, int section, int file )
{
    // 'file' parameter was for the old INI approach; ignored now.
    hInstance = hinst;
    return LoadString( hInstance, section, szSectName, BUF_SIZE ) != 0;
}

/******************************Public*Routine******************************\
* ss_GetRegistryInt
\**************************************************************************/

int ss_GetRegistryInt( int name, int iDefault )
{
    HKEY hKey;
    DWORD type, value, size = sizeof(DWORD);

    if( !LoadString( hInstance, name, szItemName, BUF_SIZE ) )
        return iDefault;

    hKey = OpenRegKey( FALSE );
    if( !hKey )
        return iDefault;

    if( RegQueryValueEx( hKey, szItemName, NULL, &type,
                         (BYTE *)&value, &size ) == ERROR_SUCCESS
        && type == REG_DWORD ) {
        RegCloseKey( hKey );
        return (int)value;
    }

    RegCloseKey( hKey );
    return iDefault;
}

/******************************Public*Routine******************************\
* ss_GetRegistryString
\**************************************************************************/

void ss_GetRegistryString( int name, LPTSTR lpDefault, LPTSTR lpDest,
                            int bufSize )
{
    HKEY hKey;
    DWORD type;
    DWORD size = (DWORD)(bufSize * sizeof(TCHAR));

    if( lpDefault )
        lstrcpyn( lpDest, lpDefault, bufSize );
    else if( bufSize > 0 )
        lpDest[0] = TEXT('\0');

    if( !LoadString( hInstance, name, szItemName, BUF_SIZE ) )
        return;

    hKey = OpenRegKey( FALSE );
    if( !hKey )
        return;

    RegQueryValueEx( hKey, szItemName, NULL, &type, (BYTE *)lpDest, &size );
    RegCloseKey( hKey );
}

/******************************Public*Routine******************************\
* ss_WriteRegistryInt
\**************************************************************************/

void ss_WriteRegistryInt( int name, int iVal )
{
    HKEY hKey;
    DWORD value = (DWORD)iVal;

    if( !LoadString( hInstance, name, szItemName, BUF_SIZE ) )
        return;

    hKey = OpenRegKey( TRUE );
    if( !hKey )
        return;

    RegSetValueEx( hKey, szItemName, 0, REG_DWORD,
                   (BYTE *)&value, sizeof(DWORD) );
    RegCloseKey( hKey );
}

/******************************Public*Routine******************************\
* ss_WriteRegistryString
\**************************************************************************/

void ss_WriteRegistryString( int name, LPTSTR lpString )
{
    HKEY hKey;

    if( !lpString )
        return;
    if( !LoadString( hInstance, name, szItemName, BUF_SIZE ) )
        return;

    hKey = OpenRegKey( TRUE );
    if( !hKey )
        return;

    RegSetValueEx( hKey, szItemName, 0, REG_SZ, (BYTE *)lpString,
                   (DWORD)((lstrlen(lpString) + 1) * sizeof(TCHAR)) );
    RegCloseKey( hKey );
}

/******************************Public*Routine******************************\
* ss_GetTrackbarPos
\**************************************************************************/

int ss_GetTrackbarPos( HWND hDlg, int item )
{
    return (int)SendDlgItemMessage( hDlg, item, TBM_GETPOS, 0, 0 );
}

/******************************Public*Routine******************************\
* ss_SetupTrackbar
\**************************************************************************/

void ss_SetupTrackbar( HWND hDlg, int item, int lo, int hi, int lineSize,
                       int pageSize, int pos )
{
    SendDlgItemMessage( hDlg, item, TBM_SETRANGE,
                        (WPARAM)TRUE, (LPARAM)MAKELONG(lo, hi) );
    SendDlgItemMessage( hDlg, item, TBM_SETPOS,
                        (WPARAM)TRUE, (LPARAM)pos );
    SendDlgItemMessage( hDlg, item, TBM_SETPAGESIZE,
                        (WPARAM)0, (LPARAM)pageSize );
    SendDlgItemMessage( hDlg, item, TBM_SETLINESIZE,
                        (WPARAM)0, (LPARAM)lineSize );
}
