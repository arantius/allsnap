#include "stdafx.h"
#pragma comment(lib, "comctl32.lib")
#include <windowsx.h>
#include <Commctrl.h>
#define SNAPLIBAPI __declspec(dllexport)
#include "snap_lib.h"
#include "snapper.h"
#include "snap_Winrects.h"
#include "wdjsub.h"
#include "mydebug.h"
#include "snap_MultiSz.h"
#include "snap_mywm_msg.h"

#ifdef BEEPING
#define BEEP {MessageBeep(-1);}
#else
#define BEEP
#endif

#ifdef _WIN64
#define ALLSNAP_SUBCLASS_ID (477524932)
#define WM_TO_SEND (WM_USER + 13)
#define WM_TO_GET (WM_USER + 12)
#define SHARENAME "Shared64"
#define SHARECMD "/section:Shared64,rws"
#else
#define SHARENAME "Shared32"
#define SHARECMD "/section:Shared32,rws"
#define ALLSNAP_SUBCLASS_ID (477524964)
#define WM_TO_SEND (WM_USER + 12)
#define WM_TO_GET  (WM_USER + 13)
#endif


#define IS_SIZING_SPOT(x) ( ((x) >= HTSIZEFIRST) && ((x) <= HTSIZELAST) \
							|| ((x) == HTSIZE))

//Instruct compiler to put the g_thresh and g_enabled data 
//in its own data Section called Shared. We then instruct the
//linker that we want to share the data in this section
//with all instances of this aplication.?

#pragma data_seg(SHARENAME)
HHOOK	g_hhook			= NULL;
HWND	g_hWnd_app		= NULL;
UINT	g_sounds_thread_id		= 0;

gridsnap_settings_t g_gridsnap = {
	{0,0,1},
	{0,0,1}
};

UINT g_win_thresh = 10;
UINT g_screen_thresh = 15;
UINT g_toggle_key = VK_MENU;
UINT g_center_key = VK_CONTROL;
UINT g_equal_key = VK_SHIFT;
UINT g_crop_top = 0;
UINT g_crop_bottom = 7;
UINT g_crop_left = 7;
UINT g_crop_right = 7;
BOOL g_is_cropping_enabled = TRUE;
BOOL g_enabled = TRUE;
UINT g_snap_type = SNAPT_OTHERS | SNAPT_DESKTOP;
BOOL g_is_noisy = FALSE;
BOOL g_snap_mdi = FALSE;
BOOL g_is_disable_toggled = TRUE;
BOOL g_snap_insides = TRUE;
BOOL g_kept_to_screen = FALSE;
BOOL g_cropping_rgn = TRUE;
BOOL g_subclassing = FALSE;

TCHAR g_skinned_classes[MAX_CLASSNAME_LENGTH * MAX_NUM_CLASS_NAMES] =
  _T("icoCTactTrilly;Winamp v1.x;ConsoleWindowClass;Winamp PE;Winamp EQ;Winamp MB;BaseWindow_RootWnd");	
TCHAR g_ignored_classes[MAX_CLASSNAME_LENGTH * MAX_NUM_CLASS_NAMES] =
  _T("Progman;IDEOwner;NeilShadow");
#pragma data_seg()

SUBCLASSPROC	g_subclass_proc = NULL;
HWND	g_subclassed_window = NULL;
WNDPROC g_original_proc = NULL; 


// Instruct the linker to make the Shared section readable
#pragma comment(linker, SHARECMD)

 
//Nonshared variables
HINSTANCE	g_hinstDll	= NULL;

BOOL		g_moved_or_sized =FALSE;
BOOL		g_was_zoomed=FALSE;

BOOL		g_was_aerosnapped = FALSE;
int			g_last_height = 0;
int			g_last_width=0;

#define NUM_MSG_TO_SKIP (25)
unsigned int		g_num_msgs_so_far;


//Forward References
LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
#ifdef _WIN64
LRESULT APIENTRY SubclassProc64(
	HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData
);
#else
LRESULT APIENTRY SubclassProc32(
	HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData
);
#endif


void UnSubclass(HWND hwnd);
void Subclass(HWND hwnd);


BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, PVOID fImplLoad){

	switch(fdwReason){
 
		case DLL_PROCESS_ATTACH:
			//DLL is attaching to the address space of the current process
			g_hinstDll = hinstDll;
			break;
			 
		case DLL_THREAD_ATTACH:
			break;
 
		case DLL_THREAD_DETACH:
			break;    

		case DLL_PROCESS_DETACH:
			UnSubclass(g_subclassed_window);
			g_subclassing = FALSE;
			break;
	} 
	return(TRUE);
}

BOOL WINAPI SnapHookAll(HWND hwnd,UINT thread_id,OSVERSIONINFO os){
	g_hWnd_app = hwnd;
	g_sounds_thread_id = thread_id;

	if (g_hhook==NULL){
		g_hhook = SetWindowsHookEx(WH_CALLWNDPROC,(HOOKPROC)CallWndProc,g_hinstDll,0);
	}

	return (g_hhook!= NULL);
}

