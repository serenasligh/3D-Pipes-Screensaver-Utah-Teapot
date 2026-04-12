/******************************Module*Header*******************************\
* Module Name: sspipes.c
*
* Message loop and dialog box for the OpenGL-based 3D Pipes screen saver.
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#include <windows.h>
#include <commdlg.h>
#include <scrnsave.h>
#include <GL/gl.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/timeb.h>
#include <time.h>
#include <commctrl.h>
#include "sscommon.h"
#include "sspipes.h"
#include "dlgs.h"
#include "dialog.h"

BOOL bFlexMode;
BOOL bMultiPipes;
BOOL bTeapotEnabled = TRUE;  // TRUE = teapots can appear; FALSE = never
int  iTeapotOdds = 1000;  // 1/N chance per joint; 1=always, 2000=very rare
int  iPipeSpeed  = 50;    // 1=slowest ... 100=fastest; 50=default (1 seg/frame)
int  iDissolveTime  = 20; // 0=instant, 80=8.0s; default 20 (=2.0s)
BOOL bDissolveSmooth = TRUE;  // TRUE=smooth (auto-calibrated), FALSE=pixelated
int  iDissolveRectLog = 3;   // log2 block size for pixelated: 0=1px,7=128px; default 3=8px
BOOL bAlternateMode = FALSE;  // TRUE = randomly alternate Default/Flex each frame reset
int  iSwitchOdds = 2;         // 1-in-N chance of mode switch per frame; default 2 (50%)

// ulJointType controls the style of the elbows.

ULONG ulJointType = JOINT_ELBOW;

// ulSurfStyle determines whether the pipe surfaces are textured.

ULONG ulSurfStyle = SURFSTYLE_SOLID;

// ulTexQuality control the texture quality.

ULONG ulTexQuality = TEXQUAL_DEFAULT;

// fTesselFact controls the how finely the surface is tesselated.  It
// varies from very course (0.0) to very fine (2.0).

float fTesselFact = 1.0f;

// If ulSurfStyle indicates a textured surface, szTexPathname specifies
// the bitmap chosen as the texture.

// Texture file(s)
TEXFILE gTexFile[MAX_TEXTURES] = {0};
int gnTextures = 0;

static void updateDialogControls(HWND hDlg);

/******************************Public*Routine******************************\
* getIniSettings
*
* Get the screen saver configuration options from .INI file/registry.
*
\**************************************************************************/

