/******************************Module*Header*******************************\
* Module Name: sscommon.hxx
*
* Defines and externals for screen saver common shell
*
* Copyright (c) 1996 Microsoft Corporation
*
\**************************************************************************/

#ifndef __clear_hxx__
#define __clear_hxx__

#include "sscommon.h"


class SS_DIGITAL_DISSOLVE_CLEAR {
public:
    SS_DIGITAL_DISSOLVE_CLEAR();
    ~SS_DIGITAL_DISSOLVE_CLEAR();
    int  CalibrateClear( int width, int height, float fClearTime );
    BOOL Clear( int width, int height, int size );
    BOOL Clear( int width, int height );
    void StartClear( int width, int height );
    BOOL ContinueClear();
private:
    BOOL *rectBuf;
    int  rectBufSize;
    int  rectSize;
    BOOL ValidateBufSize( int nRects );
    int  RectangleCount( int width, int height, int size );

    int  *orderBuf;      // pre-shuffled dissolve order (rect indices)
    int  orderBufSize;   // allocated size of orderBuf
    int  dissolveCount;  // number of rects cleared so far
    int  dissolveTotal;  // total rects needed for full dissolve
    int  dissolveStep;   // rects to add per ContinueClear call (~1/30 of total)
    int  dissolveXdim;   // x-dimension of the rect grid
    int  dissolveSize;   // rect pixel size used for this dissolve
    BOOL ValidateOrderBufSize( int nRects );
};

extern void DrawGdiRect( HDC hdc, HBRUSH hbr, RECT *pRect );
extern void ss_GdiRectWipeClear( HWND, int, int);

#endif // __clear_hxx__