BOOL WINAPI SnapUnHookAll(){
	BOOL fOk = FALSE;
	fOk = UnhookWindowsHookEx(g_hhook);
	g_hhook = NULL;
	return fOk;
} 

BOOL WINAPI SnapCanUnHook(){ 
	return !g_subclassing;
}

void UnSubclass(HWND hWnd){
	BOOL res = FALSE;
#ifdef _WIN64
	res = RemoveWindowSubclass(g_subclassed_window,(SUBCLASSPROC)SubclassProc64,ALLSNAP_SUBCLASS_ID);
#else
	res = RemoveWindowSubclass(g_subclassed_window,(SUBCLASSPROC)SubclassProc32,ALLSNAP_SUBCLASS_ID);
#endif

	DBG_MSG_PTR(g_hWnd_app,DBGMSG_UNSUBCLASS_NEW,res);	

	g_original_proc = NULL;
	g_subclassed_window = NULL;
	g_subclass_proc = NULL;
	g_subclassing = FALSE;
	g_moved_or_sized = FALSE;
	g_num_msgs_so_far = 0;
}

void Subclass(HWND hwnd){
	RECT winrect;
	GetWindowRect(hwnd,&winrect);
	g_last_height = rectHeight(winrect);
	g_last_width = rectWidth(winrect);
	
	g_num_msgs_so_far = 0;
	g_moved_or_sized = FALSE;
	g_subclassed_window = hwnd;
	g_subclassing = TRUE;
#ifdef _WIN64
	g_subclass_proc = (SUBCLASSPROC) SubclassProc64;

	if (SetWindowSubclass(hwnd,(SUBCLASSPROC) SubclassProc64,ALLSNAP_SUBCLASS_ID,0)){
		snapper_OnEnterSizeMove(g_subclassed_window);
		DBG_MSG_PTR(g_hWnd_app,DBGMSG_SUBCLASS_NEW,1);
	}
	else{
		DBG_MSG_PTR(g_hWnd_app,DBGMSG_SUBCLASS_NEW,0);
	}
#else
	g_subclass_proc = (SUBCLASSPROC) SubclassProc32;

	if (SetWindowSubclass(hwnd,(SUBCLASSPROC) SubclassProc32,ALLSNAP_SUBCLASS_ID,0)){
		snapper_OnEnterSizeMove(g_subclassed_window);
		DBG_MSG_PTR(g_hWnd_app,DBGMSG_SUBCLASS_NEW,1);
	}
	else{
		DBG_MSG_PTR(g_hWnd_app,DBGMSG_SUBCLASS_NEW,0);
	}
#endif	

} 

LRESULT WINAPI CALLBACK CallWndProc(
  int nCode,      // hook code
  WPARAM wParam,  // removal option
  LPARAM lParam   // message
  ){ 
	PCWPSTRUCT lpCHook=(PCWPSTRUCT)lParam; 
	HWND hwnd = lpCHook->hwnd;
	

	
	if (nCode == HC_ACTION){ 
		switch(lpCHook->message){
			case WM_ENTERSIZEMOVE: 
				DBG_MSG(g_hWnd_app,DBGMSG_NCLBDOWN);
				if (
					(isSnapMdi() || !IS_MDI_CHILD(hwnd))
					&& isEnabled()
					&& !IsZoomed(hwnd)
				) {
			 		Subclass(hwnd);
				}
				break; 
			case WM_EXITSIZEMOVE:
				UnSubclass(g_subclassed_window);
				break; 
		}
	}

	return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
}

BOOL window_size_changed(HWND hwnd){
	RECT win_rect;
	int new_width,new_height;
	BOOL res = FALSE;

	GetWindowRect(hwnd,&win_rect);
	new_width = rectWidth(win_rect);
	new_height = rectHeight(win_rect);

	res = ( (new_width != g_last_width) || (new_height!=g_last_height));

	g_last_width = new_width;
	g_last_height = new_height;

	return res;
}

BOOL avoid_aero_snap(HWND hwnd){
	if (IsZoomed(hwnd)){
		g_was_zoomed = TRUE;
		return TRUE;
	}
	else{
		if(g_was_zoomed){
			g_was_zoomed=FALSE;
			return TRUE;
		}
	}
	if(g_num_msgs_so_far++<NUM_MSG_TO_SKIP){
		return TRUE;
	}
	return FALSE;
}


