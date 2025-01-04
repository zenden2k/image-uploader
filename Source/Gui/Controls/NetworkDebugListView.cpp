#include "NetworkDebugListView.h"

#include <strsafe.h>

#include "Gui/Models/FileFormatCheckErrorModel.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/i18n/Translator.h"

CNetworkDebugListView::CNetworkDebugListView(FileFormatCheckErrorModel* model)
    : model_(model)
{
    using namespace std::placeholders;
    model_->setOnRowChangedCallback([this](auto&& PH1) { onRowChanged(PH1); });
}

void CNetworkDebugListView::Init() {
    SetItemCount(model_->getCount());

    AddColumn(TR("N"), 0);
    AddColumn(TR("File"), 1);
    AddColumn(TR("Mime-type"), 2);
    AddColumn(TR("Size"), 3);
    AddColumn(TR("Extension"), 4);
    AddColumn(TR("Server"), 5);
    /* AddColumn(_T("View URL"), 5);
    AddColumn(_T("Time"), 6);*/

    CClientDC dc(m_hWnd);
    int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
    //int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

    SetColumnWidth(0, MulDiv(25, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(1, MulDiv(160, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(2, MulDiv(130, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(3, MulDiv(80, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(4, MulDiv(90, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(5, MulDiv(170, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(6, MulDiv(40, dpiX, USER_DEFAULT_SCREEN_DPI));
}

LRESULT CNetworkDebugListView::OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
    auto* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pnmh);
    LV_ITEM* pItem = &(pDispInfo)->item;

    if (pItem->mask & LVIF_TEXT)  {
        std::string str = model_->getItemText(pItem->iItem, pItem->iSubItem);
        std::wstring wstr = IuCoreUtils::Utf8ToWstring(str);
        StringCchCopy(pItem->pszText, pItem->cchTextMax, wstr.c_str());
    }

    return 0;
}

LRESULT CNetworkDebugListView::OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto* lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh);

    switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
    {
        auto color = model_->getItemColor(lplvcd->nmcd.dwItemSpec);

        if (color) {
            lplvcd->clrText = color;
            return CDRF_NEWFONT;
        }
    }
    break;
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
        //lplvcd->clrText = RGB(255, 0, 0);
        return CDRF_NEWFONT;
    }
    return 0;
}

void CNetworkDebugListView::onRowChanged(size_t index) {
    RedrawItems(index, index);
}
