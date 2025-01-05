#include "NetworkDebugListView.h"

#include <strsafe.h>

#include "Gui/Models/NetworkDebugModel.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/i18n/Translator.h"
#include "Core/ServiceLocator.h"
#include "Core/TaskDispatcher.h"

CNetworkDebugListView::CNetworkDebugListView(NetworkDebugModel* model)
    : model_(model)
{
    using namespace std::placeholders;
    model_->setOnRowChangedCallback([this](auto&& PH1) { onRowChanged(PH1); });
}

void CNetworkDebugListView::Init() {
    SetItemCount(model_->getCount());

    AddColumn(TR("N"), 0);
    AddColumn(TR("Thread"), 1);
    AddColumn(TR("Time"), 2);
    AddColumn(TR("Type"), 3);
    AddColumn(TR("Text"), 4);

    /* AddColumn(_T("View URL"), 5);
    AddColumn(_T("Time"), 6);*/

    CClientDC dc(m_hWnd);
    int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
    //int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

    SetColumnWidth(0, MulDiv(25, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(1, MulDiv(45, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(2, MulDiv(160, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(3, MulDiv(130, dpiX, USER_DEFAULT_SCREEN_DPI));
    SetColumnWidth(4, MulDiv(320, dpiX, USER_DEFAULT_SCREEN_DPI));
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
    ServiceLocator::instance()->taskRunner()->runInGuiThread([&] {
        SetItemCount(model_->getCount());
        RedrawItems(index, index);
    });
}