// Subclass procedure 
#ifdef _WIN64
LRESULT APIENTRY SubclassProc64(
#else
LRESULT APIENTRY SubclassProc32(
#endif
	HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData
) {  
	switch(uMsg){
		case WM_MOVING:
			if (!window_size_changed(hwnd)){
				snapper_OnMoving(hwnd,(LPRECT)(lParam));
				if (!g_moved_or_sized){
					g_moved_or_sized= TRUE;
				}
				break;
			}
			break;
		case WM_SIZING:
			if (wParam !=9){	
				snapper_OnSizing(hwnd,wParam,(LPRECT)(lParam));
				if (!g_moved_or_sized){
					g_moved_or_sized= TRUE;
				}
			}
			break;
	}

	if ((  ( (uMsg == WM_ACTIVATE) && (wParam == WA_INACTIVE))
		|| ( (uMsg == WM_ACTIVATEAPP) && (wParam == FALSE))
		|| ( uMsg == WM_EXITSIZEMOVE)
		) 
	) {
		DBG_MSG(g_hWnd_app,DBGMSG_EXITSIZEMOVE);
		UnSubclass(hwnd);
	}

	return DefSubclassProc(hwnd,uMsg,wParam,lParam);
} 

void WINAPI getGridSnap(gridsnap_settings_t  *p_grid_settings)
{
	*p_grid_settings = g_gridsnap;
}

void WINAPI setGridSnap(gridsnap_settings_t const * p_grid_settings)
{
	g_gridsnap = *p_grid_settings;
}

BOOL WINAPI isEnabled(void){
	return	g_enabled;
}

void WINAPI setEnabled(BOOL enabled){
	g_enabled = enabled;
}

BOOL WINAPI isNoisy(void){
	return	g_is_noisy;
}

void WINAPI setNoisy(BOOL is_noisy){
	g_is_noisy = is_noisy;
}

BOOL WINAPI isCroppingEnabled(void){
	return	g_is_cropping_enabled;
}

void WINAPI setCroppingEnabled(BOOL is_cropping_enabled){
	g_is_cropping_enabled = is_cropping_enabled;
}

BOOL WINAPI isSnapMdi(void){
	return	g_snap_mdi;
}
void WINAPI setSnapMdi(BOOL snap_mdi){
	g_snap_mdi = snap_mdi;
}


BOOL WINAPI isDisableToggle(void){
	return g_is_disable_toggled;
}
void WINAPI setDisableToggle(BOOL is_disable_toggle){
	g_is_disable_toggled = is_disable_toggle;
}


void WINAPI setWinThresh(int thresh){
	g_win_thresh = thresh;
}

void WINAPI setScreenThresh(int thresh){
	g_screen_thresh = thresh;
}

int  WINAPI getWinThresh(void){
	return g_win_thresh;
}
int  WINAPI getScreenThresh(void){
	return g_screen_thresh;
}


void WINAPI setCropTop(int crop_top){
	g_crop_top = crop_top;
}
int  WINAPI getCropTop(void){
	return g_crop_top;
}

void WINAPI setCropBottom(int crop_bottom){
	g_crop_bottom = crop_bottom;
}
int  WINAPI getCropBottom(void){
	return g_crop_bottom;
}

void WINAPI setCropLeft(int crop_left){
	g_crop_left = crop_left;
}
int  WINAPI getCropLeft(void){
	return g_crop_left;
}

void WINAPI setCropRight(int crop_right){
	g_crop_right = crop_right;
}
int  WINAPI getCropRight(void){
	return g_crop_right;
}


UINT WINAPI getSnapType(void){
	return g_snap_type;
}
void WINAPI setSnapType(UINT snap_type){
	g_snap_type = snap_type;
}

UINT WINAPI getToggleKey(void){
	return g_toggle_key;
}
void WINAPI setToggleKey(UINT toggle_key){
	g_toggle_key = toggle_key;
}
UINT WINAPI getCenterKey(void){
	return g_center_key;
}
void WINAPI setCenterKey(UINT center_key){
	g_center_key = center_key;
}
UINT WINAPI getEqualKey(void){
	return g_equal_key;
}
void WINAPI setEqualKey(UINT equal_key){
	g_equal_key = equal_key;
}
BOOL WINAPI isSnappingInsides(void){
	return	g_snap_insides;
}
void WINAPI setSnappingInsides(BOOL snap_insides){
	g_snap_insides = snap_insides;
}

BOOL WINAPI isKeptToScreen(void){
	return	g_kept_to_screen;
}

void WINAPI setKeptToScreen(BOOL kept_to_screen){
	g_kept_to_screen = kept_to_screen;
}

void WINAPI setSkinnedClasses(TCHAR * sz,int len){
	if (len > MAX_CLASSNAME_LIST_LENGTH){
		return;
	}
	(void)lstrcpyn(g_skinned_classes,sz,MAX_CLASSNAME_LIST_LENGTH);
}

TCHAR *  getSkinnedClasses(void){
	return g_skinned_classes;
}

void WINAPI setIgnoredClasses(TCHAR * sz,int len){	
	if (len > MAX_CLASSNAME_LIST_LENGTH){
		return;
	}
	(void)lstrcpyn(g_ignored_classes,sz,MAX_CLASSNAME_LIST_LENGTH);
}

TCHAR *  getIgnoredClasses(void){
	return g_ignored_classes;
}

void WINAPI setCroppingRgn(BOOL cropping_rgn){
	g_cropping_rgn = cropping_rgn;
}

BOOL WINAPI isCroppingRgn(void){
	return	g_cropping_rgn;
}
