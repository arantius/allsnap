#include "stdafx.h"
#include "ctxhelp.h"
#include "snap_tips.h"
#include "snap_App.h" //for hinst
#include "Htmlhelp.h"
#include "resource.h"
#include "snap_settings.h"


DWORD mapID(DWORD ctrlid);

static DWORD tiptable[]={
   IDC_STATIC                 ,0
  ,IDC_SOUNDSL                ,IDS_SOUNDS
  ,IDC_SNAPIT                 ,0
  ,IDC_DESKTOP                ,IDS_DESKTOP
  ,IDC_SELF                   ,IDS_SELF
  ,IDC_OTHERS                 ,IDS_OTHERS
  ,IDC_ENABLED                ,IDS_ENABLED
  ,IDC_PLAYSOUNDS             ,IDS_PLAYSOUNDS
  ,IDC_UNSNAP_FILE            ,IDS_UNSNAP_FILE
  ,IDC_BROWSE_UNSNAP          ,IDS_BROWSE_UNSNAP
  ,IDC_SNAP_FILE              ,IDS_SNAP_FILE
  ,IDC_BROWSE_SNAP            ,IDS_BROWSE_SNAP
  ,IDC_TOGGLEKEYS             ,IDS_TOGGLEKEYS
  ,IDC_WIN_THRESH             ,IDS_WIN_THRESH
  ,IDC_WIN_THRESH_SPIN        ,IDS_WIN_THRESH
  ,IDC_SNAPATL                ,IDS_WIN_THRESH
  ,IDC_PIXELSL                ,IDS_WIN_THRESH
  ,IDC_SCREEN_THRESH          ,IDS_SCREEN_THRESH
  ,IDC_SCREEN_THRESH_SPIN     ,IDS_SCREEN_THRESH
  ,IDC_SNAPATL2               ,IDS_SCREEN_THRESH
  ,IDC_PIXELSL2               ,IDS_SCREEN_THRESH
  ,IDC_SNAPFILEL              ,IDS_SNAP_FILE
  ,IDC_TOGGLEKEYL             ,IDS_TOGGLEKEYS
  ,IDC_GENERALL               ,IDS_GENERALL
  ,IDC_UNSNAPFILEL            ,IDS_UNSNAP_FILE
  ,IDC_CHOSEL                 ,IDS_SNAPTOL
  ,IDC_SNAPMDI                ,IDS_SNAPMDI
  ,IDC_QUIETFAST              ,IDS_QUIETFAST
  ,IDC_INSIDES                ,IDS_INSIDES
  ,IDC_INVTOGGLE              ,IDS_INVTOGGLE
  ,IDC_ADVL                   ,IDS_ADVL
  ,IDC_PLAY_UNSNAP            ,IDS_PLAY
  ,IDC_PLAY_SNAP              ,IDS_PLAY
  ,IDC_CROP_TOP               ,IDS_CROP_HINT
  ,IDC_CROP_BOT               ,IDS_CROP_HINT
  ,IDC_CROP_LEFT              ,IDS_CROP_HINT
  ,IDC_CROP_RIGHT             ,IDS_CROP_HINT
  ,IDC_CROP_ENABLED           ,IDS_CROP_HINT
  ,IDC_GROUP_CROP             ,IDS_CROP_HINT
  ,IDC_HIDEICON               ,IDS_HIDEICON
  ,IDC_HCENTER                ,IDS_HCENTER
  ,IDC_VCENTER                ,IDS_VCENTER
  ,IDC_KEPT                   ,IDS_KEPT
  ,IDC_CROP_RGN               ,IDS_CROP_RGN
  ,IDC_CHECK_HORIZONTAL       ,IDS_CHECK_HORIZONTAL
  ,IDC_CHECK_VERTICAL         ,IDS_CHECK_VERTICAL
  ,0                          ,0
};


DWORD mapID(DWORD ctrlid){
	int i=0;
	while(tiptable[i]!=0){		
		if (tiptable[i] == ctrlid){
			return tiptable[i+1];
		}
		i+=2;
	}
	return 0;
}

 void tips_show(HWND hwndCtl){
#ifdef NOHTMLHELP
	 return;
#else
	HH_POPUP hPop;
	UINT idString = -1;
	TCHAR HelpString[MAX_LOADSTRING];

	memset(&hPop, 0, sizeof (HH_POPUP));
	hPop.cbStruct = sizeof (HH_POPUP);
	hPop.clrBackground = -1;
	hPop.clrForeground = -1;
	hPop.rcMargins.left = 8;
	hPop.rcMargins.right = 8;
	hPop.rcMargins.bottom = 8;
	hPop.rcMargins.top = 8;
	hPop.idString = 0;
	hPop.pszText=_T("No help for this item.");
	hPop.pt.x = -1;
	hPop.pt.y = -1;

	idString = mapID(GetDlgCtrlID(hwndCtl));
	if (idString != 0){  //???? why -1 UINT is unsigned
		LoadString(g_hInst,idString,HelpString,MAX_LOADSTRING);
		hPop.pszText = HelpString;
	}

	// It seems this type cast is the right thing to do; suppress warning.
	#pragma warning(suppress : 4311 6387)
	HtmlHelp(hwndCtl, NULL, HH_DISPLAY_TEXT_POPUP, (DWORD)&hPop);
#endif
}
