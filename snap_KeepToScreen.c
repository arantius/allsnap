#include "stdafx.h"
#include "snap_WinRects.h"
#include "snap_TestInfo.h"
#include "snap_CornerInfo.h"
#include "snap_InTests.h"
#include "snap_lib_internal.h"
#include <crtdbg.h>


void testSingleCorner(SNAP_RESULTS * p_snap_results,enum SIDES h_side,enum SIDES v_side,enum SIDES ignored_side,LPCRECT pRect);
BOOL isWholeSideOut_Sizing(enum SIDES side,TEST_INFO * p_test_info, LPRECT pScreen, P_CORNER_INFO pci);
BOOL isWholeSideOut_Moving(enum SIDES side,TEST_INFO * p_test_info, LPCRECT pRect,LPRECT pScreen,P_CORNER_INFO pci);
BOOL isWholeSideOut(enum SIDES side,TEST_INFO * p_test_info, LPCRECT pRect,LPRECT pScreen,P_CORNER_INFO pci);
BOOL testWholeDimension(enum SIDES first_side,
						enum SIDES second_side,
						TEST_INFO * p_test_info,
						LPCRECT pRect,
						LPRECT pScreen,
						P_CORNER_INFO pci,
						enum SIDES * p_kept_side);
void KeepTopLeftOnScreen(SNAP_RESULTS * p_sr, LPCRECT pRect);


static void OverwriteResult(SNAP_RESULTS * p_snap_results,enum SIDES which_side,LPCRECT pScreen){
	SIDE_SNAP_RESULTS  * p_side_r = (isVerticalSide(which_side))?
		&(p_snap_results->v):&(p_snap_results->h);

	p_side_r->closeness =1;
	p_side_r->side	   = which_side;
	p_side_r->to_side  = which_side;
	p_side_r->value	   = GetSideOfRect(which_side,pScreen);

}

BOOL isWholeSideOut_Sizing(enum SIDES side,TEST_INFO * p_test_info, LPRECT pScreen, P_CORNER_INFO pci){
	LPCRECT pRect = p_test_info->mine;
	if (!isSideIn(pci,side)){

		WinRects_getCurrentScreen(pScreen);
		
		return(  ( isVerticalSide(side)? (p_test_info->v_sizing_side == side):(p_test_info->h_sizing_side== side))
			&& ( isLargerSide(side)?	(GetSideOfRect(side,pRect) > GetSideOfRect(side,pScreen ))
									:	(GetSideOfRect(side,pRect) < GetSideOfRect(side,pScreen))));
	}
	else{
		return FALSE;
	}
}

BOOL isWholeSideOut_Moving(enum SIDES side,TEST_INFO * p_test_info, LPCRECT pRect,LPRECT pScreen,P_CORNER_INFO pci){
	if (isSideOut(pci,side)){

		InTests_GetClosestScreenToSide(side,pRect,pScreen);

		return ( isLargerSide(side)?	(GetSideOfRect(side,pRect) > GetSideOfRect(side,pScreen))
									:	(GetSideOfRect(side,pRect) < GetSideOfRect(side,pScreen)));
	}
	else{
		return FALSE;
	}
}


BOOL isWholeSideOut(enum SIDES side,TEST_INFO * p_test_info, LPCRECT pRect,LPRECT pScreen,P_CORNER_INFO pci){
	if (p_test_info->sizing){
		return isWholeSideOut_Sizing(side,p_test_info,pScreen,pci);
	}
	else{
		return isWholeSideOut_Moving(side,p_test_info,pRect,pScreen,pci);
	}
}

BOOL testWholeDimension(enum SIDES first_side,
						enum SIDES second_side,
						TEST_INFO * p_test_info,
						LPCRECT pRect,
						LPRECT pScreen,
						P_CORNER_INFO pci,
						enum SIDES * p_kept_side)
{
	if (isWholeSideOut(first_side,p_test_info,pRect,pScreen,pci)){
		*p_kept_side = first_side;
		return TRUE;
	}
	else if (isWholeSideOut(second_side,p_test_info,pRect,pScreen,pci)){
		*p_kept_side = second_side;
		return TRUE;
	}
	else{
		*p_kept_side = SIDE_NONE;
		return FALSE;
	}
}


