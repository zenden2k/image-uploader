#include "ServerListView.h"

#include "ServersCheckerModel.h"
#include "Core/Utils/CoreUtils.h"

namespace ServersListTool {

CServerListView::CServerListView(ServersCheckerModel* model) : model_(model){
    using namespace std::placeholders;
    model_->setOnRowChangedCallback(std::bind(&CServerListView::onRowChanged, this, _1));
}

CServerListView::~CServerListView() {
}

void CServerListView::Init() {
    SetItemCount(model_->getCount());

    AddColumn(_T("N"), 0);
    AddColumn(_T("Server"), 1);
    AddColumn(_T("Status"), 2);
    AddColumn(_T("Direct URL"), 3);
    AddColumn(_T("Thumb URL"), 4);
    AddColumn(_T("View URL"), 5);
    AddColumn(_T("Time"), 6);
    SetColumnWidth(0, 30);
    SetColumnWidth(1, 150);
    SetColumnWidth(2, 100);
    SetColumnWidth(3, 205);
    SetColumnWidth(4, 205);
    SetColumnWidth(5, 205);
    SetColumnWidth(6, 50);
}

LRESULT CServerListView::OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
    LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pnmh);
    LV_ITEM* pItem = &(pDispInfo)->item;
    DWORD n = pItem->iItem;

    if (pItem->mask & LVIF_TEXT)  {
        std::string str = model_->getItemText(n, pItem->iSubItem);
        lstrcpy(pItem->pszText, U2W(str));
    }

    return 0;
}

LRESULT CServerListView::OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh);

    switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
    {
        auto color = model_->getItemColor(lplvcd->nmcd.dwItemSpec);

        if (color) {
            lplvcd->clrTextBk = color;
            return CDRF_NEWFONT;
        }
    }
    break;
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
        lplvcd->clrText = RGB(255, 0, 0);
        return CDRF_NEWFONT;
    }
    return 0;
}

void CServerListView::onRowChanged(size_t index) {
    RedrawItems(index, index);
}
}