void 
getIniSettings()
{
    int    tessel;

    // Load resources

    LoadString(hMainInstance, IDS_GENNAME, szScreenSaver, 
               sizeof(szScreenSaver) / sizeof(TCHAR));

    // Load resource strings for texture processing

    ss_LoadTextureResourceStrings();

    // Get registry settings

    if( ss_RegistrySetup( hMainInstance, IDS_SAVERNAME, IDS_INIFILE ) )
    {
        ulJointType = ss_GetRegistryInt( IDS_JOINTTYPE, JOINT_ELBOW );

        ulSurfStyle = ss_GetRegistryInt( IDS_SURFSTYLE, SURFSTYLE_SOLID );

        ulTexQuality = ss_GetRegistryInt( IDS_TEXQUAL, TEXQUAL_DEFAULT );

        tessel = ss_GetRegistryInt( IDS_TESSELATION, 0 );
        SS_CLAMP_TO_RANGE2( tessel, 0, 200 );
        fTesselFact  = (float)tessel / 100.0f;

        bFlexMode = ss_GetRegistryInt( IDS_FLEX, 0 );

        bMultiPipes = ss_GetRegistryInt( IDS_MULTIPIPES, 0 );

        bTeapotEnabled = ss_GetRegistryInt( IDS_TEAPOTENABLED, 1 );

        iTeapotOdds = ss_GetRegistryInt( IDS_TEAPOTODDS, 1000 );
        SS_CLAMP_TO_RANGE2( iTeapotOdds, 1, 2000 );

        iPipeSpeed = ss_GetRegistryInt( IDS_PIPESPEED, 50 );
        SS_CLAMP_TO_RANGE2( iPipeSpeed, 1, 100 );

        iDissolveTime = ss_GetRegistryInt( IDS_DISSOLVETIME, 20 );
        SS_CLAMP_TO_RANGE2( iDissolveTime, 0, 80 );

        bDissolveSmooth = ss_GetRegistryInt( IDS_DISSOLVESMOOTH, 1 );

        iDissolveRectLog = ss_GetRegistryInt( IDS_DISSOLVERECT, 3 );
        SS_CLAMP_TO_RANGE2( iDissolveRectLog, 0, 7 );

        bAlternateMode = ss_GetRegistryInt( IDS_ALTERNATEMODE, 0 );

        iSwitchOdds = ss_GetRegistryInt( IDS_SWITCHODDS, 2 );
        SS_CLAMP_TO_RANGE2( iSwitchOdds, 1, 5 );

        // Get any textures

#ifndef NEW_TEXTURE
        // Just get one texture with old registry names
        ss_GetRegistryString( IDS_TEXTURE, 0, gTexFile[0].szPathName, MAX_PATH);
        gTexFile[0].nOffset = ss_GetRegistryInt( IDS_TEXTURE_FILE_OFFSET, 0 );
        gnTextures = 1;
#else
        gnTextures = ss_GetRegistryInt( IDS_TEXTURE_COUNT, 0 );
        SS_CLAMP_TO_RANGE2( gnTextures, 0, MAX_TEXTURES );

        idsTexture = IDS_TEXTURE0;
        idsTexOffset = IDS_TEXOFFSET0;
        for( i = 0; i < gnTextures; i++, idsTexture++, idsTexOffset++ ) {
            ss_GetRegistryString( idsTexture, 0, gTexFile[i].szPathName,
                                  MAX_PATH);
            gTexFile[i].nOffset = ss_GetRegistryInt( idsTexOffset, 0 );
        }
#endif
    }
}

/**************************************************************************\
* ConfigInit
*
\**************************************************************************/
BOOL
ss_ConfigInit( HWND hDlg )
{
    return TRUE;
}

/******************************Public*Routine******************************\
* saveIniSettings
*
* Save the screen saver configuration option to the .INI file/registry.
*
\**************************************************************************/

static void saveIniSettings(HWND hDlg)
{
    if( ss_RegistrySetup( hMainInstance, IDS_SAVERNAME, IDS_INIFILE ) )
    {
        ss_WriteRegistryInt( IDS_JOINTTYPE, ulJointType );
        ss_WriteRegistryInt( IDS_SURFSTYLE, ulSurfStyle );
        ss_WriteRegistryInt( IDS_TEXQUAL, ulTexQuality );
        ss_WriteRegistryInt( IDS_TESSELATION, 
                    ss_GetTrackbarPos(hDlg, DLG_SETUP_TESSEL) );
        ss_WriteRegistryInt( IDS_FLEX, bFlexMode );
        ss_WriteRegistryInt( IDS_MULTIPIPES, bMultiPipes );
        ss_WriteRegistryInt( IDS_TEAPOTENABLED, bTeapotEnabled );
        ss_WriteRegistryInt( IDS_TEAPOTODDS,
                    ss_GetTrackbarPos(hDlg, IDC_SLIDER_TEAPOT_ODDS) );
        ss_WriteRegistryInt( IDS_PIPESPEED,
                    ss_GetTrackbarPos(hDlg, IDC_SLIDER_SPEED) );
        ss_WriteRegistryInt( IDS_DISSOLVETIME,
                    ss_GetTrackbarPos(hDlg, IDC_SLIDER_DISSOLVE_TIME) );
        ss_WriteRegistryInt( IDS_DISSOLVESMOOTH, bDissolveSmooth );
        ss_WriteRegistryInt( IDS_DISSOLVERECT,
                    ss_GetTrackbarPos(hDlg, IDC_SLIDER_DISSOLVE_RES) );
        ss_WriteRegistryInt( IDS_ALTERNATEMODE, bAlternateMode );
        ss_WriteRegistryInt( IDS_SWITCHODDS, iSwitchOdds );
#ifndef NEW_TEXTURE
        ss_WriteRegistryString( IDS_TEXTURE, gTexFile[0].szPathName );
        ss_WriteRegistryInt( IDS_TEXTURE_FILE_OFFSET, gTexFile[0].nOffset );
#else
        idsTexture = IDS_TEXTURE0;
        idsTexOffset = IDS_TEXOFFSET0;
        for( i = 0; i < gnTextures; i++, idsTexture++, idsTexOffset++ ) {
            ss_WriteRegistryString( idsTexture, gTexFile[i].szPathName );
            ss_WriteRegistryInt( idsTexOffset, gTexFile[i].nOffset );
        }
#endif
    }
}

