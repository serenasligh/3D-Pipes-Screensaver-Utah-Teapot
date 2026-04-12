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
#include "util.hxx"


class SS_DIGITAL_DISSOLVE_CLEAR {
public:
    SS_DIGITAL_DISSOLVE_CLEAR();
    ~SS_DIGITAL_DISSOLVE_CLEAR();
    int  CalibrateClear( int width, int height, float fClearTime );
    BOOL Clear( int width, int height, int size );
    BOOL Clear( int width, int height );
    // dissolveTime: target duration in seconds (0 = instant, skipped by Draw)
    // manualRectSize: 0 = use auto-calibrated rectSize (smooth), >0 = explicit
    //                 pixel block size (pixelated), e.g. 8 means 8x8 blocks
    void StartClear( int width, int height, float dissolveTime, int manualRectSize = 0 );
    BOOL ContinueClear();
private:
    BOOL *rectBuf;
    int  rectBufSize;
    int  rectSize;
    BOOL ValidateBufSize( int nRects );
    int  RectangleCount( int width, int height, int size );

    int  *orderBuf;          // pre-shuffled dissolve order (rect indices)
    int  orderBufSize;       // allocated size of orderBuf
    int  *recheckBuf;        // rects cleared last frame needing one more clear (double-buffer)
    int  recheckBufSize;     // allocated size of recheckBuf
    int  recheckCount;       // number of rects queued for re-clear this frame
    int  dissolveCount;      // number of rects cleared so far
    int  dissolveTotal;      // total rects needed for full dissolve
    int  dissolveXdim;       // x-dimension of the rect grid
    int  dissolveSize;       // rect pixel size used for this dissolve
    float dissolveDuration;  // target dissolve time in seconds
    SS_TIMER dissolveTimer;  // tracks elapsed time for progress calculation
    BOOL ValidateOrderBufSize( int nRects );
    BOOL ValidateRecheckBufSize( int nRects );
};

extern void DrawGdiRect( HDC hdc, HBRUSH hbr, RECT *pRect );
extern void ss_GdiRectWipeClear( HWND, int, int);

#endif // __clear_hxx__
