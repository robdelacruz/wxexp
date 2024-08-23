#include "wx/wx.h"
#include "ExpListView.h"

wxBEGIN_EVENT_TABLE(ExpListView, wxListView)
    EVT_SIZE(ExpListView::OnSize)
wxEND_EVENT_TABLE()

ExpListView::ExpListView(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxListView(parent, winid, pos, size, style, validator, name) {
    printf("ExpListView()\n");
}

static void FitListViewColumnWidths(wxListView *lv) {
    int ncols = lv->GetColumnCount();
    if (ncols == 0)
        return;

    int w = lv->GetClientSize().GetWidth();
    for (int i=0; i < ncols-1; i++)
        w -= lv->GetColumnWidth(i);

    lv->SetColumnWidth(ncols-1, w);
}

void ExpListView::OnSize(wxSizeEvent& e) {
    FitListViewColumnWidths(this);
    e.Skip(true);
}

