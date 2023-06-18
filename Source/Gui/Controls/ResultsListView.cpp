#include "ResultsListView.h"

#include "Gui/UploadListModel.h"
#include "Core/i18n/Translator.h"

CResultsListView::CResultsListView() :  model_(nullptr) {
}

bool CResultsListView::AttachToDlgItem(HWND parent, UINT dlgID) {

    HWND hWnd = ::GetDlgItem(parent,dlgID);
    bool res = SubclassWindow(hWnd)!=FALSE;
    Init();
    return res;
}

void CResultsListView::Init() {

    AddColumn(TR("File"), 1);
    AddColumn(TR("Status"), 1);
    AddColumn(TR("Thumbnail"), 2);

    CWindowDC hdc(m_hWnd);
    float dpiScaleX = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX)) / 96.0f;
    int columnWidth = static_cast<int>(170 * dpiScaleX);
    SetColumnWidth(0, columnWidth);
    SetColumnWidth(1, columnWidth);
    SetColumnWidth(2, columnWidth);
}

void CResultsListView::SetModel(UploadListModel* model) {
    if (model_ == model) {
        return;
    }
    model_ = model;
    using namespace std::placeholders;
    SetItemCount(model_ ? static_cast<int>(model_->getCount()) : 0);
    if (model_) {
        model_->setOnRowChangedCallback([this](size_t index) { onRowChanged(index); });
    }
}

LRESULT CResultsListView::OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
    auto* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pnmh);
    LV_ITEM* pItem = &(pDispInfo)->item;
    DWORD n = pItem->iItem;

    if (pItem->mask & LVIF_TEXT) {
        CString str = model_->getItemText(n, pItem->iSubItem);
        lstrcpy(pItem->pszText, str);
    }

    return 0;
}

// Disabled in MSG MAP
LRESULT CResultsListView::OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto* lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh);

    switch (lplvcd->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;

        case CDDS_ITEMPREPAINT:
        {
            return CDRF_NOTIFYSUBITEMDRAW;
            /*auto color = model_->getItemTextColor(lplvcd->nmcd.dwItemSpec);
            if (color) {
                lplvcd->clrText = color;
                //return CDRF_NEWFONT;
                
            }*/
        }
        break;
        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
            lplvcd->clrText = model_->getItemTextColor(lplvcd->nmcd.dwItemSpec, lplvcd->iSubItem);
            return CDRF_NEWFONT;
    }
    return CDRF_DODEFAULT;
}

// Called from the worker thread
void CResultsListView::onRowChanged(size_t index) {
    RedrawItems(index, index);
}