/******************************Public*Routine******************************\
* setupDialogControls
*
* Do initial setup of dialog controls.
\**************************************************************************/

static void 
setupDialogControls(HWND hDlg)
{
    int pos;
    int wTmp;
    TCHAR szStr[GEN_STRING_SIZE];
    int idsJointType;

    InitCommonControls();

    pos = (int)(fTesselFact * 100.0f);
    ss_SetupTrackbar( hDlg, DLG_SETUP_TESSEL, 0, 200, 1, 10, pos );

    ss_SetupTrackbar( hDlg, IDC_SLIDER_TEAPOT_ODDS, 1, 2000, 1, 100,
                      iTeapotOdds );
    ss_SetupTrackbar( hDlg, IDC_SLIDER_SPEED, 1, 100, 1, 5,
                      iPipeSpeed );

    // Dissolve sliders: time = 0..80 (→ 0.0..8.0s), res = 0..7 (log2 block size)
    ss_SetupTrackbar( hDlg, IDC_SLIDER_DISSOLVE_TIME, 0, 80, 1, 5,
                      iDissolveTime );
    ss_SetupTrackbar( hDlg, IDC_SLIDER_DISSOLVE_RES, 0, 7, 1, 1,
                      iDissolveRectLog );

    // setup jointType combo box
    idsJointType = IDS_JOINT_ELBOW;
    for (wTmp = 0; wTmp < NUM_JOINTTYPES; wTmp++, idsJointType++) {
        LoadString(hMainInstance, idsJointType, szStr,
                    GEN_STRING_SIZE);
        SendDlgItemMessage(hDlg, DLG_COMBO_JOINTTYPE, CB_ADDSTRING, 0,
                           (LPARAM) szStr);
    }
    SendDlgItemMessage(hDlg, DLG_COMBO_JOINTTYPE, CB_SETCURSEL,
                       ulJointType, 0);

    // Pipe Style: Default/Flex radios + Incl. Teapot + Alternate checkboxes
    CheckDlgButton( hDlg, IDC_RADIO_NORMAL,     !bAlternateMode && !bFlexMode );
    CheckDlgButton( hDlg, IDC_RADIO_FLEX,       !bAlternateMode && bFlexMode );
    CheckDlgButton( hDlg, IDC_CHECK_TEAPOT,     bTeapotEnabled );
    CheckDlgButton( hDlg, IDC_CHECK_ALTERNATE,  bAlternateMode );

    // Switch Odds combo: "1 in 1" through "1 in 5"
    {
        TCHAR szBuf[32];
        int k;
        for( k = 1; k <= 5; k++ ) {
            wsprintf( szBuf, "1 in %d", k );
            SendDlgItemMessage( hDlg, IDC_COMBO_SWITCHODDS, CB_ADDSTRING, 0,
                                (LPARAM) szBuf );
        }
        SendDlgItemMessage( hDlg, IDC_COMBO_SWITCHODDS, CB_SETCURSEL,
                            iSwitchOdds - 1, 0 );
    }

    // Dissolve style radios
    CheckDlgButton( hDlg, IDC_RADIO_DISSOLVE_SMOOTH,    bDissolveSmooth );
    CheckDlgButton( hDlg, IDC_RADIO_DISSOLVE_PIXELATED, !bDissolveSmooth );

    updateDialogControls( hDlg );
}

/******************************Public*Routine******************************\
* updateDialogControls
*
* Setup the dialog controls based on the current global state.
*
\**************************************************************************/

