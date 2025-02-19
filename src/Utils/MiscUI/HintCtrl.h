﻿// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2016, 2019-2021 - TortoiseGit
// Copyright (C) 2011, 2013, 2015, 2018, 2020 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once
#include "MyMemDC.h"
#include "Theme.h"

/**
 * \ingroup Utils
 * Allows to show a hint text on a control, basically hiding the control
 * content. Can be used for example during lengthy operations (showing "please wait")
 * or to indicate why the control is empty (showing "no data available").
 */
template <typename BaseType> class CHintCtrl : public BaseType
{
public:
	CHintCtrl() : BaseType(), m_uiFont(nullptr)
	{
		NONCLIENTMETRICS metrics = { 0 };
		metrics.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, FALSE);
		m_uiFont = CreateFontIndirect(&metrics.lfMessageFont);
	}
	virtual ~CHintCtrl()
	{
		if (m_uiFont)
			DeleteObject(m_uiFont);
	}

	void ShowText(const CString& sText, bool forceupdate = false)
	{
		m_sText = sText;
		BaseType::Invalidate();
		if (forceupdate)
			BaseType::UpdateWindow();
	}

	void ClearText()
	{
		m_sText.Empty();
		BaseType::Invalidate();
	}

	bool HasText() const {return !m_sText.IsEmpty();}

	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnPaint()
	{
		LRESULT defres = BaseType::Default();
		if (!m_sText.IsEmpty())
		{
			COLORREF clrText = CTheme::Instance().IsDarkTheme() ? CTheme::darkTextColor : ::GetSysColor(COLOR_WINDOWTEXT);
			COLORREF clrTextBk;
			if (BaseType::IsWindowEnabled())
				clrTextBk = CTheme::Instance().IsDarkTheme() ? CTheme::darkBkColor : ::GetSysColor(COLOR_WINDOW);
			else
				clrTextBk = CTheme::Instance().GetThemeColor(::GetSysColor(COLOR_3DFACE));

			CRect rc;
			BaseType::GetClientRect(&rc);
			bool bIsEmpty = false;
			CListCtrl * pListCtrl = dynamic_cast<CListCtrl*>(this);
			if (pListCtrl)
			{
				CHeaderCtrl* pHC;
				pHC = pListCtrl->GetHeaderCtrl();
				if (pHC)
				{
					CRect rcH;
					rcH.SetRectEmpty();
					pHC->GetItemRect(0, &rcH);
					rc.top += rcH.bottom;
				}
				bIsEmpty = pListCtrl->GetItemCount() == 0;
			}
			CDC* pDC = BaseType::GetDC();
			{
				pDC->SetBkMode(TRANSPARENT);
				pDC->SetTextColor(clrText);
				pDC->SetBkColor(clrTextBk);
				CMyMemDC memDC(pDC, &rc);

				memDC.SetTextColor(clrText);
				memDC.SetBkColor(clrTextBk);
				memDC.SetBkMode(TRANSPARENT);
				if (bIsEmpty)
					memDC.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), pDC, rc.left, rc.top, SRCCOPY);
				else
					memDC.FillSolidRect(rc, clrTextBk);
				rc.top += 10;
				CGdiObject* oldfont = memDC.SelectObject(CGdiObject::FromHandle(m_uiFont));
				memDC.DrawText(m_sText, rc, DT_CENTER | DT_VCENTER |
					DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
				memDC.SelectObject(oldfont);
			}
			BaseType::ReleaseDC(pDC);
		}
		if (defres)
		{
			// the Default() call did not process the WM_PAINT message!
			// Validate the update region ourselves to avoid
			// an endless loop repainting
			CRect rc;
			BaseType::GetUpdateRect(&rc, FALSE);
			if (!rc.IsRectEmpty())
				BaseType::ValidateRect(rc);
		}
	}

private:
	CString			m_sText;
	HFONT			m_uiFont;
};

BEGIN_TEMPLATE_MESSAGE_MAP(CHintCtrl, BaseType, BaseType)
	ON_WM_PAINT()
END_MESSAGE_MAP()