//a function to call at the end of testing to limit a window to
//it's initial screen by modifying the snap results.
void KeepToScreen( 
		TEST_INFO * p_test_info, 
		SNAP_RESULTS * p_snap_results)
{
	RECT   keptRect = *p_test_info->mine;
	CORNER_INFO ci;
	RECT  screen_rect;
	
	enum SIDES kept_side = SIDE_NONE;

	InTests_TestAllCorners(&ci,&keptRect);
   
	if (testWholeDimension(SIDE_TOP,SIDE_BOTTOM,p_test_info,&keptRect,&screen_rect,&ci,&kept_side))
	{	
		DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTTOPBOTTOM,kept_side,GetSideOfRect(kept_side,&screen_rect));
		OverwriteResult(p_snap_results,kept_side,&screen_rect);
		AlignToSide(&keptRect,kept_side,GetSideOfRect(kept_side,&screen_rect));
		InTests_TestAllCorners(&ci,&keptRect);
	}
	if (testWholeDimension(SIDE_LEFT,SIDE_RIGHT,p_test_info,&keptRect,&screen_rect,&ci,&kept_side))
	{
		DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTLEFTRIGHT,kept_side,GetSideOfRect(kept_side,&screen_rect));
		OverwriteResult(p_snap_results,kept_side,&screen_rect);
		AlignToSide(&keptRect,kept_side,GetSideOfRect(kept_side,&screen_rect));
		InTests_TestAllCorners(&ci,&keptRect);
	}

	if (!p_test_info->sizing){
		if (isOnlyOneCornerOfSideOut(&ci,SIDE_TOP)){
			testSingleCorner(
				p_snap_results,
                (isCornerOut(&ci,SIDE_TOP,SIDE_LEFT)?SIDE_LEFT:SIDE_RIGHT),
				SIDE_TOP,
				kept_side,
				&keptRect);
			
		}
		else if (isOnlyOneCornerOfSideOut(&ci,SIDE_BOTTOM)){
			testSingleCorner(
				p_snap_results,
				(isCornerOut(&ci,SIDE_BOTTOM,SIDE_LEFT)?SIDE_LEFT:SIDE_RIGHT),
				SIDE_BOTTOM,
				kept_side,
				&keptRect);
		}

		KeepTopLeftOnScreen(p_snap_results,&keptRect);
	}
}


void testSingleCorner(SNAP_RESULTS * p_snap_results,enum SIDES h_side,enum SIDES v_side,enum SIDES ignored_side,LPCRECT pRect){
	RECT h_screen;
	RECT v_screen;
	int h_dist = -1;
	int v_dist = -1;
	enum SIDES closest_side = SIDE_NONE;
	BOOL has_h = FALSE;
	BOOL has_v = FALSE;
	LPCRECT pScreen;
	POINT pt;

	pt.x = GetSideOfRect(h_side,pRect);
	pt.y = GetSideOfRect(v_side,pRect);

	if (h_side != ignored_side){
        has_h = InTests_GetClosestScreenToPt(pt,h_side,&h_dist,&h_screen);
	}

	if (v_side != ignored_side){
        has_v = InTests_GetClosestScreenToPt(pt,v_side,&v_dist,&v_screen);
	}

	if (has_h && (!has_v || (h_dist <= v_dist))){
		closest_side = h_side;
		pScreen = &h_screen;
		DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTSINGLECORNER,h_side,GetSideOfRect(h_side,&h_screen));
	}
	else if (has_v && (!has_h || (v_dist < h_dist))){
		closest_side = v_side;
		pScreen = &v_screen;
		DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTSINGLECORNER,v_side,GetSideOfRect(v_side,&v_screen));
	}
	else{
		return;
	}
		
	OverwriteResult(p_snap_results,closest_side,pScreen);
}

void KeepTopLeftOnScreen(SNAP_RESULTS * p_sr, LPCRECT pRect){
	RECT snapped_position = *pRect;
	POINT pt;
	
	if (p_sr->v.side != SIDE_NONE){
		AlignToSide(&snapped_position,p_sr->v.side,p_sr->v.value);
	}
	if (p_sr->h.side != SIDE_NONE){
		AlignToSide(&snapped_position,p_sr->h.side,p_sr->h.value);
	}

	pt.x = snapped_position.left+1;
	pt.y = snapped_position.top+1;

	if (!InTests_isPtInMonitors(pt.x,pt.y)){
		RECT screen_left;
		RECT screen_top;
		
		if (InTests_GetClosestScreenToSide(SIDE_TOP,&snapped_position,&screen_top)
			&& (pt.y < screen_top.top)){
				DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTTOPLEFT,SIDE_TOP,screen_top.top);
				OverwriteResult(p_sr,SIDE_TOP,&screen_top);
			
		}
		if (InTests_GetClosestScreenToSide(SIDE_LEFT,&snapped_position,&screen_left)
			&& (pt.x < screen_left.left)){
			DBG_MSG_SIDE_VAL(g_hWnd_app,DBGMSG_KEPTTOPLEFT,SIDE_LEFT,screen_left.left);	
			OverwriteResult(p_sr,SIDE_LEFT,&screen_left);
		}
	}
}