static void updateDialogControls(HWND hDlg)
{
    // Surface style flags
    BOOL bSolidActive = (ulSurfStyle == SURFSTYLE_SOLID || ulSurfStyle == SURFSTYLE_TRANS);
    BOOL bTexture     = (ulSurfStyle == SURFSTYLE_TEX);
    // Pixelated dissolve resolution slider active only in pixelated mode
    BOOL bPixelated   = !bDissolveSmooth;
    // Incl. Teapot checkbox enabled unless Flex-only (not alternate) is active
    BOOL bTeapotCtrlEnabled = !bFlexMode || bAlternateMode;
    // Teapot frequency group enabled when teapot is enabled
    BOOL bTeapotActive = bTeapotEnabled;
    // Joint type enabled in non-Flex-only mode
    BOOL bJointEnabled = !bFlexMode || bAlternateMode;

    // Surface style radios + :3 checkbox
    CheckDlgButton( hDlg, IDC_RADIO_SOLID, bSolidActive );
    CheckDlgButton( hDlg, IDC_RADIO_TEX,   bTexture );
    CheckDlgButton( hDlg, IDC_CHECK_TRANS, ulSurfStyle == SURFSTYLE_TRANS );
    // :3 checkbox: enabled only when Solid surface is selected
    EnableWindow( GetDlgItem(hDlg, IDC_CHECK_TRANS), bSolidActive );

    // Pipe style radios and checkboxes
    CheckDlgButton( hDlg, IDC_RADIO_NORMAL,     !bAlternateMode && !bFlexMode );
    CheckDlgButton( hDlg, IDC_RADIO_FLEX,       !bAlternateMode && bFlexMode );
    CheckDlgButton( hDlg, IDC_CHECK_TEAPOT,     bTeapotEnabled );
    CheckDlgButton( hDlg, IDC_CHECK_ALTERNATE,  bAlternateMode );
    // Default/Flex radios disabled when Alternate mode is on
    EnableWindow( GetDlgItem(hDlg, IDC_RADIO_NORMAL), !bAlternateMode );
    EnableWindow( GetDlgItem(hDlg, IDC_RADIO_FLEX),   !bAlternateMode );
    // Incl. Teapot: grayed when Flex-only (not when alternate which uses both)
    EnableWindow( GetDlgItem(hDlg, IDC_CHECK_TEAPOT), bTeapotCtrlEnabled );
    // Switch Odds combo: only meaningful when Alternate is on
    EnableWindow( GetDlgItem(hDlg, IDC_COMBO_SWITCHODDS), bAlternateMode );

    CheckDlgButton( hDlg, IDC_RADIO_SINGLE_PIPE,    !bMultiPipes );
    CheckDlgButton( hDlg, IDC_RADIO_MULTIPLE_PIPES, bMultiPipes );

    // Joint type combo: disabled when Flex-only (no joint style in flex)
    EnableWindow( GetDlgItem(hDlg, DLG_COMBO_JOINTTYPE),  bJointEnabled );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_JOINTTYPE), bJointEnabled );

    // "Choose Texture" button: only when Textured is selected
    EnableWindow( GetDlgItem(hDlg, DLG_SETUP_TEXTURE), bTexture );

    // Teapot frequency group: enabled when Incl. Teapot checkbox is on
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TEAPOT_GRP),  bTeapotActive );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TEAPOT_MIN),  bTeapotActive );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TEAPOT_MAX),  bTeapotActive );
    EnableWindow( GetDlgItem(hDlg, IDC_SLIDER_TEAPOT_ODDS), bTeapotActive );

    // Resolution slider: only when Pixelated dissolve style is active
    EnableWindow( GetDlgItem(hDlg, IDC_SLIDER_DISSOLVE_RES),     bPixelated );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_DISSOLVE_RES_MIN), bPixelated );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_DISSOLVE_RES_MAX), bPixelated );

    EnableWindow( GetDlgItem(hDlg, DLG_SETUP_TESSEL),     TRUE );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TESS_MIN),  TRUE );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TESS_MAX),  TRUE );
    EnableWindow( GetDlgItem(hDlg, IDC_STATIC_TESS_GRP),  TRUE );
}

