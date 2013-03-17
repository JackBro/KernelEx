/*
 *  KernelEx
 *  Copyright (C) 2009, Tihiy
 *
 *  This file is part of KernelEx source code.
 *
 *  KernelEx is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; version 2 of the License.
 *
 *  KernelEx is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <windows.h>
#include "desktop.h"

/* MAKE_EXPORT CreateDialogIndirectParamA_fix=CreateDialogIndirectParamA */
HWND WINAPI CreateDialogIndirectParamA_fix(HINSTANCE hInstance,
    LPCDLGTEMPLATE lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM lParamInit
)
{
	PTHREADINFO pti = get_tdb()->Win32Thread;
	HWND hWnd = NULL;
	PWND pWnd = NULL;

	hWnd = CreateDialogIndirectParamA(hInstance,
								lpTemplate,
								hWndParent,
								lpDialogFunc,
								lParamInit);

	GrabWin16Lock();

	if(hWnd == NULL)
		goto _ret;

	pWnd = HWNDtoPWND(hWnd);

	if(pWnd == NULL)
		goto _ret;

	if(pti == NULL)
		goto _ret;

	if(pti->rpdesk != gpdeskInputDesktop && pWnd->style & WS_VISIBLE)
	{
		pWnd->style |= WS_INTERNAL_WASVISIBLE;
		ReleaseWin16Lock();
		ShowWindowAsync(hWnd, SW_HIDE);
		GrabWin16Lock();
	}

_ret:
	ReleaseWin16Lock();
	return hWnd;
}

/* MAKE_EXPORT CreateDialogParamA_fix=CreateDialogParamA */
HWND WINAPI CreateDialogParamA_fix(HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam
)
{
	PTHREADINFO pti = get_tdb()->Win32Thread;
	HWND hWnd = NULL;
	PWND pWnd = NULL;

	hWnd = CreateDialogParamA(hInstance,
							lpTemplateName,
							hWndParent,
							lpDialogFunc,
							dwInitParam);

	GrabWin16Lock();

	if(hWnd == NULL)
		goto _ret;

	pWnd = HWNDtoPWND(hWnd);

	if(pWnd == NULL)
		goto _ret;

	if(pti == NULL)
		goto _ret;

	if(pti->rpdesk != gpdeskInputDesktop && pWnd->style & WS_VISIBLE)
	{
		pWnd->style |= WS_INTERNAL_WASVISIBLE;
		ReleaseWin16Lock();
		ShowWindowAsync(hWnd, SW_HIDE);
		GrabWin16Lock();
	}

_ret:
	ReleaseWin16Lock();
	return hWnd;
}

/* Here we have fix for retarded situation.
 * 9x stucks trying to get next control in dialog to tab
 * when there is only one control on dialog, and it has tab-able
 *  child windows (WS_EX_CONTROLPARENT), and if it's disabled or hidden,
 * user.exe crashes with stack overflow, silently or soundly.
 * More proper fix would be to scan whole dialog and enable/show at least
 * one such window, since GetNextDlgTabItem also happens internally.
 * However, likely it won't be this function and our case works for
 * Winamp ML.
 */


/* MAKE_EXPORT GetNextDlgTabItem_fix=GetNextDlgTabItem */
HWND WINAPI GetNextDlgTabItem_fix(
	HWND hDlg,       // handle to dialog box
	HWND hCtl,       // handle to known control
	BOOL bPrevious   // direction flag
)
{
	HWND hTestCtl = hCtl;
	if (!hTestCtl) hTestCtl = GetWindow(hDlg,GW_CHILD);
	if ( hTestCtl && !GetWindow(hTestCtl,GW_HWNDNEXT)
		&& ( !IsWindowVisible(hTestCtl) || !IsWindowEnabled(hTestCtl) )
		&& ( GetWindowLong(hTestCtl,GWL_EXSTYLE) & WS_EX_CONTROLPARENT ) ) return hCtl;

	return GetNextDlgTabItem(hDlg,hCtl,bPrevious);
}