BOOL WINAPI RegisterDialogClasses(HANDLE hinst)
{
    return TRUE;
}


/******************************Public*Routine******************************\
* ScreenSaverConfigureDialog
*
* Screen saver setup dialog box procedure.
\**************************************************************************/

BOOL ScreenSaverConfigureDialog(HWND hDlg, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
    int optMask = 1;

    switch (message)
    {
        case WM_INITDIALOG:
            getIniSettings();
            setupDialogControls(hDlg);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO_SOLID:
                    // Keep trans mode if :3 checkbox is currently checked
                    if( IsDlgButtonChecked(hDlg, IDC_CHECK_TRANS) == BST_CHECKED )
                        ulSurfStyle = SURFSTYLE_TRANS;
                    else
                        ulSurfStyle = SURFSTYLE_SOLID;
                    break;
                case IDC_RADIO_TEX:
                    ulSurfStyle = SURFSTYLE_TEX;
                    break;
                case IDC_RADIO_WIREFRAME:
                    ulSurfStyle = SURFSTYLE_WIREFRAME;
                    break;

                case IDC_CHECK_TRANS:
                    // Toggle :3 (trans flag texture) on solid pipes
                    if( IsDlgButtonChecked(hDlg, IDC_CHECK_TRANS) == BST_CHECKED )
                        ulSurfStyle = SURFSTYLE_TRANS;
                    else
                        ulSurfStyle = SURFSTYLE_SOLID;
                    break;

                case IDC_RADIO_TEXQUAL_DEFAULT:
                case IDC_RADIO_TEXQUAL_HIGH:
                    ulTexQuality = IDC_TO_TEXQUAL(LOWORD(wParam));
                    break;

                case IDC_RADIO_NORMAL:
                    bFlexMode = FALSE;
                    break;
                case IDC_RADIO_FLEX:
                    bFlexMode = TRUE;
                    break;

                case IDC_CHECK_TEAPOT:
                    bTeapotEnabled = (IsDlgButtonChecked(hDlg, IDC_CHECK_TEAPOT) == BST_CHECKED);
                    break;

                case IDC_CHECK_ALTERNATE:
                    bAlternateMode = (IsDlgButtonChecked(hDlg, IDC_CHECK_ALTERNATE) == BST_CHECKED);
                    break;

                case IDC_RADIO_SINGLE_PIPE:
                    bMultiPipes = FALSE;
                    break;
                case IDC_RADIO_MULTIPLE_PIPES:
                    bMultiPipes = TRUE;
                    break;

                case IDC_RADIO_DISSOLVE_SMOOTH:
                    bDissolveSmooth = TRUE;
                    break;
                case IDC_RADIO_DISSOLVE_PIXELATED:
                    bDissolveSmooth = FALSE;
                    break;

                case IDC_COMBO_SWITCHODDS:
                    switch( HIWORD(wParam) )
                    {
                        case CBN_SELCHANGE:
                            iSwitchOdds = (int)SendDlgItemMessage(
                                hDlg, IDC_COMBO_SWITCHODDS, CB_GETCURSEL, 0, 0) + 1;
                            break;
                        default:
                            break;
                    }
                    break;

                case DLG_SETUP_TEXTURE:
                    // Run choose texture dialog
                    ss_SelectTextureFile( hDlg, &gTexFile[0] );
                    break;

                case DLG_COMBO_JOINTTYPE:
                    switch (HIWORD(wParam))
                    {
                        case CBN_EDITCHANGE:
                        case CBN_SELCHANGE:
                            ulJointType = 
                                SendDlgItemMessage(hDlg, DLG_COMBO_JOINTTYPE,
                                                      CB_GETCURSEL, 0, 0);
                            break;
                        default:
                            return FALSE;
                    }
                    break;

                case IDOK:
                    saveIniSettings(hDlg);
                    EndDialog(hDlg, TRUE);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    break;
            }
            updateDialogControls(hDlg);
            return TRUE;

        default:
            return 0;
    }
    return 0;
}
